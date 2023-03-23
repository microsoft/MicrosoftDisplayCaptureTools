#pragma once
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Devices.Display.h>
#include <winrt/Windows.Devices.Display.Core.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Json.h>

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Usb.h>

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <iterator>

#include "IteIt68051.h"

#include "Controller.h"
#include "TanagerDevice.h"

#define TCA6416A_BANK_0 0
#define TCA6416A_BANK_1 1
