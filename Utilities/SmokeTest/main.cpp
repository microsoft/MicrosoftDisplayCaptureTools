#include "pch.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace MicrosoftDisplayCaptureTools;
    using namespace Windows::Devices::Display;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Data::Json;
}

//
// There are a few options to configure the test settings and to set up the framework to handle particular situations. Several
// are implemented here for illustrative purposes, but they are functionally equivalent.
// 
// 1. Load a config file (json) from disk
// 2. Create a config file in memory
// 3. Directly interact with with framework
//
enum class Scenario { LoadFromDisk, CreateInMemory, DirectSet };
auto currentScenario = Scenario::DirectSet;

int main()
{
    winrt::init_apartment();

    // Load the framework
    auto core = winrt::Framework::Core();

    switch (currentScenario)
    {
    case Scenario::CreateInMemory: // Create a config file in code to specify test settings
        {
            auto testConfig = winrt::JsonObject();
            auto testComponents = winrt::JsonObject::JsonObject();

            // Specify the capture card plugin
            auto capturePlugin = winrt::JsonObject::JsonObject();
            capturePlugin.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"GenericCaptureCardPlugin\\GenericCaptureCardPlugin.dll"));
            capturePlugin.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"GenericCaptureCardPlugin.ControllerFactory"));
            capturePlugin.Insert(L"Settings", winrt::JsonValue::CreateNullValue());
            testComponents.Insert(L"CapturePlugin", capturePlugin);

            // Specify the DisplayEngine to load
            auto displayEngine = winrt::JsonObject::JsonObject();
            displayEngine.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"BasicDisplayControl\\BasicDisplayControl.dll"));
            displayEngine.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"DisplayControl.DisplayEngineFactory"));
            displayEngine.Insert(L"Settings", winrt::JsonValue::CreateNullValue());
            testComponents.Insert(L"DisplayEngine", displayEngine);

            // Specify the ConfigurationToolbox to load
            auto configToolboxArray = winrt::JsonArray::JsonArray();
            auto configToolbox = winrt::JsonObject::JsonObject();
            configToolbox.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"BasicDisplayConfiguration\\BasicDisplayConfiguration.dll"));
            configToolbox.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"DisplayConfiguration.ToolboxFactory"));
            configToolbox.Insert(L"Settings", winrt::JsonValue::CreateNullValue());
            configToolboxArray.Append(configToolbox);
            testComponents.Insert(L"ConfigurationToolboxes", configToolboxArray);

            testConfig.SetNamedValue(L"HardwareHLK", testComponents);

            core.LoadConfigFile(testConfig.Stringify());
        }
        break;

    case Scenario::LoadFromDisk: // Load the config file from disk
        {
            auto cwd = std::filesystem::current_path();
            winrt::hstring fullPath = winrt::hstring(cwd.c_str()) + L"\\SmokeTest\\BasicConfig.json";
            core.LoadConfigFile(fullPath.c_str());
        }
        break;

    case Scenario::DirectSet: // Directly specify to the framework the components to use.
        {
            // Tell the framework to load the components.
            core.LoadCapturePlugin(L"GenericCaptureCardPlugin\\GenericCaptureCardPlugin.dll", L"GenericCaptureCardPlugin.ControllerFactory");
            core.LoadToolbox(L"BasicDisplayConfiguration\\BasicDisplayConfiguration.dll", L"BasicDisplayConfiguration.ToolboxFactory");
            core.LoadDisplayManager(L"BasicDisplayControl\\BasicDisplayControl.dll", L"DisplayControl.DisplayEngineFactory");
        }
        break;
    }

    // If the engine does not already have a display target (loaded from the config file), ask
    // the user to specify one
    auto mappings = core.GetSourceToSinkMappings(true);
    winrt::Framework::ISourceToSinkMapping displayMapping = mappings.Size() > 0 ? mappings.GetAt(0) :nullptr;

    // Tell the capture card to finalize any state on this input. After this call returns the display should be
    // visible to windows and ready for output.
    displayMapping.GetSink().FinalizeDisplayState();

    auto caps = displayMapping.GetSource().GetCapabilities();
    auto modes = caps.GetSupportedModes();

    winrt::DisplayModeInfo bestMode{ nullptr };
    double bestModeDiff = INFINITY;
    for (auto&& mode : modes)
    {
        if (mode.SourcePixelFormat() == winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R8G8B8A8UIntNormalized &&
            mode.IsInterlaced() == false &&
            mode.TargetResolution().Height == 1080 &&
            mode.SourceResolution().Height == 1080)
        {
            auto vSync = mode.PresentationRate().VerticalSyncRate;
            double vSyncDouble = (double)vSync.Numerator / vSync.Denominator;

            double modeDiff = abs(vSyncDouble - 60);
            if (modeDiff < bestModeDiff)
            {
                bestMode = mode;
                bestModeDiff = modeDiff;
            }
        }
    }

    if (!bestMode) throw winrt::hresult_error();

    displayMapping.GetSource().GetProperties().ActiveMode(bestMode);
    displayMapping.GetSource().GetProperties().GetPlaneProperties()[0].ClearColor({1.0f, 0.0f, 0.0f});

    auto render = displayMapping.GetSource().StartRender();

    // Actually capture a frame
    auto capturedFrame = displayMapping.GetSink().CaptureFrame();

    auto prediction = displayMapping.GetSource().GetPrediction();

    capturedFrame.CompareCaptureToPrediction(L"BasicTest", prediction);

    // Wait for user to kill the process
    std::wcout << "\nPress enter to exit\n";
    (void)getchar();
    
    if (render) render.Close();
}
