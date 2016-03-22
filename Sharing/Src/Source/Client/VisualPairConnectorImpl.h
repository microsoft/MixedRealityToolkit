//////////////////////////////////////////////////////////////////////////
// VisualPairConnectorImpl.h
//
// Implementation of the VisualPairConnector interface
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "ZXingIncludes.h"
#include "ImageData.h"
#include "PairingInfo.h"

XTOOLS_NAMESPACE_BEGIN

class VisualPairConnectorImpl : public VisualPairConnector
{
public:
	VisualPairConnectorImpl();
	virtual ~VisualPairConnectorImpl();

	// PairMaker Functions:
	// Return true if the local machine will be receiving the pairing connection.
	// Return false if it will be initiating the pairing connection
	virtual bool		IsReceiver() XTOVERRIDE;

	// Return the number of addresses test when attempting to connect
	// Only used if IsReceiver() returns false
	virtual int32		GetAddressCount() XTOVERRIDE;

	// Return one of the address of the remote client to try to connect to.
	// Only used if IsReceiver() returns false
	virtual XStringPtr	GetAddress(int32 index) XTOVERRIDE;

	// If receiving the pairing connection, returns the port to listen on.
	// If initiating the pairing connection, returns the port on the remote client to connect to
	virtual uint16		GetPort() XTOVERRIDE;

	// Return true when this object is ready to provide XTools with the information it needs to start 
	// the pairing process.  ie: GetAddress and GetPort will not be called until this returns true.
	virtual bool		IsReadyToConnect() XTOVERRIDE;

	// Returns the key to send the remote client when validating the pairing connection.  
	// If no such check is necessary, return 0
	virtual int32		GetLocalKey() XTOVERRIDE;

	// Returns the key that should be received from the remote machine when validating the pairing connection.  
	// If the key received from the remote client does not match this key then the pairing will fail.  
	// If no such check is necessary, return 0
	virtual int32		GetRemoteKey() XTOVERRIDE;

	// Provide the connector with an image taken from the device's camera.  
	// Calling this function will kick off an asynchronous task to find any pairing tags in the image
	// and decode the pairing information within it.  Returns true if the async process is kicked off,
	// returns false if another image is already being processed.  
	virtual bool		ProcessImage(byte* image, int32 width, int32 height, int32 bytesPerPixel) XTOVERRIDE;

	// Returns true if an image is currently being processed from a previous call to ProcessImage().  
	virtual bool		IsProcessingImage() const XTOVERRIDE;

private:

	void				ThreadFunc();

	// Returns the text string encoded in the QR Code in the given image.  
	// Returns an empty string on failure.  
	std::string			DecodeImage(const ImageDataPtr& image);

	ImageDataPtr		m_imageData;

	PairingInfo			m_pairingInfo;

	MemberFuncThreadPtr	m_thread;
	Mutex				m_mutex;
	Event				m_event;

	volatile int		m_stopping;
	std::atomic<bool>	m_bProcessing;
	std::atomic<bool>	m_bProcessingSuccessful;

};

XTOOLS_NAMESPACE_END