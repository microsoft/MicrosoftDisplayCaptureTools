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
            tool.Apply(m_displayEngine);
            testName = testName + tool.GetConfiguration() + L"_";
        }
    }

    // Make sure the capture card is ready
    // TODO: This should use the configured path mapping, instead of guessing input 0.
    auto captureCard = g_framework->GetCaptureCard();
    auto captureInput = captureCard.EnumerateDisplayInputs()[0];
    captureInput.FinalizeDisplayState();

    // Reset the display manager to the correct
    auto displayId = m_targetMap[captureInput.Name()];
    m_displayEngine.InitializeForStableMonitorId(displayId);

    {
        auto renderer = m_displayEngine.StartRender();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Capture the frame.
        auto capturedFrame = captureInput.CaptureFrame();
        auto predictedFrame = m_displayEngine.GetPrediction();

        // TODO: build a uniquely identifying string from the currently selected tools

        capturedFrame.CompareCaptureToPrediction(testName, predictedFrame);
    }
}