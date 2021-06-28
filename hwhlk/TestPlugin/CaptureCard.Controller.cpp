#include "pch.h"
#include "CaptureCard.Controller.g.cpp"
#include "IteIt6803.h"
#include <initguid.h>
#include "MethodAccess.h"
//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
//
// {AF594BBC-240A-42D5-A805-7A1960EAEAD8}
DEFINE_GUID(GUID_DEVINTERFACE_Frankenboard,
    0xaf594bbc, 0x240a, 0x42d5, 0xa8, 0x5, 0x7a, 0x19, 0x60, 0xea, 0xea, 0xd8);

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;

namespace winrt::CaptureCard::implementation
{
    Controller::Controller()
    {
        // Enumerate and initialize all Frankenboard USB devices
        for (auto&& device : DeviceInformation::FindAllAsync(UsbDevice::GetDeviceSelector(GUID_DEVINTERFACE_Frankenboard)).get())
        {
            auto input = std::make_shared<FrankenboardDevice>(device.Id());
            m_frankenboardDevices.push_back(input);
            m_displayInputs.push_back(input->GetHdmiInput());
        }

		//Instantiating pointer to self & singleton
		std::weak_ptr<int>wp(Controller);
		MethodAccess::getMethodInstance(wp);
    }

    hstring Controller::Name()
    {
        return L"Frankenboard";
    }
    com_array<CaptureCard::IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<CaptureCard::IDisplayInput>(m_displayInputs);
        return ret;
    }
    ConfigurationTools::ConfigurationToolbox Controller::GetToolbox()
    {
        return ConfigurationTools::ConfigurationToolbox();
    }

	FrankenboardDevice::FrankenboardDevice(winrt::param::hstring deviceId)
        : m_usbDevice(nullptr)
    {
        m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();
        if (m_usbDevice == nullptr)
        {
            throw_hresult(E_FAIL);
        }

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

m_hdmiInput = winrt::make<CaptureCard::implementation::SampleDisplayInput>(this);
	}

	FrankenboardDevice::~FrankenboardDevice()
	{
		SetHpd(false);
	}

	CaptureCard::IDisplayInput FrankenboardDevice::GetHdmiInput()
	{
		return m_hdmiInput;
	}

	void FrankenboardDevice::TriggerHdmiCapture()
	{
		byte data = 0x0;
		std::vector<byte> dataBuff;
		dataBuff.push_back(data);
		FpgaWrite(0x20, dataBuff);	// Reset

		dataBuff[0] = 0x1;
		FpgaWrite(0x20, dataBuff);	// Clear reset, triggers capture logic
		FpgaRead(0x20, dataBuff);
	}

	void FrankenboardDevice::FpgaWrite(unsigned short address, std::vector<byte> data)
	{
		// Since I'm testing on USB2 break this up into chunks
		const size_t writeBlockSize = 0x50;
		for (int i = 0, remaining = data.size(); remaining > 0; i += writeBlockSize, remaining -= writeBlockSize)
		{
			size_t amountToWrite = min(writeBlockSize, remaining);
			UsbSetupPacket setupPacket;
			UsbControlRequestType requestType;
			requestType.AsByte(0x40); // 0_10_00000
			setupPacket.RequestType(requestType);
			setupPacket.Request(VR_UART_DATA_TRANSFER);
			setupPacket.Value(0);
			setupPacket.Index(address + i);	// EDID memory
			setupPacket.Length(amountToWrite);

			Buffer writeBuffer(amountToWrite);
			writeBuffer.Length(amountToWrite);
			memcpy_s(writeBuffer.data() + i, writeBuffer.Length(), data.data(), amountToWrite);
			m_usbDevice.SendControlOutTransferAsync(setupPacket, writeBuffer).get();
			Sleep(100);
		}
		
	}
	

	Buffer FrankenboardDevice::FpgaRead(unsigned short address, std::vector<byte> data)
	{
		DWORD retVal = MAXDWORD;
		const size_t readBlockSize = 0x50;
		UINT16 remaining = data.size();
		ULONG amountToRead = min(readBlockSize, remaining);
		Buffer readBuffer(amountToRead);
		readBuffer.Length(amountToRead);
		retVal = FpgaRead(readBuffer, address, remaining, &amountToRead);
		memcpy_s(data.data(), remaining, readBuffer.data(), readBuffer.Length());
		/*if (retVal == 0)
		{
			printf("Bytes read: %d \n", amountToRead);
			for (ULONG i = 0; i < amountToRead && i < sizeof(readBuffer); i++)
				printf("%02X", readBuffer[i]);
		}*/
		Buffer readValue(retVal);
	Exit:
		//return retVal;
		return readValue;

	}

	DWORD FrankenboardDevice::FpgaRead(Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead)
	{
		DWORD retVal = MAXDWORD;
		const size_t readBlockSize = 0x50;
		for (int i = 0; len > 0; i += readBlockSize, len -= readBlockSize)
		{
			UsbSetupPacket setupPacket;
			UsbControlRequestType requestType;
			requestType.AsByte(0xC0);
			setupPacket.RequestType(requestType);
			setupPacket.Request(VR_UART_DATA_TRANSFER);
			setupPacket.Value(0);
			setupPacket.Index(address);
			setupPacket.Length(len);
			if (m_usbDevice.SendControlInTransferAsync(setupPacket, readBuffer).get() == NULL)
			{
				retVal = GetLastError();
				printf("Error with Control transfer: %X\n", GetLastError());
				goto Exit;
			}
			retVal = 0;
		}
		
	Exit:
		return retVal;

	}
	//

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
