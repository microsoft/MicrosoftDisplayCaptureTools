#include "pch.h"
#include "SingleScreenTestMatrix.h"

// Shared Utilities
#include "BinaryLoader.h"

#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Graphics.Imaging.h>

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

using namespace MicrosoftDisplayCaptureTools::Tests;
namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Devices::Display;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Storage;
    using namespace MicrosoftDisplayCaptureTools;
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework;
    using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
}

namespace MicrosoftDisplayCaptureTools::Tests
{
    winrt::IAsyncAction SaveFrameToDisk(winrt::IRawFrame frame, winrt::StorageFolder folder, winrt::hstring fileNamePrefix)
    {
        {
            auto filePathRaw = fileNamePrefix + L"_raw.hwhlk";
            auto file = co_await folder.CreateFileAsync(filePathRaw, winrt::CreationCollisionOption::ReplaceExisting);

            co_await winrt::FileIO::WriteBufferAsync(file, frame.Data());
        }

        auto renderableFrame = frame.try_as<winrt::IRawFrameRenderable>();
        if (renderableFrame)
        {
            auto softwareBitmap = co_await renderableFrame.GetRenderableApproximationAsync();
            if (!softwareBitmap)
            {
                winrt::Logger().LogAssert(L"GetRenderableApproximationAsync returned null but the frame class implemets IRawFrameRenderable.");
                co_return;
            }

            auto filePathImage = fileNamePrefix + L"_approximate.png";
            auto file = co_await folder.CreateFileAsync(filePathImage, winrt::CreationCollisionOption::ReplaceExisting);
            auto encoder = co_await winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(
                winrt::Windows::Graphics::Imaging::BitmapEncoder::PngEncoderId(), co_await file.OpenAsync(winrt::FileAccessMode::ReadWrite));
            encoder.SetSoftwareBitmap(softwareBitmap);
            co_await encoder.FlushAsync();
        }

        co_return;
    }

    winrt::IAsyncAction SaveFrameSetToDisk(winrt::IRawFrameSet frameset, winrt::hstring resultFolderPath, winrt::hstring testName)
    {
        auto cwd = co_await winrt::StorageFolder::GetFolderFromPathAsync(resultFolderPath);
        auto resultsFolder = co_await cwd.CreateFolderAsync(L"Results", winrt::CreationCollisionOption::OpenIfExists);

        unsigned long frameCounter = 0;
        std::vector<winrt::IAsyncAction> frameSaveActions;
        for (auto frame : frameset.Frames())
        {
            auto filePrefix = winrt::hstring(String().Format(L"%s_Frame_%d", testName.c_str(), frameCounter++));

            frameSaveActions.push_back(SaveFrameToDisk(frame, resultsFolder, filePrefix));
        }

        for (auto& frameSave : frameSaveActions)
        {
             co_await frameSave;
        }

        co_return;
    }
}

bool SingleScreenTestMatrix::Setup()
{
    return __super::Setup();
}

bool SingleScreenTestMatrix::Cleanup()
{
    for (auto& action : fileOperationsVector)
        action.get();

    return __super::Cleanup();
}

void SingleScreenTestMatrix::Test()
{
    // Lock the framework's set of loaded components
    auto frameworkLock = g_framework.LockFramework();
    VERIFY_IS_NOT_NULL(frameworkLock);

    winrt::IDisplayEngine displayEngine = nullptr;
    winrt::IConfigurationToolbox toolbox = nullptr;
    winrt::IDisplayOutput displayOutput = nullptr;
    winrt::IDisplayInput displayInput = nullptr;

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        auto displayEngines = g_framework.GetDisplayEngines();

        if (displayEngines.empty())
        {
             winrt::Logger().LogAssert(L"No DisplayEngines loaded.");
             return;
        }

        // This test only supports a single screen and so can only load a single DisplayEngine,
        // so select the highest version one installed.
        displayEngine = displayEngines[0];
        for (auto&& engine : displayEngines)
        {
             if (engine.Version().IsHigherVersion(displayEngine.Version()))
             {
                 displayEngine = engine;
             }
        }
    }

    auto toolboxes = g_framework.GetConfigurationToolboxes();

    if (toolboxes.empty())
    {
        winrt::Logger().LogAssert(L"No ConfigurationToolboxes loaded.");
        return;
    }

    // Only a single toolbox can be used for a single display test run currently, so select
    // the highest version toolbox installed.
    toolbox = toolboxes[0];
    for (auto&& box : toolboxes)
    {
        if (box.Version().IsHigherVersion(toolbox.Version()))
        {
             toolbox = box;
        }
    }

    std::vector<winrt::ConfigurationTools::IConfigurationTool> tools;
    for (auto&& toolbox : toolboxes)
    {
        auto toolList = toolbox.GetSupportedTools();

        for (auto toolName : toolList)
        {
             tools.push_back(toolbox.GetTool(toolName));
        }
    }

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        // Pick the display - capture pair to use for this test.
        VERIFY_IS_GREATER_THAN(g_displayMap.Size(), (uint32_t)0);
        String inputName;
        if (FAILED(TestData::TryGetValue(CaptureBoardInputSourceTableName, inputName)))
        {
             winrt::Logger().LogError(L"This test requires a TAEF table data source with names for the target inputs.");
             return;
        }

        winrt::ISourceToSinkMapping mapping = nullptr;
        for (auto&& map : g_displayMap)
        {
             if (0 == inputName.CompareNoCase(map.Sink().Name().c_str()))
             {
                 mapping = map;
                 break;
             }
        }

        if (!mapping)
        {
             winrt::Logger().LogError(winrt::hstring(L"Requested input name: ") + winrt::hstring(inputName) + winrt::hstring(L" was not found!"));
             return;
        }

        auto displayOutputTarget = mapping.Source();
        displayInput = mapping.Sink();

        displayOutput = displayEngine.InitializeOutput(displayOutputTarget);
        if (!displayOutput)
        {
             // We could not initialize the display output for whatever reason, there is no
             // point in testing this mapping.
             Log::Result(TestResults::Blocked);
             return;
        }
    }

    auto prediction = toolbox.CreatePrediction();
    if (!prediction)
    {
        // We could not initialize the display prediction for whatever reason, there is no
        // point in testing this mapping.
        Log::Result(TestResults::Blocked);
        return;
    }

    winrt::hstring testName = L"";
    testName =
        testName + (winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter) ? L"" : displayInput.Name()) + L"_";

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
                 if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
                     tool.ApplyToOutput(displayOutput);
                 tool.ApplyToPrediction(prediction);
                 testName = testName + tool.GetConfiguration() + L"_";
             }
        }
    }

    // Start generating the prediction at the same time as we start outputting.
    auto predictionDataAsync = prediction.FinalizePredictionAsync();
    winrt::IRawFrameSet predictionFrameSet = nullptr;

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        // Make sure the capture card is ready
        displayInput.FinalizeDisplayState();

        auto inputCaps = displayInput.GetCapabilities();
        inputCaps.ValidateAgainstDisplayOutput(displayOutput);

        winrt::IDisplayCapture capturedFrame = nullptr;

        {
             auto renderer = displayOutput.StartRender();
             if (!renderer)
             {
                 Log::Result(TestResults::Blocked, "Could not run this test due to mode incompatibilities.");
                 return;
             }

             std::this_thread::sleep_for(std::chrono::seconds(1));

             // Capture the frame.
             capturedFrame = displayInput.CaptureFrame();
             if (!capturedFrame)
             {
                 // CaptureFrame should log errors if there are any non-continuable issues.
                 Log::Error(L"Failed to capture a frame");
                 return;
             }
        }

        predictionFrameSet = predictionDataAsync.get();

        auto captureResult = capturedFrame.CompareCaptureToPrediction(testName, predictionFrameSet);

        auto resultsSaveSetting = winrt::RuntimeSettings().GetSettingValueAsString(SaveResultsSelection);
        if (resultsSaveSetting.empty() || SaveResultsSelectionOnError == resultsSaveSetting)
        {
            // Default mode - save data out on failure
            if (!captureResult)
            {
                SaveOutput(predictionFrameSet, testName + L"_Prediction");
                SaveOutput(capturedFrame.GetFrameData(), testName + L"_Capture");
            }
        }
        else if (SaveResultsSelectionAll == resultsSaveSetting)
        {
            // Save all results, regardless of success/failure
            SaveOutput(predictionFrameSet, testName + L"_Prediction");
            SaveOutput(capturedFrame.GetFrameData(), testName + L"_Capture");
        }
    }
    else
    {
        if (!predictionFrameSet)
        {
             // We synchronize on the data being generated, so that any errors that happen there will be flagged
             // synchronously in this test method. However, by default we will save the data asynchronously. This
             // helps make best use of the testing time with the physical devices.
             predictionFrameSet = predictionDataAsync.get();
        }

        SaveOutput(predictionFrameSet, testName + L"_Prediction");
    }
}

void SingleScreenTestMatrix::SaveOutput(winrt::IRawFrameSet frames, winrt::hstring name)
{
    auto resultFolderPath = winrt::hstring(std::wstring(std::filesystem::current_path()));
    auto savePredictionTask = SaveFrameSetToDisk(frames, resultFolderPath, name);

    if (winrt::RuntimeSettings().GetSettingValueAsBool(SynchronizeSavingPredictionToDisk))
    {
        savePredictionTask.get();
    }
    else
    {
        fileOperationsVector.push_back(savePredictionTask);
    }
}