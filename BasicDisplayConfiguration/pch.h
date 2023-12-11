#pragma once

// Normally memcpy_s is defined as `static __inline`, but it needs to be defined as only `__inline` in order to get external linkage from a header unit
#define _CRT_MEMCPY_S_INLINE __inline

#include <unknwn.h>

// C++/WinRT headers
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Devices.Display.h>
#include <winrt/Windows.Devices.Display.Core.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include "winrt/Windows.Foundation.Numerics.h"
#include "winrt/Microsoft.Graphics.Canvas.h"
#include "winrt/Microsoft.Graphics.Canvas.Effects.h"
#include "winrt/Microsoft.Graphics.Canvas.Brushes.h"
#include "winrt/Windows.Graphics.DirectX.Direct3D11.h"
#include "winrt/Windows.UI.h"
#include "winrt/Windows.Security.Cryptography.h"
#include "winrt/Windows.Security.Cryptography.Core.h"

#include "winrt/MicrosoftDisplayCaptureTools.Display.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"

#include "TestRuntime.h"

#include <map>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <strsafe.h>
#include <memory>
#include <functional>
#include <string>
#include <sstream>
#include <concepts>
#include <format>

// Windows headers
#include <Windows.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <d2d1_3.h>
#include <MemoryBuffer.h>

// Interop headers
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
// #include <Microsoft.Graphics.Canvas.h> // Can't be included without WinAppSDK
#include <Microsoft.Graphics.Canvas.native.h>

namespace ABI::Microsoft::Graphics::Canvas {

// Declare the ICanvasDevice interface since we can't include the real Microsoft.Graphics.Canvas.h header
MIDL_INTERFACE("A27F0B5D-EC2C-4D4F-948F-0AA1E95E33E6")
ICanvasDevice : public ::IInspectable{};

} // namespace ABI::Microsoft::Graphics::Canvas
