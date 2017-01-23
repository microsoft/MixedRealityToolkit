//////////////////////////////////////////////////////////////////////////
// ClientWrapperAPI.h
//
// Starting point for SWIG to generate language wrappers for the XTools client library
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Common/WrapperAPIs.h"
#include "Settings.h"
#include "PairingResult.h"
#include "PairMaker.h"
#include "PairingListener.h"
#include "PairingManager.h"
#include "SharingManager.h"

// Direct Pairing
#include "DirectPairConnector.h"
#include "DirectPairReceiver.h"

// Visual Pairing
#include "TagImage.h"
#include "VisualPairReceiver.h"
#include "VisualPairConnector.h"

//Discovery
#include "../Common/Public/SystemRole.h"
#include "../Common/Public/DiscoveredSystem.h"
#include "../Common/Public/DiscoveryClientListener.h"
#include "../Common/Public/DiscoveryClient.h"