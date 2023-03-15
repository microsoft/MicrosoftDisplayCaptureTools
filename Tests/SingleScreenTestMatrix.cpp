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
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework;
    using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
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

    auto displayOutputTarget = mapping.Source();
    auto displayInput = mapping.Sink();

    auto displayEngines = g_framework.GetDisplayEngines();

    if (displayEngines.empty())
    {
        g_logger.LogAssert(L"No DisplayEngines loaded.");
    }

    winrt::IDisplayEngine displayEngine = displayEngines[0];

    // This test only supports a single screen and so can only load a single DisplayEngine,
    // so select the highest version one installed.
    for (auto&& engine : displayEngines)
    {
        if (engine.Version().IsHigherVersion(displayEngine.Version()))
        {
            displayEngine = engine;
        }
    }

    auto displayOutput = displayEngine.InitializeOutput(displayOutputTarget);
    auto prediction = displayEngine.CreateDisplayPrediction();
    
    winrt::hstring testName = L"";
    auto toolboxes = g_framework.GetConfigurationToolboxes();
    std::vector<winrt::ConfigurationTools::IConfigurationTool> tools;

    for (auto&& toolbox : toolboxes)
    {
        auto toolList = toolbox.GetSupportedTools();

        for (auto toolName : toolList)
        {
            tools.push_back(toolbox.GetTool(toolName));
        }
    }

    // All tools need to be run in order of their category
    constexpr winrt::ConfigurationToolCategory categoryOrder[] = {
        winrt::ConfigurationToolCategory::DisplaySetup, winrt::ConfigurationToolCategory::RenderSetup, winrt::ConfigurationToolCategory::Render};

    for (auto& category : categoryOrder)
    {
        for (auto tool : tools)
        {
            if (tool.Category() != category)
                continue;

            String toolSetting;
            if (SUCCEEDED(TestData::TryGetValue(tool.Name().c_str(), toolSetting)))
            {
                String output = L"";

                // Setting the tool value
                tool.SetConfiguration(winrt::hstring(toolSetting));
                tool.ApplyToOutput(displayOutput);
                tool.ApplyToPrediction(prediction);
                testName = testName + tool.GetConfiguration() + L"_";
            }
        }
    }

    // Start generating the prediction at the same time as we start outputting.
    auto predictionDataAsync = prediction.GeneratePredictionDataAsync();

    // Make sure the capture card is ready
    displayInput.FinalizeDisplayState();

    {
        auto renderer = displayOutput.StartRender();
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Capture the frame.
        auto capturedFrame = displayInput.CaptureFrame();

        capturedFrame.CompareCaptureToPrediction(testName, predictionDataAsync.get());
    }
}