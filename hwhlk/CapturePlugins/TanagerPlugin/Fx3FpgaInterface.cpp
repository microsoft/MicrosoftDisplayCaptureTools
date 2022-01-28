#include "pch.h"
#include "Fx3FpgaInterface.h"

using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;

namespace winrt::TanagerPlugin::implementation
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
		const size_t writeBlockSize = 0x40;
        for (size_t i = 0, remaining = data.size(); remaining > 0; i += writeBlockSize, remaining -= min(writeBlockSize, remaining))
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
			memcpy_s(writeBuffer.data(), writeBuffer.Length(), data.data() + i, amountToWrite);
			m_usbDevice.SendControlOutTransferAsync(setupPacket, writeBuffer).get();
			Sleep(100);
		}
	}

	std::vector<byte> Fx3FpgaInterface::Read(unsigned short address, UINT16 size)
	{
		std::vector<byte> dataVector;
		const size_t readBlockSize = 0x50;
		UINT16 remaining = size;
		while (remaining)
		{
			UINT16 amountToRead = min(readBlockSize, remaining);
			
			UsbSetupPacket setupPacket;
			UsbControlRequestType requestType;
			requestType.AsByte(0xC0);
			setupPacket.RequestType(requestType);
			setupPacket.Request(VR_UART_DATA_TRANSFER);
			setupPacket.Value(0);
			setupPacket.Index(address);
			setupPacket.Length(amountToRead);
			auto buffer = m_usbDevice.SendControlInTransferAsync(setupPacket).get();
			if (buffer == nullptr)
			{
				throw_last_error();
			}

			dataVector.insert(dataVector.end(), buffer.data(), buffer.data() + buffer.Length());
			remaining -= (UINT16)buffer.Length();
		}
		return dataVector;
	}

	std::vector<byte> Fx3FpgaInterface::ReadEndPointData(UINT32 dataSize)
	{
		std::vector<byte> frameBuffer;
        auto bulkInPipe = m_usbDevice.DefaultInterface().BulkInPipes().GetAt(0);

        DataReader reader = DataReader(bulkInPipe.InputStream());
        auto readSize = dataSize;
        auto bytesRemaining = dataSize;
        while (bytesRemaining)
        {
            std::vector<byte> buffer(dataSize);
            auto bytesToRead = min(bytesRemaining, readSize);
            reader.LoadAsync(bytesToRead).get();
            reader.ReadBytes(buffer);
            frameBuffer.insert(frameBuffer.end(), buffer.begin(), buffer.end());
            bytesRemaining -= bytesToRead;
        }
        return frameBuffer;
	}

	void Fx3FpgaInterface::FlashFpgaFirmware(Windows::Foundation::Uri uri)
	{
		throw winrt::hresult_not_implemented();
	}

	void Fx3FpgaInterface::FlashFx3Firmware(Windows::Foundation::Uri uri)
	{
		throw winrt::hresult_not_implemented();
	}

	FirmwareVersionInfo Fx3FpgaInterface::GetFirmwareVersionInfo()
	{
		UsbSetupPacket setupPacket;
		UsbControlRequestType requestType;
		requestType.AsByte(0xC0);
		setupPacket.RequestType(requestType);
		setupPacket.Request(VR_VERSION);
		setupPacket.Value(0);
		setupPacket.Index(0);
		setupPacket.Length(sizeof(FirmwareVersionInfo));
		auto buffer = m_usbDevice.SendControlInTransferAsync(setupPacket).get();
		if (buffer == nullptr)
		{
			throw_last_error();
		}
		if (buffer.Length() != sizeof(FirmwareVersionInfo))
		{
			throw winrt::hresult_error();
		}

		FirmwareVersionInfo versionInfo = { 0 };
		memcpy(&versionInfo, buffer.data(), sizeof(versionInfo));
		return versionInfo;
	}

	void Fx3FpgaInterface::SysReset()
    {
        UsbSetupPacket setupPacket;
        UsbControlRequestType requestType;
        requestType.AsByte(0x40);
        setupPacket.RequestType(requestType);
        setupPacket.Request(VR_SYS_RESET);
        setupPacket.Value(1);
        setupPacket.Index(0);
        setupPacket.Length(0);
        m_usbDevice.SendControlOutTransferAsync(setupPacket).get();

        while (!IsFpgaReady())
            Sleep(250);
    }

    bool Fx3FpgaInterface::IsFpgaReady()
    {
        UsbSetupPacket setupPacket;
        UsbControlRequestType requestType;
        requestType.AsByte(0xc0);
        setupPacket.RequestType(requestType);
        setupPacket.Request(VR_FPGA_READY);
        setupPacket.Value(0);
        setupPacket.Index(0);
        setupPacket.Length(1);
        Buffer inReadBuffer(1);
        inReadBuffer.Length(1);
        auto buffer = m_usbDevice.SendControlInTransferAsync(setupPacket, inReadBuffer).get();
        if (buffer == NULL || buffer.Length() != 1)
        {
            throw winrt::hresult_error();
        }

        return buffer.data()[0] != 0;
    }
    }
