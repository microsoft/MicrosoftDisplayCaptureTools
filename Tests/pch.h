#pragma once

// General headers
#include <hstring.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>

// TAEF headers
#include <Wex.Common.h>
#include <Wex.Logger.h>
#include <WexString.h>
#include <WexTestClass.h>
#include <WexLogTrace.h>
#include <WexTypes.h>
#include <LogController.h>
#include <Log.h>

// WinRT headers
#include <winrt/Windows.Foundation.Collections.h>

// Project headers
#include "winrt/Windows.Foundation.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"
#include "../Shared/Inc/BinaryLoader.h"

// Local headers
#include "TestSettings.h"
#include "Logger.h"