/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace RakNet_WindowsStore8_VS2012;

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "MessageIdentifiers.h"
using namespace RakNet;
//#define DEFAULT_SERVER_PORT 61111
#define DEFAULT_SERVER_PORT 20603
//#define DEFAULT_SERVER_ADDRESS "natpunch.jenkinssoftware.com"
//#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define DEFAULT_SERVER_ADDRESS "localhost"
#define MAX_CONNECTIONS 10

MainPage::MainPage()
{
	InitializeComponent();


// #error "add raknet init code, what happened to windows phone?"

	RakPeerInterface *rakPeer = RakPeerInterface::GetInstance();

	RakNet::SocketDescriptor socketDescriptor;
	socketDescriptor.port = DEFAULT_SERVER_PORT;
	socketDescriptor.socketFamily = AF_INET; // IPV4

	// Startup the peer interface with the given socket settings.  Will fail if we already have a peer interface for this host
	RakNet::StartupResult startupResult = rakPeer->Startup(MAX_CONNECTIONS, &socketDescriptor, 1);
	assert(startupResult == RAKNET_STARTED);

	// Calling this function starts the peer listening for incoming connections
	rakPeer->SetMaximumIncomingConnections(MAX_CONNECTIONS);


	Packet *packet;
	do 
	{
		packet = rakPeer->Receive();
		Sleep(1);
	} 
	while (!packet || packet->data[0] != ID_NEW_INCOMING_CONNECTION);

	//RakAssert(packet->data[0] == ID_NEW_INCOMING_CONNECTION);
	

	//RakPeerInterface *rakPeer = RakPeerInterface::GetInstance();
	//SocketDescriptor sd;
	//StartupResult sr = rakPeer->Startup(1, &sd, 1);
	//assert(sr==RAKNET_STARTED);
	//ConnectionAttemptResult car = rakPeer->Connect(DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT, 0, 0);
	//assert(car==CONNECTION_ATTEMPT_STARTED);
	//RakSleep(1000);
	//Packet *packet;
	//packet=rakPeer->Receive();
	//if (packet)
	//{
	//	RakAssert(packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED);
	//}

}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter
}
