
#include "pch.h"

#include "Controller.h"
#include "IteIt6803.h"
#include <initguid.h>
#include "Singleton.h"
#include "SampleDisplayCapture.h"
#include "Controller.g.cpp"

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::MicrosoftDisplayCaptureTools;
using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;

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
        // TODO: Discover capture boards connected via USB and place them in m_captureBoards
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
		std::vector<byte> frameVector;
		UsbBulkInPipe bulkIn = m_usbDevice.DefaultInterface().BulkInPipes().GetAt(0);
		DataReader reader = DataReader(bulkIn.InputStream());
		reader.LoadAsync(dataSize).GetResults();
		UINT64 data = reader.ReadUInt64();
		return frameVector;
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


	/*Buffer FrankenboardDevice::FpgaRead(unsigned short address, std::vector<byte> data)
	{
		DWORD retVal = MAXDWORD;
		const size_t readBlockSize = 0x50;
		UINT16 remaining = data.size();
		ULONG amountToRead = min(readBlockSize, remaining);
		Buffer readBuffer(amountToRead);
		readBuffer.Length(amountToRead);
		retVal = fpgaReadSetupPacket(readBuffer, address, remaining, &amountToRead);
		memcpy_s(data.data(), remaining, readBuffer.data(), readBuffer.Length());
		Buffer readValue(retVal);
	Exit:
		//return retVal;
		return readValue;

	}

	DWORD FrankenboardDevice::fpgaReadSetupPacket(Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead)
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

	}*/

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
