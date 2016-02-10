//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "MessageIdentifiers.h"

using namespace WinRTTest;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;


using namespace RakNet;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238


//#define DEFAULT_SERVER_PORT 61111
#define DEFAULT_SERVER_PORT 0x507B
//#define DEFAULT_SERVER_ADDRESS "natpunch.jenkinssoftware.com"
//#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define DEFAULT_SERVER_ADDRESS "localhost"
//#define DEFAULT_SERVER_ADDRESS "www.microsoft.com"

MainPage::MainPage()
{
	InitializeComponent();

	RakPeerInterface *rakPeer = RakPeerInterface::GetInstance();
	SocketDescriptor sd;
	StartupResult sr = rakPeer->Startup(1, &sd, 1);
	assert(sr == RAKNET_STARTED);
	ConnectionAttemptResult car = rakPeer->Connect(DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT, 0, 0);
	assert(car == CONNECTION_ATTEMPT_STARTED);
	RakSleep(1000);
	Packet *packet;
	packet = rakPeer->Receive();
	//if (packet)
	//{
	//	RakAssert(packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED);
	//}
}
