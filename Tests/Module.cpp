#include "pch.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace MicrosoftDisplayCaptureTools;
    using namespace Windows::Devices::Display;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Graphics::Imaging;
    using namespace MicrosoftDisplayCaptureTools::Tests::Logging;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
} // namespace winrt

winrt::Framework::Core g_framework{nullptr};
winrt::IVector<winrt::Framework::ISourceToSinkMapping> g_displayMap;

winrt::Framework::IRuntimeSettings g_runtimeSettings{nullptr};

namespace MicrosoftDisplayCaptureTools::Tests {

BEGIN_MODULE()
    MODULE_PROPERTY(L"Area", L"Graphics")
    MODULE_PROPERTY(L"SubArea", L"Display")
    // TODO: Re-enable this
    //MODULE_PROPERTY(L"RunAs", L"Elevated")
END_MODULE()

MODULE_SETUP(ModuleSetup)
{
    winrt::init_apartment();

    // Load the framework with new instances of the logger and runtime settings types from this module.
    g_framework = winrt::Framework::Core(
        winrt::make<winrt::WEXLogger>().as<winrt::Framework::ILogger>(), winrt::make<RuntimeSettings::RuntimeSettings>());

    if (winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        winrt::Logger().LogNote(L"Running tests in prediction-only mode.");
    }

    // If the user specified a particular configuration file in the test command, use it. Otherwise,
    // the framework will auto-discover installed components.
    String configFile;
    if (SUCCEEDED(RuntimeParameters::TryGetValue<String>(ConfigFileRuntimeParameter, configFile)) && !String::IsNullOrEmpty(configFile))
    {
        g_framework.LoadConfigFile(static_cast<const wchar_t*>(configFile));
    }

    // DiscoverInstalledPlugins will only discover plugin categories that are not specified via configuration file.
    g_framework.DiscoverInstalledPlugins();

    winrt::Display::IDisplayEngine displayEngineForMapping = nullptr;
    winrt::ConfigurationTools::IConfigurationToolbox toolboxForMapping = nullptr;

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        auto displayEngines = g_framework.GetDisplayEngines();

        if (!displayEngines.empty())
        {
            // This displayEngine is only used for the purposes of identifying display mappings, any full implementation
            // of the interface should work for this.
            displayEngineForMapping = displayEngines[0];
        }

        auto toolboxes = g_framework.GetConfigurationToolboxes();

        if (!toolboxes.empty())
        {
            // This toolbox is only used for the purposes of identifying display mappings, any full implementation
            // of the interface should work for this.
            toolboxForMapping = toolboxes[0];
        }
    }

    winrt::Logger().LogNote(L"Loaded plugins");

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        bool disableFirmwareUpdate = false;
        RuntimeParameters::TryGetValue(DisableFirmwareUpdateRuntimeParameter, disableFirmwareUpdate);

        // Check for a firmware update
        auto captureCards = g_framework.GetCaptureCards();
        for (auto&& captureCard : captureCards)
        {
            if (auto firmwareInterface = captureCard.try_as<winrt::CaptureCard::IControllerWithFirmware>())
            {
                auto firmwareState = firmwareInterface.FirmwareState();
                auto firmwareVersion = firmwareInterface.FirmwareVersion();

                winrt::Logger().LogNote((std::wstring(L"Firmware version detected:\n") + firmwareVersion).c_str());

                if (!disableFirmwareUpdate)
                {
                    switch (firmwareState)
                    {
                    case winrt::CaptureCard::ControllerFirmwareState::ManualUpdateNeeded:
                        winrt::Logger().LogError(L"The capture device requires a manual firmware update, or the firmware version cannot "
                                          L"be identified.");
                        return false;

                    case winrt::CaptureCard::ControllerFirmwareState::UpdateRequired:
                        winrt::Logger().LogNote(L"The capture device requires a firmware update. Starting update...");

                        try
                        {
                            // Trigger a firmware update synchronously
                            firmwareInterface.UpdateFirmwareAsync().get();

                            firmwareVersion = firmwareInterface.FirmwareVersion();
                            winrt::Logger().LogNote((std::wstring(L"Successfully updated to new firmware version:\n") + firmwareVersion).c_str());
                        }
                        catch (...)
                        {
                            winrt::Logger().LogError(
                                L"Failed to update capture device firmware. Please manually update the firmware and restart "
                                L"the test.");
                            return false;
                        }
                        break;

                    case winrt::CaptureCard::ControllerFirmwareState::UpdateAvailable:
                        winrt::Logger().LogWarning(L"A newer firmware version is available for the capture device but is not required. "
                                            L"For best results, "
                                            L"consider upgrading firmware.");
                        break;

                    case winrt::CaptureCard::ControllerFirmwareState::UpToDate:
                        winrt::Logger().LogNote(L"The capture device firmware is up to date!");
                        break;
                    }
                }
            }
        }
    }

    auto frameworkLock = g_framework.LockFramework();
    if (!frameworkLock)
    {
        winrt::Logger().LogError(L"Unable to lock the framework during test setup.");
    }

    if (!winrt::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        // First see if the config file contained any display mappings, if so we will use these.
        g_displayMap = g_framework.GetSourceToSinkMappings(false, displayEngineForMapping, toolboxForMapping);
        if (g_displayMap.Size() == 0)
        {
            winrt::Logger().LogNote(
                L"No display output to display capture device mapping from the configuration file - attempting to auto-map.");

            // if no display mappings were in the config file - attempt to figure out the mappings automatically
            // This uses the normal test mechanisms to control and render to displays - so if this fails normally it can be safely ignored.
            g_displayMap = g_framework.GetSourceToSinkMappings(true, displayEngineForMapping, toolboxForMapping);

            if (g_displayMap.Size() == 0)
            {
                winrt::Logger().LogError(
                    L"Unable to determine any display output to display capture device mappings - tests cannot continue.");
            }
        }
    }

    return true;
}

MODULE_CLEANUP(ModuleCleanup)
{
    // enforce the order of main object cleanup
    g_displayMap = nullptr;
    g_framework = nullptr;

    return true;
}

} // namespace MicrosoftDisplayCaptureTools::Tests