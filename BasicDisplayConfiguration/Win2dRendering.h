#pragma once

#include <unknwn.h>

// C++/WinRT headers
#include "winrt/Windows.Foundation.Numerics.h"
#include "winrt/Microsoft.Graphics.Canvas.h"
#include "winrt/Microsoft.Graphics.Canvas.Brushes.h"
#include "winrt/Windows.Devices.Display.Core.h"
#include "winrt/Windows.Graphics.DirectX.Direct3D11.h"
#include "winrt/Windows.UI.h"

// Windows headers
#include <dxgi.h>

// Interop headers
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
// #include <Microsoft.Graphics.Canvas.h> // Can't be included without WinAppSDK
#include <Microsoft.Graphics.Canvas.native.h>

namespace ABI::Microsoft::Graphics::Canvas {

// Declare the ICanvasDevice interface since we can't include the real Microsoft.Graphics.Canvas.h header
MIDL_INTERFACE("A27F0B5D-EC2C-4D4F-948F-0AA1E95E33E6")
ICanvasDevice : public ::IInspectable{};

} // namespace ABI::Microsoft::Graphics::Canvas
