#include "pch.h"
#include "PictTests.h"

// Shared Utilities
#include "BinaryLoader.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Devices::Display;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Graphics::Imaging;
    using namespace MicrosoftDisplayCaptureTools;
}

bool PictTests::Setup()
{
    return __super::Setup();
}

bool PictTests::Cleanup()
{
    return __super::Cleanup();
}

void PictTests::Test()
{
    auto tools = g_framework.GetLoadedTools();
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

    g_framework.RunTest();
}