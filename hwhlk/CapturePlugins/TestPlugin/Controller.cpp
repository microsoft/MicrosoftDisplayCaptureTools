
#include "pch.h"

#include "Controller.h"
#include "IteIt6803.h"
#include <initguid.h>
#include "SampleDisplayCapture.h"
#include "Controller.g.cpp"
#include "TanagerDevice.h"

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::MicrosoftDisplayCaptureTools;
using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;

//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
//
// {AF594BBC-240A-42D5-A805-7A1960EAEAD8}
DEFINE_GUID(GUID_DEVINTERFACE_Frankenboard,
	0xaf594bbc, 0x240a, 0x42d5, 0xa8, 0x5, 0x7a, 0x19, 0x60, 0xea, 0xea, 0xd8);
DEFINE_GUID(GUID_DEVINTERFACE_Tanager,
	0x237e1ed8, 0x4c6b, 0x421e, 0xbe, 0x8f, 0x48, 0x52, 0x84, 0x42, 0x88, 0xed);

namespace winrt::TestPlugin::implementation
{
    Controller::Controller()
    {
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

    hstring Controller::Name()
    {
        return L"Software Test Plugin";
    }

    com_array<IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<IDisplayInput>(m_displayInputs);
        return ret;
    }

    ConfigurationTools::IConfigurationToolbox Controller::GetToolbox()
    {
        return nullptr;
    }

    void Controller::DiscoverCaptureBoards()
    {
		for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Frankenboard)).get())
		{
			auto input = std::make_shared<FrankenboardDevice>(device.Id());
			m_captureBoards.push_back(input);
		}

		for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Tanager)).get())
		{
			auto input = std::make_shared<TanagerDevice>(device.Id());
			m_captureBoards.push_back(input);
		}
    }

    std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> FrankenboardDevice::EnumerateDisplayInputs()
    {
        //
        // Normally this is where a capture card would initialize and determine its own capabilities and 
        // inputs. For this sample, we are reporting a single input to this 'fake' capture card.
        //

        auto input = winrt::make<SampleDisplayInput>(this->weak_from_this());
        
        return std::vector<IDisplayInput>{ input };
    }
	
	FrankenboardDevice::FrankenboardDevice(winrt::param::hstring deviceId)
		: m_usbDevice(nullptr)
	{
		m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();
		if (m_usbDevice == nullptr)
		{
			throw_hresult(E_FAIL);
		}
		m_fpga.SetUsbDevice(m_usbDevice);

		pDriver = std::make_shared<I2cDriver>(m_usbDevice);
		TiTca6416a ioExp1(0x20, pDriver);
		TiTca6416a ioExp2(0x21, pDriver);

		ioExp1.SetOutputDirection(TCA6416A_BANK_0, 0, 0xff);
		ioExp1.SetOutput(TCA6416A_BANK_0, 0x4f, 0xff);
		ioExp1.SetOutputDirection(TCA6416A_BANK_1, 0, 0xff);
		ioExp1.SetOutput(TCA6416A_BANK_1, 0x8f, 0x8f);

		ioExp2.SetOutputDirection(TCA6416A_BANK_1, 0, 0xff);
		ioExp2.SetOutput(TCA6416A_BANK_1, 0x80, 0xff);  // Power LED on, muxes off

		ioExp2.SetOutputDirection(TCA6416A_BANK_0, 0, 0xff);
		ioExp2.SetOutput(TCA6416A_BANK_0, 0x20, 0xff);  // Route HDMI to HSMC

		Sleep(100);

		IteIt6803 hdmi(pDriver);    // Enable ITE HDMI chip

		std::vector<byte> edid = {
			0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x10, 0xac, 0x84, 0x41, 0x4c, 0x34, 0x45, 0x42, 0x1e, 0x1e, 0x01, 0x04, 0xa5, 0x3c, 0x22, 0x78, 0x3a, 0x48, 0x15, 0xa7, 0x56, 0x52, 0x9c, 0x27,
			0x0f, 0x50, 0x54, 0xa5, 0x4b, 0x00, 0x71, 0x4f, 0x81, 0x80, 0xa9, 0xc0, 0xd1, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
			0x45, 0x00, 0x56, 0x50, 0x21, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x36, 0x56, 0x54, 0x48, 0x5a, 0x31, 0x33, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x44,
			0x45, 0x4c, 0x4c, 0x20, 0x50, 0x32, 0x37, 0x31, 0x39, 0x48, 0x0a, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x38, 0x4c, 0x1e, 0x53, 0x11, 0x01, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0xec
		};
		SetEdid(edid);
		SetHpd(true);

		m_hdmiInput = winrt::make<TestPlugin::implementation::SampleDisplayInput>(this->weak_from_this());
	}

	FrankenboardDevice::~FrankenboardDevice()
	{
		SetHpd(false);
	}

	/*MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput FrankenboardDevice::GetHdmiInput()
	{
		return m_hdmiInput;
	}*/

	void FrankenboardDevice::TriggerHdmiCapture()
	{
		byte data = 0x0;
		std::vector<byte> dataBuff;
		dataBuff.push_back(data);
		FpgaWrite(0x20, dataBuff);	// Reset

		dataBuff[0] = 0x1;
		FpgaWrite(0x20, dataBuff);	// Clear reset, triggers capture logic
	}

	std::vector<byte> FrankenboardDevice::ReadEndPointData(UINT32 dataSize)
	{
		return m_fpga.ReadEndPointData(dataSize);
	}

	void FrankenboardDevice::FlashFpgaFirmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFpgaFirmware(uri);
	}

	void FrankenboardDevice::FlashFx3Firmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFx3Firmware(uri);
	}

	FirmwareVersionInfo FrankenboardDevice::GetFirmwareVersionInfo()
	{
		return m_fpga.GetFirmwareVersionInfo();
	}

	void FrankenboardDevice::FpgaWrite(unsigned short address, std::vector<byte> data)
	{
		m_fpga.Write(address, data);
	}

	std::vector<byte> FrankenboardDevice::FpgaRead (unsigned short address, UINT16 size)
	{
		return m_fpga.Read(address, size);
	}

	void FrankenboardDevice::SetEdid(std::vector<byte> Edid)
	{
		FpgaWrite(0x400, Edid);
	}

	void FrankenboardDevice::SetHpd(bool isPluggedIn)
	{
		byte data = 0x2;
		if (isPluggedIn)
		{
			data |= 0x1;
		}
		std::vector<byte> dataBuff;
		dataBuff.push_back(data);

		FpgaWrite(0x4, dataBuff);
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
