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
auto currentScenario = Scenario::LoadFromDisk;

int main()
{
    winrt::init_apartment();

    // Load the framework
    auto core = winrt::Libraries::LoadInterfaceFromPath<winrt::Framework::ICore>(
        L"Core\\Core.dll", L"MicrosoftDisplayCaptureTools.Framework.Core");

    switch (currentScenario)
    {
    case Scenario::CreateInMemory: // Create a config file in code to specify test settings
        {
            auto testConfig = winrt::JsonObject();
            auto testComponents = winrt::JsonObject::JsonObject();

            // Specify the capture card plugin
            auto capturePlugin = winrt::JsonObject::JsonObject();
            capturePlugin.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"GenericCaptureCardPlugin\\GenericCaptureCardPlugin.dll"));
            capturePlugin.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"GenericCaptureCardPlugin.Plugin"));
            capturePlugin.Insert(L"Settings", winrt::JsonValue::CreateNullValue());
            testComponents.Insert(L"CapturePlugin", capturePlugin);

            // Specify the DisplayEngine to load
            auto displayEngine = winrt::JsonObject::JsonObject();
            displayEngine.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"BasicDisplayControl\\BasicDisplayControl.dll"));
            displayEngine.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"DisplayControl.DisplayEngine"));
            displayEngine.Insert(L"Settings", winrt::JsonValue::CreateNullValue());
            testComponents.Insert(L"DisplayEngine", displayEngine);

            // Specify the ConfigurationToolbox to load
            auto configToolboxArray = winrt::JsonArray::JsonArray();
            auto configToolbox = winrt::JsonObject::JsonObject();
            configToolbox.Insert(L"Path", winrt::JsonValue::CreateStringValue(L"CommonToolbox\\CommonToolbox.dll"));
            configToolbox.Insert(L"Class", winrt::JsonValue::CreateStringValue(L"DisplayConfiguration.Toolbox"));
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
            core.LoadPlugin(L"GenericCaptureCardPlugin\\GenericCaptureCardPlugin.dll", L"GenericCaptureCardPlugin.Plugin");
            core.LoadToolbox(L"CommonToolbox\\CommonToolbox.dll", L"DisplayConfiguration.Toolbox");
            core.LoadDisplayManager(L"BasicDisplayControl\\BasicDisplayControl.dll", L"DisplayControl.DisplayEngine");
        }
        break;
    }

    // Set up the capture card and get the first input from it.
    auto genericCapture = core.GetCaptureCard();

    // In this 'smoketest' project, we expect the first enumerated input to match the one indicated by the config file.
    auto captureInput = genericCapture.EnumerateDisplayInputs()[0];

    // Tell the capture card to finalize any state on this input. After this call returns the display should be
    // visible to windows and ready for output.
    captureInput.FinalizeDisplayState();

    // If the engine does not already have a display target (loaded from the config file), ask
    // the user to specify one
    auto displayEngine = core.GetDisplayEngine();
    if (!displayEngine.GetTarget())
    {
        // Ask the user to indicate the display to use.
        std::wcout << "Connected Displays:\n";
        int index = 0;
        auto manager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);
        auto allTargets = manager.GetCurrentTargets();
        auto connectedTargets = winrt::single_threaded_map<int32_t, winrt::DisplayTarget>();

        for (auto&& target : allTargets)
        {
            if (target.IsConnected())
            {
                connectedTargets.Insert(index++, target);
            }
        }

        for (auto&& target : connectedTargets)
        {
            std::wcout << L"\t" << target.Key() + 1 << L". " << target.Value().StableMonitorId().c_str() << std::endl;
        }

        int selection = -1;
        while (1)
        {
            std::wcout << L"\n Enter a display selection: ";
            std::cin >> selection;
            (void)getchar(); // eat the newline

            if (connectedTargets.HasKey(selection - 1)) break;
            else
            {
                std::wcout << L"\nInvalid choice: " << selection << std::endl;
            }
        }

        std::wcout << "\nChosen Display: " << connectedTargets.Lookup(selection - 1).StableMonitorId().c_str() << std::endl;

        displayEngine.InitializeForDisplayTarget(connectedTargets.Lookup(selection - 1));
    }

    auto caps = displayEngine.GetCapabilities();
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

    displayEngine.GetProperties().ActiveMode(bestMode);
    displayEngine.GetProperties().GetPlaneProperties()[0].ClearColor({ 1.0f,0.0f,0.0f });

    auto render = displayEngine.StartRender();

    // Actually capture a frame
    auto capturedFrame = captureInput.CaptureFrame();

    auto prediction = displayEngine.GetPrediction();

    capturedFrame.CompareCaptureToPrediction(L"BasicTest", prediction);

    // Wait for user to kill the process
    std::wcout << "\nPress enter to exit\n";
    (void)getchar();
    
    if (render) render.Close();
}
