#pragma once
#include <unknwn.h>
#include <Windows.h>
#include <hstring.h>
#include <winstring.h>
#include <MemoryBuffer.h>
#include <iostream>
#include <string>
#include <sstream>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Devices.Display.h>
#include <winrt/Windows.Devices.Display.Core.h>

// Headers for the GPU-Accelerated reference image rendering
#include <windows.ui.xaml.media.dxinterop.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dxgi.h>

// Headers for this project's various components
#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.DisplayStateReference.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "StaticReferenceData.h"

// Headers for accessing TAEF parameters & logging with TAEF
#include <WexTestClass.h>
#include <Wex.Common.h>
#include <Wex.Logger.h>
#include <WexString.h>