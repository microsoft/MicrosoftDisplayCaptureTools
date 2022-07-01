#include "pch.h"

#include "IteIt6803.h"
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
// DEFINE_GUID(GUID_DEVINTERFACE_Frankenboard, 0xaf594bbc, 0x240a, 0x42d5, 0xa8, 0x5, 0x7a, 0x19, 0x60, 0xea, 0xea, 0xd8);
DEFINE_GUID(GUID_DEVINTERFACE_Tanager, 0x237e1ed8, 0x4c6b, 0x421e, 0xbe, 0x8f, 0x48, 0x52, 0x84, 0x42, 0x88, 0xed);


namespace winrt::TanagerPlugin::implementation
{
    CaptureCard::IController ControllerFactory::CreateController(ILogger const& logger)
    {
        return winrt::make<Controller>(logger);
    }

    Controller::Controller(ILogger const& logger) : m_logger(logger)
    {
        m_logger.LogNote(L"Instantiated the" + Name() + L"Plugin");
		
        DiscoverCaptureBoards();

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
        return L"Software Test Plugin";
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

        return winrt::hstring{version.str()};
    }

    void Controller::DiscoverCaptureBoards()
    {
		/*
		for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Frankenboard)).get())
		{
			auto input = std::make_shared<FrankenboardDevice>(device.Id());
			m_captureBoards.push_back(input);
		}
		*/

		for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Tanager)).get())
		{
			auto input = std::make_shared<TanagerDevice>(device.Id());
			m_captureBoards.push_back(input);
		}
    }

#define TCA6416A_INPUT_BASE_REGISTER 0
#define TCA6416A_OUTPUT_BASE_REGISTER 2
#define TCA6416A_POLARITY_BASE_REGISTER 4
#define TCA6416A_CONFIGURATION_BASE_REGISTER 6

	TiTca6416a::TiTca6416a(unsigned char address, std::shared_ptr<I2cDriver> pDriver) : address(address), pDriver(pDriver)
	{
	}

	TiTca6416a::~TiTca6416a()
	{
	}

	void TiTca6416a::SetOutputDirection(char bank, unsigned char value, unsigned char mask)
	{
		if (bank != TCA6416A_BANK_0 && bank != TCA6416A_BANK_1)
		{
			throw;
		}

		// If we're enabling outputs, make sure they're turned off first
		if ((value & mask) != mask)
		{
			SetOutput(bank, 0, (~value & mask));
		}

		if (mask == 0xff)
		{
			pDriver->writeRegisterByte(address, TCA6416A_CONFIGURATION_BASE_REGISTER + bank, value);
		}
		else
		{
			char current = pDriver->readRegisterByte(address, TCA6416A_CONFIGURATION_BASE_REGISTER + bank);
			char newValue = (current & ~mask) | value;
			pDriver->writeRegisterByte(address, TCA6416A_CONFIGURATION_BASE_REGISTER + bank, newValue);
		}
	}

	void TiTca6416a::SetOutput(char bank, unsigned char value, unsigned char mask)
	{
		if (bank != TCA6416A_BANK_0 && bank != TCA6416A_BANK_1)
		{
			throw;
		}

		if (mask == 0xff)
		{
			pDriver->writeRegisterByte(address, TCA6416A_OUTPUT_BASE_REGISTER + bank, value);
		}
		else
		{
			char current = pDriver->readRegisterByte(address, TCA6416A_OUTPUT_BASE_REGISTER + bank);
			char newValue = (current & ~mask) | value;
			pDriver->writeRegisterByte(address, TCA6416A_OUTPUT_BASE_REGISTER + bank, newValue);
		}
	}
}
