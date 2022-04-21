#include "pch.h"
#include "CaptureFrameworkTestBase.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt {
using namespace Windows::Foundation;
using namespace MicrosoftDisplayCaptureTools;
using namespace Windows::Devices::Display;
using namespace Windows::Devices::Display::Core;
using namespace Windows::Graphics::Imaging;
} // namespace winrt

bool CaptureFrameworkTestBase::Setup()
{
    // Identify the config file path
    auto cwd = std::filesystem::current_path();
    winrt::hstring defaultConfigPath = winrt::hstring(cwd.c_str()) + L"\\Tests\\TestConfig.json";

    String configPath = defaultConfigPath.c_str();
    RuntimeParameters::TryGetValue(ConfigFileRuntimeParameter, configPath);

    // Load the framework
    m_framework = winrt::Framework::Core();
    m_framework.LoadConfigFile(static_cast<const wchar_t*>(configPath));

    Log::Comment(L"Loaded plugins");

    bool disableFirmwareUpdate = false;
    RuntimeParameters::TryGetValue(DisableFirmwareUpdateRuntimeParameter, disableFirmwareUpdate);

    // Check for a firmware update
    auto captureCard = m_framework.GetCaptureCard();
    if (auto firmwareInterface = captureCard.try_as<winrt::CaptureCard::IControllerWithFirmware>())
    {
        auto firmwareState = firmwareInterface.FirmwareState();
        auto firmwareVersion = firmwareInterface.FirmwareVersion();

        Log::Comment((std::wstring(L"Firmware version detected:\n") + firmwareVersion).c_str());

        if (!disableFirmwareUpdate)
        {
            switch (firmwareState)
            {
            case winrt::CaptureCard::ControllerFirmwareState::ManualUpdateNeeded:
                Log::Error(
                    L"The capture device requires a manual firmware update, or the firmware version cannot be identified.");
                return false;

            case winrt::CaptureCard::ControllerFirmwareState::UpdateRequired:
                Log::Comment(L"The capture device requires a firmware update. Starting update...");

                try
                {
                    // Trigger a firmware update synchronously
                    firmwareInterface.UpdateFirmwareAsync().get();

                    firmwareVersion = firmwareInterface.FirmwareVersion();
                    Log::Comment((std::wstring(L"Successfully updated to new firmware version:\n") + firmwareVersion).c_str());
                }
                catch (...)
                {
                    Log::Error(
                        L"Failed to update capture device firmware. Please manually update the firmware and restart the test.");
                    return false;
                }
                break;

            case winrt::CaptureCard::ControllerFirmwareState::UpdateAvailable:
                Log::Warning(
                    L"A newer firmware version is available for the capture device but is not required. For best results, "
                    L"consider upgrading firmware.");
                break;

            case winrt::CaptureCard::ControllerFirmwareState::UpToDate:
                Log::Comment(L"The capture device firmware is up to date!");
                break;
            }
        }
    }

    return true;
}

bool CaptureFrameworkTestBase::Cleanup()
{
    return true;
}