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

    auto displayEngines = g_framework.GetDisplayEngines();

    if (displayEngines.empty())
    {
        g_logger.LogAssert(L"No DisplayEngines loaded.");
    }

    // This test only supports a single screen and so can only load a single DisplayEngine,
    // so select the highest version one installed.
    winrt::IDisplayEngine displayEngine = displayEngines[0];
    for (auto&& engine : displayEngines)
    {
        if (engine.Version().IsHigherVersion(displayEngine.Version()))
        {
            displayEngine = engine;
        }
    }

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

    // Pick the display - capture pair to use for this test.
    VERIFY_IS_GREATER_THAN(g_displayMap.Size(), (uint32_t)0);

    for (auto&& mapping : g_displayMap)
    {
        auto displayOutputTarget = mapping.Source();
        auto displayInput = mapping.Sink();

        auto displayOutput = displayEngine.InitializeOutput(displayOutputTarget);
        if (!displayOutput)
        {
            // We could not initialize the display output for whatever reason, there is no
            // point in testing this mapping.
            continue;
        }

        auto prediction = displayEngine.CreateDisplayPrediction();
        if (!prediction)
        {
            // We could not initialize the display prediction for whatever reason, there is no
            // point in testing this mapping.
            continue;
        }

        winrt::hstring testName = displayInput.Name() + L"_";

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
            if (!capturedFrame)
            {
                // CaptureFrame should log errors if there are any non-continuable issues.
                continue;
            }

            capturedFrame.CompareCaptureToPrediction(testName, predictionDataAsync.get());
        }
    }
}