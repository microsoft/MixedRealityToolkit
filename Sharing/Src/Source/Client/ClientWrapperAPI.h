// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ClientWrapperAPI.h
// Starting point for SWIG to generate language wrappers for the XTools client library
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Common/WrapperAPIs.h"
#include "Settings.h"
#include "PairingResult.h"
#include "PairMaker.h"
#include "PairingListener.h"
#include "PairingManager.h"
#include "SharingManager.h"

// Image tag processing
#include "ImageTagManager.h"
#include "ImageTagLocation.h"
#include "ImageTagLocationListener.h"

// Direct Pairing
#include "DirectPairConnector.h"
#include "DirectPairReceiver.h"

// Visual Pairing
#include "TagImage.h"
#include "VisualPairReceiver.h"
#include "VisualPairConnector.h"
