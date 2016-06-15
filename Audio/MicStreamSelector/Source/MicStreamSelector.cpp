#include <ppltasks.h>
#include <MemoryBuffer.h>
#include <mutex>
#include <queue>

using namespace concurrency;
using namespace std;
using namespace Platform;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Windows::Media::Audio;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Render;
using namespace Windows::Media::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Media::Transcoding;

#define API __declspec(dllexport)

// #define MEMORYLEAKDETECT // Enables memory leak debugging output on program.

#ifdef MEMORYLEAKDETECT
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif MEMORYLEAKDETECT

//#define DEBUG  // allows more thorough debugging information regardless of other settings
#ifdef DEBUG
	#pragma optimize( "", off)
#endif

void	OnQuantumProcessed(Windows::Media::Audio::AudioGraph ^sender, Platform::Object ^args);
static	AudioGraph^				graph;							// we will only use one graph
static	AudioDeviceInputNode^	deviceInputNode;				// we will only use one device input node
static	AudioFrameOutputNode^	frameoutputnode;				// this is where we will grab our data to send back to the app
static	AudioFileOutputNode^	fileOutputNode;					// so we can very easily record files
static	AudioDeviceOutputNode^	deviceOutputNode;				// so we can preview the stream if desired
static	MediaEncodingProfile^	mediaEncodingProfile;			// we can set the type of file we want to record
static	StorageFolder^			localFolder;					// current folder we're using to record files
static	StorageFile^			wavFile;						// the file we'll be recording to. we drop the handle after we save it, and this will recycle and point to the next file.
static	char					filepath_char[255];				// stores the full file path to return to the app for easy wav loading
static	std::queue<Windows::Media::AudioFrame^> audioqueue;		// stores our microphone data to hand back to the app

// error codes to hand back to engine with nice printed output
private enum ErrorCodes { ALREADY_RUNNING = -10, NO_AUDIO_DEVICE, NO_INPUT_DEVICE, ALREADY_RECORDING, GRAPH_NOT_EXIST, CHANNEL_COUNT_MISMATCH, FILE_CREATION_PERMISSION_ERROR, NOT_ENOUGH_DATA, NEED_ENABLED_MIC_CAPABILITY};

bool		keepAllData = false;	// Keeping all data can cause voice the microphone to accumulate large lag and memory if the program ever blocks or breaks, but is sometimes needed
bool		recording = false;
int			appExpectedBufferLength = -1; // By making negative, we won't output data until we get at least one app call.
int			dataInBuffer = 0;
int			samplesPerQuantum = 256;
int			numChannels = 2;
int			indexInFrame = 0;
float		micGain = 1.f;
std::mutex	mtx, creationmtx, readmtx;

typedef void(__stdcall * CallbackIntoHost)();	// a potential callback into non-polled apps where this audio engine drives behaviour in the app, like VOIP
static CallbackIntoHost hostCallback = nullptr;


// This is the callback after every frame the audio device captures.
void OnQuantumProcessed(Windows::Media::Audio::AudioGraph ^sender, Platform::Object ^args)
{
	// If we're under our desired queue size or are keeping everything, take a frame and account for it.
	if (dataInBuffer < appExpectedBufferLength*2 && !keepAllData || keepAllData)
	{
		mtx.lock();	// Ensure thread safety on this hand-off point.
		audioqueue.push(frameoutputnode->GetFrame());
		mtx.unlock();
		dataInBuffer += samplesPerQuantum * numChannels;
	}
	else // If we're here, we don't want this data. Just eat it from the audio driver & throw it away.
	{
		frameoutputnode->GetFrame();
	}

	if (dataInBuffer >= appExpectedBufferLength * 2 && hostCallback)	// if we're using a callback, let the host know that there is a buffer ready to go
		hostCallback();
}

// TODO allow saving to designated folder?
int MakeSaveFile(char* c_filename) {
	std::string str = std::string(c_filename);
	std::wstring widestr = std::wstring(str.begin(), str.end());
	String^ filename = ref new String(widestr.c_str());
	try {
		localFolder = KnownFolders::MusicLibrary;	// Works by default on HoloLens.
	}
	catch (Exception^ e) {
		return ErrorCodes::FILE_CREATION_PERMISSION_ERROR;	// Didn't have access to folder.
	}

	wavFile = concurrency::create_task(localFolder->CreateFileAsync(filename, CreationCollisionOption::GenerateUniqueName)).get();
	MediaEncodingProfile^ profile = MediaEncodingProfile::CreateWav(AudioEncodingQuality::High);
	CreateAudioFileOutputNodeResult^ fileOutputNodeResult = concurrency::create_task(graph->CreateFileOutputNodeAsync(wavFile, profile)).get();

	if (fileOutputNodeResult->Status != AudioFileNodeCreationStatus::Success)
	{
		return ErrorCodes::FILE_CREATION_PERMISSION_ERROR;	// Didn't have access to file.
	}

	fileOutputNode = fileOutputNodeResult->FileOutputNode;
	return 0;
}

extern "C" {

	API int MicInitializeCustomRateWithGraph(int category, int samplerate, AudioGraph^ appGraph) // If category is 1 or 2, we'll do speech things, otherwise we'll do environment.
	{  
		std::lock_guard<std::mutex> lock(creationmtx); // allows only one process to access this function at a time
#ifdef MEMORYLEAKDETECT
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
		if (appGraph != nullptr)
		{
			if (graph == appGraph)
				return ErrorCodes::ALREADY_RUNNING;	// No need to start if it's already running.

			graph = appGraph;	// if the app itself made its own graph, lets attach to that
		}
		else					// otherwise, we need to make our graph
		{
			if (graph)
				return ErrorCodes::ALREADY_RUNNING;	// No need to start if it's already running.

			AudioGraphSettings^ settings = ref new AudioGraphSettings(AudioRenderCategory::Media);
			settings->QuantumSizeSelectionMode = QuantumSizeSelectionMode::LowestLatency;	// Usually good to try even if device doesn't have a low latency mode.
			if (samplerate != 0)	// if not default, then try to set to requested size
				settings->EncodingProperties = AudioEncodingProperties::CreatePcm(samplerate, 2, 16);	// HoloLens is a 2 channel 16 bit endpoint.
			auto outputdevices = concurrency::create_task(DeviceInformation::FindAllAsync(MediaDevice::GetAudioRenderSelector())).get();
			settings->PrimaryRenderDevice = outputdevices->GetAt(0);	// First indexed device is always the default render device as set by OS.
			CreateAudioGraphResult^ result = concurrency::create_task(AudioGraph::CreateAsync(settings)).get();

			if (result->Status != AudioGraphCreationStatus::Success)
				return ErrorCodes::NO_AUDIO_DEVICE;	// No audio device present on machine.

			graph = result->Graph;
		}
		appExpectedBufferLength = samplesPerQuantum = graph->SamplesPerQuantum;

		CreateAudioDeviceInputNodeResult^ deviceInputNodeResult;
		if (category == 0)
		{
			// This will return beam-formed capture for voice on HoloLens. Lower bit-rate for streaming voice stuff.
			deviceInputNodeResult = concurrency::create_task(graph->CreateDeviceInputNodeAsync(Windows::Media::Capture::MediaCategory::Speech)).get();
		}
		else if (category == 1)
		{
			// This will return beam-formed capture for voice on HoloLens. High quality voice.
			deviceInputNodeResult = concurrency::create_task(graph->CreateDeviceInputNodeAsync(Windows::Media::Capture::MediaCategory::Communications)).get();
		}
		else
		{
			// This will return an outward facing capture of the HoloLens' environment, explicitly not capturing the vocal microphones. 
			deviceInputNodeResult = concurrency::create_task(graph->CreateDeviceInputNodeAsync(Windows::Media::Capture::MediaCategory::Media)).get();
		}
		if (deviceInputNodeResult->Status != AudioDeviceNodeCreationStatus::Success)
			return ErrorCodes::NO_INPUT_DEVICE;

		deviceInputNode = deviceInputNodeResult->DeviceInputNode;

		CreateAudioDeviceOutputNodeResult^ deviceOutputNodeResult;
		deviceOutputNodeResult = concurrency::create_task(graph->CreateDeviceOutputNodeAsync()).get();
		deviceOutputNode = deviceOutputNodeResult->DeviceOutputNode;
		
		AudioEncodingProperties^ props = ref new AudioEncodingProperties();
		props = deviceInputNode->EncodingProperties;
		frameoutputnode = graph->CreateFrameOutputNode(props);
		deviceInputNode->AddOutgoingConnection(frameoutputnode);
		deviceInputNode->AddOutgoingConnection(deviceOutputNode);
		deviceInputNode->OutgoingGain = micGain;

		// Make a callback at the end of every frame to store our data until the app wants it.
		graph->QuantumProcessed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Audio::AudioGraph ^, Platform::Object ^>(&OnQuantumProcessed);

		return 0;
	}

	API int MicInitializeCustomRate(int category, int samplerate) // same as before, but uses custom samplerate 
	{
		return MicInitializeCustomRateWithGraph(category, samplerate, nullptr);
	}

	API int MicInitializeDefault(int category) // same as before, but uses default samplerate
	{  
		return MicInitializeCustomRateWithGraph(category, 0, nullptr);
	}

	API int MicInitializeDefaultWithGraph(int category, AudioGraph^ appGraph) // same as before, but uses default samplerate and graph from app
	{
		return MicInitializeCustomRateWithGraph(category, 0, appGraph);
	}

	API int MicStartStream(bool keepData, bool previewOnDevice, CallbackIntoHost cb)
	{
		if (!graph) 
		{
			int err = MicInitializeDefault(0);
			if (err != 0)
				return err;
		}
		keepAllData = keepData;
		if (previewOnDevice)
			deviceOutputNode->OutgoingGain = 1;
		else if (deviceOutputNode)
			deviceOutputNode->OutgoingGain = 0;
		hostCallback = cb;
		graph->Start();
		return 0;
	}

	API int MicStopStream()
	{
		if (!graph)
			return ErrorCodes::GRAPH_NOT_EXIST;
		graph->Stop();
		hostCallback = nullptr;
		return 0;
	}

	API int MicStartRecording(char* c_filename, bool previewOnDevice) 
	{
		if (recording)
		{
			return ErrorCodes::ALREADY_RECORDING;
		}
		if (!graph)
		{
			int err = MicInitializeDefault(0);
			if (err != 0)
				return err;
		}
		MakeSaveFile(c_filename);
		deviceInputNode->AddOutgoingConnection(fileOutputNode);
		if (previewOnDevice)
			deviceOutputNode->OutgoingGain = 1;
		else if (deviceOutputNode)
			deviceOutputNode->OutgoingGain = 0;
		graph->Start();
		recording = true;
		return 0;
	}

	API void MicStopRecording(char* path)
	{
		if (!recording)
		{
			strncpy_s(path, 255, "YouArentRecordingRightNow", 26);
			return;
		}
		fileOutputNode->Stop();
		TranscodeFailureReason finalizeResult = concurrency::create_task(fileOutputNode->FinalizeAsync()).get();
		recording = false;

		size_t nConverted = 0;

		if (finalizeResult != TranscodeFailureReason::None)
		{
			wcstombs_s(&nConverted, filepath_char, 255, finalizeResult.ToString()->Data(), 255);
		}
		else {
			wcstombs_s(&nConverted, filepath_char, 255, wavFile->Path->Data(), 255);
		}
		filepath_char[nConverted] = '\0';
		strncpy_s(path, 255, filepath_char, nConverted + 1);
	}

	API int MicGetFrame(float* buffer, int length, int numchannels) {
		std::lock_guard<std::mutex> lock(readmtx); // allows only one process to access this function at a time

		if (!graph)
			return ErrorCodes::GRAPH_NOT_EXIST;

		if (length != 0)
			appExpectedBufferLength = length; // Let the system know what our program is trying to deal with for background buffering reasons.
		else
			appExpectedBufferLength = samplesPerQuantum;	// use defualt size if not custom set by application

		if (dataInBuffer < length)	// Make sure we have enough data to hand back to the app.
			return ErrorCodes::NOT_ENOUGH_DATA;					// This should only hit when we first start the application.
		
		int framesize = samplesPerQuantum * numChannels;
		
		if (numchannels != numChannels)	// This is a gross assumption of this entire plug-in.
		{
			return ErrorCodes::CHANNEL_COUNT_MISMATCH;
		}

		size_t sizer = audioqueue.size();
		mtx.lock();
		Windows::Media::AudioFrame^ frame = audioqueue.front();
		mtx.unlock();
		Windows::Media::AudioBuffer^ framebuffer = frame->LockBuffer(Windows::Media::AudioBufferAccessMode::Read);
		Windows::Foundation::IMemoryBufferReference^ memoryBufferReference = framebuffer->CreateReference();
		ComPtr<IMemoryBufferByteAccess> memoryBufferByteAccess;
		auto hr = reinterpret_cast<IInspectable*>(memoryBufferReference)->QueryInterface(IID_PPV_ARGS(&memoryBufferByteAccess));
		byte *outdataInBytes = nullptr;
		UINT32 outcapacityInBytes = 0;
		hr = memoryBufferByteAccess->GetBuffer(&outdataInBytes, &outcapacityInBytes);
		float* outdataInFloat = (float*)outdataInBytes;
		for (int i = 0; i < length; i++)
		{
			if (indexInFrame == framesize) {	// If we've reached the end of our audio frame, get the next frame.
				mtx.lock();
				frame = nullptr;			// release last frame (probably unnecessary)
				audioqueue.pop();			// remove old front frame in queue (the one we just released)
				frame = audioqueue.front();	// point to the new front of the queue
				mtx.unlock();
				framebuffer = frame->LockBuffer(Windows::Media::AudioBufferAccessMode::Read);
				memoryBufferReference = framebuffer->CreateReference();
				hr = reinterpret_cast<IInspectable*>(memoryBufferReference)->QueryInterface(IID_PPV_ARGS(&memoryBufferByteAccess));
				hr = memoryBufferByteAccess->GetBuffer(&outdataInBytes, &outcapacityInBytes);
				outdataInFloat = (float*)outdataInBytes;
				indexInFrame = 0;
			}
			buffer[i] = outdataInFloat[indexInFrame];
			++indexInFrame;
			--dataInBuffer;
		}
		return 0;
	}

	API int MicPause() {
		if (!graph)
			return ErrorCodes::GRAPH_NOT_EXIST;
		graph->Stop();
		return 0;
	}

	API int MicResume() {
		if (!graph)
			return ErrorCodes::GRAPH_NOT_EXIST;
		graph->Start();
		return 0;
	}

	API int MicSetGain(float gain) {
		micGain = gain;
		if (!graph || !deviceInputNode)
			return 0;// ErrorCodes::GRAPH_NOT_EXIST;;	//allow early setting of Volume before graph init. not an error

		// The range of this parameter is not documented, but 1 is default and 0 is silence. Can go very high and distorts quickly.
		deviceInputNode->OutgoingGain = gain;
		return 0;
	}

	API int MicDestroy() 
	{
		if (!graph)	// If there isn't a graph, there is nothing to stop, so just return
			return ErrorCodes::GRAPH_NOT_EXIST;
		hostCallback = nullptr;
		graph->Stop(); 
		//graph->Close();	// this should work, but appears bugged in current APIs
#ifdef MEMORYLEAKDETECT
		_CrtDumpMemoryLeaks(); // output our memory stats if desired
#endif // MEMORYLEAKDETECT
		return 0;
	}

	API int MicGetDefaultBufferSize() // if you were doing default setup, you need to know what's going on sometimes
	{
		return appExpectedBufferLength;
	}
	API int MicGetDefaultNumChannels() // if you were doing default setup, you need to know what's going on sometimes
	{
		return numChannels;
	}
}

#ifdef DEBUG
	#pragma optimize( "", on)
#endif