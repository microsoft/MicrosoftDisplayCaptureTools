#pragma once
// WinRT Headers
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Devices.Display.h>
#include <winrt/Windows.Devices.Display.Core.h>
#include <winrt/Windows.Graphics.Imaging.h>

// STD headers
#include <mutex>
#include <filesystem>
#include <fstream>

// Windows Headers
#include <Windows.h>
#include <hstring.h>

// WIL Headers
#include <wil\resource.h>
#include <wil\result.h>
#include <wil\cppwinrt.h>

// Shared Utilities
#include "BinaryLoader.h"
#include "TestRuntime.h"

// Component Headers
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"

// Internal classes
#include "EDIDDescriptor.h"
#include "Logger.h"