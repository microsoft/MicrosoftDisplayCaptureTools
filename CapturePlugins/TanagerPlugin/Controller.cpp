#include "pch.h"

#include <initguid.h>
#include "Controller.g.cpp"
#include "ControllerFactory.g.cpp"

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::MicrosoftDisplayCaptureTools;
using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;
using namespace winrt::MicrosoftDisplayCaptureTools::Framework;

//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
//
DEFINE_GUID(GUID_DEVINTERFACE_Tanager, 0x237e1ed8, 0x4c6b, 0x421e, 0xbe, 0x8f, 0x48, 0x52, 0x84, 0x42, 0x88, 0xed);


namespace winrt::TanagerPlugin::implementation
{
    CaptureCard::IController ControllerFactory::CreateController(ILogger const& logger)
    {
        return winrt::make<Controller>(logger);
    }

    Controller::Controller(ILogger const& logger) : m_logger(logger)
    {
        DiscoverCaptureBoards();

        // If no board is detected, this object will not be valid.
        if (m_captureBoards.size() == 0)
        {
            m_logger.LogWarning(L"No board detected!");
            throw winrt::hresult_access_denied();
        }

        // Add the inputs from all the discovered capture boards to the m_displayInputs list
        for (auto captureBoard : m_captureBoards)
        {
            auto boardInputs = captureBoard->EnumerateDisplayInputs();
            for (auto boardInput : boardInputs)
            {
                m_displayInputs.push_back(boardInput);
            }
        }
    }

    Controller::Controller()
    {
        // Throw - callers should explicitly instantiate through the factory
        throw winrt::hresult_illegal_method_call();
    }

    hstring Controller::Name()
    {
        return L"Tanager";
    }

    com_array<IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<IDisplayInput>(m_displayInputs);
        return ret;
    }

    void Controller::SetConfigData(Windows::Data::Json::IJsonValue data)
    {
		// There is no config data to be consumed right now.
    }

    ControllerFirmwareState Controller::FirmwareState()
    {
        auto firmwareStates = m_captureBoards | std::views::transform([](auto x) { return x->GetFirmwareState(); });

		// Return the most "severe" firmware state
        return *std::ranges::min_element(firmwareStates);
    }

    Windows::Foundation::IAsyncAction Controller::UpdateFirmwareAsync()
    {
        winrt::apartment_context origContext;

		// Resume onto a background thread
        co_await winrt::resume_background();

		// Update all firmware async
        for (auto captureBoard : m_captureBoards)
        {
            auto firmwareState = captureBoard->GetFirmwareState();
            if (firmwareState == ControllerFirmwareState::UpdateAvailable || firmwareState == ControllerFirmwareState::UpdateRequired)
            {
                co_await captureBoard->UpdateFirmwareAsync();
            }
		}

		// Switch back to calling context
		co_await origContext;
    }

    winrt::hstring Controller::FirmwareVersion()
    {
        std::wstringstream version;

        for (auto&& captureBoard : m_captureBoards)
        {
            auto deviceId = captureBoard->GetDeviceId();
            auto firmwareVersion = captureBoard->GetFirmwareVersionInfo();
            version << std::format(
                L"{}: FPGA {}.{}.{}, FX3 {}.{}.{}, HW {}\n",
                deviceId,
                firmwareVersion.fpgaFirmwareVersionMajor,
                firmwareVersion.fpgaFirmwareVersionMinor,
                firmwareVersion.fpgaFirmwareVersionPatch,
                firmwareVersion.fx3FirmwareVersionMajor,
                firmwareVersion.fx3FirmwareVersionMinor,
                firmwareVersion.fx3FirmwareVersionPatch,
                firmwareVersion.hardwareRevision);
        }

        return winrt::hstring{ version.str() };
    }

    void Controller::DiscoverCaptureBoards()
    {
		for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Tanager)).get())
		{
			auto input = std::make_shared<TanagerDevice>(device.Id(), m_logger);
			m_captureBoards.push_back(input);
		}
    }
}
