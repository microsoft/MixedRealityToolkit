//////////////////////////////////////////////////////////////////////////
// VisualPairConnectorImpl.cpp
//
// Implementation of the VisualPairConnector interface
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisualPairConnectorImpl.h"
#include "ImageLuminanceSource.h"
#include "PairingInfo.h"
#include <Private/Utils/ScopedLock.h>

XTOOLS_NAMESPACE_BEGIN

VisualPairConnectorPtr VisualPairConnector::Create()
{
	return new VisualPairConnectorImpl();
}



VisualPairConnectorImpl::VisualPairConnectorImpl()
	: m_event(false, false)
	, m_stopping(0)
	, m_bProcessing(false)
	, m_bProcessingSuccessful(false)
{
	m_thread = new MemberFuncThread(&VisualPairConnectorImpl::ThreadFunc, this);
}


VisualPairConnectorImpl::~VisualPairConnectorImpl()
{
	m_stopping = 1;
	m_event.Set();
	m_thread->WaitForThreadExit();
}


bool VisualPairConnectorImpl::IsReceiver()
{
	return false;
}


int32 VisualPairConnectorImpl::GetAddressCount()
{
	return (int32)m_pairingInfo.GetAddresses().size();
}


XStringPtr VisualPairConnectorImpl::GetAddress(int32 index)
{
	return new XString(m_pairingInfo.GetAddresses()[index].ToString());
}


uint16 VisualPairConnectorImpl::GetPort()
{
	return m_pairingInfo.GetPort();
}


bool VisualPairConnectorImpl::IsReadyToConnect()
{
	return (!m_bProcessing && m_bProcessingSuccessful);
}


int32 VisualPairConnectorImpl::GetLocalKey()
{
	return m_pairingInfo.GetLocalKey();
}


int32 VisualPairConnectorImpl::GetRemoteKey()
{
	return m_pairingInfo.GetRemoteKey();
}


bool VisualPairConnectorImpl::ProcessImage(byte* image, int32 width, int32 height, int32 bytePerPixel)
{
	if (image != nullptr && 
		!m_bProcessing	// There's already an image being processed, don't start another
		)
	{
		m_bProcessingSuccessful = false;
		m_bProcessing = true;

		m_imageData = new ImageData(image, width, height, bytePerPixel);

		m_event.Set();

		return true;
	}
	else
	{
		return false;
	}
}


bool VisualPairConnectorImpl::IsProcessingImage() const
{
	return m_bProcessing;
}


void VisualPairConnectorImpl::ThreadFunc()
{
	while (!m_stopping)
	{
		m_bProcessing = false;

		// Wait to be signaled that an image is ready to be processed
		if (m_event.Wait())
		{
			// If there is an image to process
			if (m_imageData != nullptr)
			{
				std::string qrCodeString = DecodeImage(m_imageData);
				if (qrCodeString.length() > 0)
				{
					LogInfo("Decoded string: %s", qrCodeString.c_str());

					if (m_pairingInfo.Decode(qrCodeString))
					{
						m_bProcessingSuccessful = true;
					}
				}

				m_imageData = nullptr;
			}
		}
	}
}


std::string VisualPairConnectorImpl::DecodeImage(const ImageDataPtr& image)
{
	PROFILE_SCOPE("Detecting QR Code");

	using namespace zxing;

	try
	{
		// Only look for QR Codes.  
		DecodeHints hints(DecodeHints::QR_CODE_HINT);
		hints.setTryHarder(true);

		Ref<LuminanceSource> source(new ImageLuminanceSource(image));
		Ref<Binarizer> binarizer(new HybridBinarizer(source));
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		Ref<Reader> reader(new qrcode::QRCodeReader);

		Ref<Result> result = reader->decode(binary, hints);
		if (!result.empty())
		{
			return result->getText()->getText();
		}
	}
	catch (const ReaderException& e)
	{
		LogError("zxing::ReaderException: %s", std::string(e.what()).c_str());
	}
	catch (const zxing::IllegalArgumentException& e)
	{
		LogError("zxing::IllegalArgumentException: %s", std::string(e.what()).c_str());
	}
	catch (const zxing::Exception& e)
	{
		LogError("zxing::Exception: %s", std::string(e.what()).c_str());
	}
	catch (const std::exception& e)
	{
		LogError("std::exception: %s", std::string(e.what()).c_str());
	}

	return std::string();
}

XTOOLS_NAMESPACE_END
