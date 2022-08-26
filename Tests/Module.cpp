#include "pch.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace MicrosoftDisplayCaptureTools;
using namespace Windows::Devices::Display;
using namespace Windows::Devices::Display::Core;
using namespace Windows::Graphics::Imaging;
using namespace MicrosoftDisplayCaptureTools::Tests::Logging;
} // namespace winrt

winrt::Framework::Core g_framework{nullptr};
winrt::Framework::ILogger g_logger{nullptr};
winrt::IVector<winrt::Framework::ISourceToSinkMapping> g_displayMap;

namespace MicrosoftDisplayCaptureTools::Tests {

// clang-format off
BEGIN_MODULE()
	MODULE_PROPERTY(L"Area", L"Graphics")
	MODULE_PROPERTY(L"SubArea", L"Display")
    MODULE_PROPERTY(L"RunAs", L"Elevated")
END_MODULE()
// clang-format on

MODULE_SETUP(ModuleSetup)
{
    winrt::init_apartment();

    // Create WEX logger to log tests/results/errors
    g_logger = winrt::make<winrt::WEXLogger>().as<winrt::Framework::ILogger>();

    // Identify the config file path
    auto cwd = std::filesystem::current_path();
    winrt::hstring defaultConfigPath = winrt::hstring(cwd.c_str()) + L"\\Tests\\TestConfig.json";

    String configPath = defaultConfigPath.c_str();
    RuntimeParameters::TryGetValue(ConfigFileRuntimeParameter, configPath);

    // Load the framework
    g_framework = winrt::Framework::Core(g_logger);
    g_framework.LoadConfigFile(static_cast<const wchar_t*>(configPath));

    g_logger.LogNote(L"Loaded plugins");

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

            g_logger.LogNote((std::wstring(L"Firmware version detected:\n") + firmwareVersion).c_str());

            if (!disableFirmwareUpdate)
            {
                switch (firmwareState)
                {
                case winrt::CaptureCard::ControllerFirmwareState::ManualUpdateNeeded:
                    g_logger.LogError(
                        L"The capture device requires a manual firmware update, or the firmware version cannot be identified.");
                    return false;

                case winrt::CaptureCard::ControllerFirmwareState::UpdateRequired:
                    g_logger.LogNote(L"The capture device requires a firmware update. Starting update...");

                    try
                    {
                        // Trigger a firmware update synchronously
                        firmwareInterface.UpdateFirmwareAsync().get();

                        firmwareVersion = firmwareInterface.FirmwareVersion();
                        g_logger.LogNote((std::wstring(L"Successfully updated to new firmware version:\n") + firmwareVersion).c_str());
                    }
                    catch (...)
                    {
                        g_logger.LogError(L"Failed to update capture device firmware. Please manually update the firmware and restart "
                                   L"the test.");
                        return false;
                    }
                    break;

                case winrt::CaptureCard::ControllerFirmwareState::UpdateAvailable:
                    g_logger.LogWarning(
                        L"A newer firmware version is available for the capture device but is not required. For best results, "
                        L"consider upgrading firmware.");
                    break;

                case winrt::CaptureCard::ControllerFirmwareState::UpToDate:
                    g_logger.LogNote(L"The capture device firmware is up to date!");
                    break;
                }
            }
        }
    }

    auto frameworkLock = g_framework.LockFramework();
    if (!frameworkLock)
    {
        g_logger.LogError(L"Unable to lock the framework during test setup.");
    }

    // First see if the config file contained any display mappings, if so we will use these.
    auto mappings = g_framework.GetSourceToSinkMappings(false);
    if (mappings.Size() == 0)
    {
        // if no display mappings were in the config file - attempt to figure out the mappings automatically
        mappings = g_framework.GetSourceToSinkMappings(true);
    }

	return true;
}

MODULE_CLEANUP(ModuleCleanup)
{
    return true;
}

} // namespace MicrosoftDisplayCaptureTools::Tests