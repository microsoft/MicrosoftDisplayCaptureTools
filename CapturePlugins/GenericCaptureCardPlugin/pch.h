#pragma once
#include <iostream>
#include <format>
#include <mutex>

#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Storage.h>

// Include the headers for the MediaCapture APIs
#include <winrt/Windows.Media.Capture.h>
#include <winrt/Windows.Media.Capture.Core.h>
#include <winrt/Windows.Media.MediaProperties.h>

// Include the headers for various graphics APIs
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Devices.Display.h>
#include <winrt/Windows.Devices.Display.Core.h>

// Include the DeviceInformation APIs
#include <winrt/Windows.Devices.Enumeration.h>

// Include the storage stream APIs
#include <winrt/Windows.Storage.Streams.h>

// Include the headers for the Imaging APIs
#include <winrt/Windows.Graphics.Imaging.h>

// Include the headers for the displayengine components of these tools
#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

// Include the header for the current test runtime
#include "TestRuntime.h"

// Include the D3D headers for doing shader-based math
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
#include <DirectXMath.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3d11shader.h>

// Include the header for processing frame data into consistent formats
#include "FrameProcessor.h"

// Include the header for the Generic Capture Card Plugin
#include "GenericCaptureCardPlugin.h"

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;