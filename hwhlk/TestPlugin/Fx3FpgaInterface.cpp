#include "pch.h"
#include "Fx3FpgaInterface.h"

using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;

namespace winrt::TestPlugin::implementation
{
	Fx3FpgaInterface::Fx3FpgaInterface()
		: m_usbDevice(nullptr)
	{
	}

	void Fx3FpgaInterface::SetUsbDevice(winrt::Windows::Devices::Usb::UsbDevice usbDevice)
	{
		m_usbDevice = usbDevice;
	}

	void Fx3FpgaInterface::Write(unsigned short address, std::vector<byte> data)
	{
		// Since I'm testing on USB2 break this up into chunks
		const size_t writeBlockSize = 0x50;
		for (size_t i = 0, remaining = data.size(); remaining > 0; i += writeBlockSize, remaining -= writeBlockSize)
		{
			size_t amountToWrite = min(writeBlockSize, remaining);
			UsbSetupPacket setupPacket;
			UsbControlRequestType requestType;
			requestType.AsByte(0x40); // 0_10_00000
			setupPacket.RequestType(requestType);
			setupPacket.Request(VR_UART_DATA_TRANSFER);
			setupPacket.Value(0);
			setupPacket.Index(address + (uint32_t)i);	// Cast to silence the compiler warnings.
			setupPacket.Length((uint32_t)amountToWrite); // If we try to write more than 2^32 bytes at once we have problems already.

			Buffer writeBuffer((uint32_t)amountToWrite);
			writeBuffer.Length((uint32_t)amountToWrite);
			memcpy_s(writeBuffer.data() + i, writeBuffer.Length(), data.data(), amountToWrite);
			m_usbDevice.SendControlOutTransferAsync(setupPacket, writeBuffer).get();
			Sleep(100);
		}
	}

	std::vector<byte> Fx3FpgaInterface::Read(unsigned short address, UINT16 size)
	{
		DWORD retVal = MAXDWORD;
		std::vector<byte> dataVector;
		const size_t readBlockSize = 0x50;
		UINT16 remaining = size;
		ULONG amountToRead = min(readBlockSize, remaining);
		dataVector.resize(amountToRead);
		Buffer readBuffer(amountToRead);
		readBuffer.Length(amountToRead);
		retVal = ReadSetupPacket(readBuffer, address, remaining, &amountToRead);
		memcpy_s(dataVector.data(), dataVector.size(), readBuffer.data(), amountToRead);
		return dataVector;
	}

	std::vector<byte> Fx3FpgaInterface::ReadEndPointData(UINT32 dataSize)
	{
		std::vector<byte> frameVector;
		UsbBulkInPipe bulkIn = m_usbDevice.DefaultInterface().BulkInPipes().GetAt(0);
		DataReader reader = DataReader(bulkIn.InputStream());
		reader.LoadAsync(dataSize).GetResults();
		reader.ReadBytes(frameVector);
		return frameVector;
	}

	DWORD Fx3FpgaInterface::ReadSetupPacket(winrt::Windows::Storage::Streams::Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead)
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
			retVal = readBuffer.Length();
		}

	Exit:
		return retVal;
	}
}
