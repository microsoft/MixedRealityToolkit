//////////////////////////////////////////////////////////////////////////
// VisualPairReceiverImpl.cpp
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VisualPairReceiverImpl.h"
#include "TagImageImpl.h"

#define NO_PNG
#include <zint.h>

XTOOLS_NAMESPACE_BEGIN

VisualPairReceiverPtr VisualPairReceiver::Create()
{
	return new VisualPairReceiverImpl();
}


VisualPairReceiverImpl::VisualPairReceiverImpl()
{
	// Get a list of all the IP addresses that this machine knows about
	m_pairingInfo.SetAddresses(XSocketManager::GetLocalMachineAddresses());

	m_pairingInfo.SetPort(kAppPluginPort);

	uint32 localKey = std::rand();
	uint32 remoteKey = std::rand();

	m_pairingInfo.SetLocalKey(localKey);
	m_pairingInfo.SetRemoteKey(remoteKey);
}


bool VisualPairReceiverImpl::IsReceiver()
{
	return true;
}


int32 VisualPairReceiverImpl::GetAddressCount()
{
	return (int32)m_pairingInfo.GetAddresses().size();
}


XStringPtr VisualPairReceiverImpl::GetAddress(int32 index)
{
	return new XString(m_pairingInfo.GetAddresses()[index].ToString());
}


uint16 VisualPairReceiverImpl::GetPort()
{
	return m_pairingInfo.GetPort();
}


bool VisualPairReceiverImpl::IsReadyToConnect()
{
	return true;
}


int32 VisualPairReceiverImpl::GetLocalKey()
{
	return m_pairingInfo.GetLocalKey();
}


int32 VisualPairReceiverImpl::GetRemoteKey()
{
	return m_pairingInfo.GetRemoteKey();
}


TagImagePtr VisualPairReceiverImpl::CreateTagImage() const
{
	TagImageImplPtr tagImage;

	std::string pairingString = m_pairingInfo.Encode();

	zint_symbol *my_symbol;

	my_symbol = ZBarcode_Create();
	my_symbol->input_mode = UNICODE_MODE;
	my_symbol->symbology = BARCODE_QRCODE;
	my_symbol->border_width = 2;

	int error_number = ZBarcode_Encode_and_Buffer(my_symbol, (byte*)pairingString.c_str(), 0, 0);
	if (error_number == 0)
	{
		tagImage = new TagImageImpl(my_symbol->bitmap_width, my_symbol->bitmap_height);

		int i = 0;
		for (int row = 0; row < my_symbol->bitmap_height; row++)
		{
			for (int column = 0; column < my_symbol->bitmap_width; column++)
			{
				char value = (my_symbol->bitmap[i] == 0) ? 0 : 255;

				tagImage->SetPixel(column, row, value);

				i += 3;
			}
		}
	}

	ZBarcode_Delete(my_symbol);

	return tagImage;
}

XTOOLS_NAMESPACE_END