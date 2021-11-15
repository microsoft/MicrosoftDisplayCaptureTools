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

// Project headers
#include "winrt/Windows.Foundation.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"

#include "HardwareVerificationTests.h"

// Shared Utilities
#include "BinaryLoader.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace MicrosoftDisplayCaptureTools;
    using namespace Windows::Devices::Display;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Graphics::Imaging;
}

bool PictTests::Setup()
{
    winrt::init_apartment();
    
    // Load the framework
    framework = winrt::Libraries::LoadInterfaceFromPath<winrt::Framework::ICore>(
        L"Core\\Core.dll", L"MicrosoftDisplayCaptureTools.Framework.Core");

    // Load the config file from disk
    auto cwd = std::filesystem::current_path();
    winrt::hstring fullPath = winrt::hstring(cwd.c_str()) + L".\\BasicConfig.json";
    framework.LoadConfigFile(fullPath.c_str());

    Log::Comment(L"Initialization Complete");
    
    return true;
}

bool PictTests::Cleanup()
{
    return true;
}

void PictTests::Test()
{
    auto tools = framework.GetLoadedTools();
    for (auto tool : tools)
    {
        String toolSetting;
        if (SUCCEEDED(TestData::TryGetValue(tool.Name().c_str(), toolSetting)))
        {
            String output = L"";

            // Setting the tool value
            tool.SetConfiguration(winrt::hstring(toolSetting));
        }
    }

    framework.RunTest();
}