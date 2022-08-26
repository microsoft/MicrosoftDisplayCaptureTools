#include "pch.h"
#include "SingleScreenTestMatrix.h"

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
    using namespace MicrosoftDisplayCaptureTools::Framework;
}

bool SingleScreenTestMatrix::Setup()
{
    return __super::Setup();
}

bool SingleScreenTestMatrix::Cleanup()
{
    return __super::Cleanup();
}

void SingleScreenTestMatrix::Test()
{
    // Lock the framework's set of loaded components
    auto frameworkLock = g_framework.LockFramework();
    VERIFY_IS_NOT_NULL(frameworkLock);

    // Pick the display - capture pair to use for this test.
    VERIFY_IS_GREATER_THAN(g_displayMap.Size(), (uint32_t)0);
    winrt::ISourceToSinkMapping mapping = g_displayMap.GetAt(0);

    auto displayEngine = mapping.Source();
    auto displayInput = mapping.Sink();
    
    winrt::hstring testName = L"";
    auto tools = g_framework.GetLoadedTools();
    for (auto tool : tools)
    {
        String toolSetting;
        if (SUCCEEDED(TestData::TryGetValue(tool.Name().c_str(), toolSetting)))
        {
            String output = L"";

            // Setting the tool value
            tool.SetConfiguration(winrt::hstring(toolSetting));
            tool.Apply(displayEngine);
            testName = testName + tool.GetConfiguration() + L"_";
        }
    }

    // Make sure the capture card is ready
    displayInput.FinalizeDisplayState();

    {
        auto renderer = displayEngine.StartRender();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Capture the frame.
        auto capturedFrame = displayInput.CaptureFrame();
        auto predictedFrame = displayEngine.GetPrediction();

        capturedFrame.CompareCaptureToPrediction(testName, predictedFrame);
    }
}