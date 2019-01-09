////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#define NOMINMAX


// SDL
#include <optional>
#include <tuple>
#include <future>

#include <array>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <mutex>
#include <sstream>
#include <functional>

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdint.h>

#include <d2d1_2.h>
#include <d3d11_4.h>
#include <DirectXColors.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <WindowsNumerics.h>
#include <Windows.Graphics.Directx.Direct3D11.Interop.h>

#include <wrl\client.h>

#include <ppl.h>
#include <winrt\Windows.ApplicationModel.Activation.h>
#include <winrt\Windows.ApplicationModel.Core.h>
#include <winrt\Windows.Devices.h>
#include <winrt\Windows.Devices.haptics.h>
#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>
#include <winrt\Windows.Gaming.Input.h>
#include <winrt\Windows.Graphics.Display.h>
#include <winrt\Windows.Graphics.Holographic.h>
#include <winrt\Windows.Perception.People.h>
#include <winrt\Windows.Perception.Spatial.h>
#include <winrt\Windows.Storage.h>
#include <winrt\Windows.Storage.Streams.h>
#include <winrt\Windows.UI.Core.h>
#include <winrt\Windows.UI.Input.Spatial.h>
