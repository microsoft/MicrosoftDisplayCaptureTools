#include "pch.h"
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

    // Identify the config file path
    auto cwd = std::filesystem::current_path();
    winrt::hstring configPath = winrt::hstring(cwd.c_str()) + L"\\Tests\\TestConfig.json";

    // Load the framework
    framework = winrt::Libraries::LoadInterfaceFromPath<winrt::Framework::ICore>
        (L"Core\\Core.dll", L"MicrosoftDisplayCaptureTools.Framework.Core");

    framework.LoadConfigFile(configPath.c_str());

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