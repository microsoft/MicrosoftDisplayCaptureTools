#include "pch.h"
#include <filesystem>
#include <DirectXPackedVector.h>

namespace winrt 
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Enumeration;
    using namespace winrt::Windows::Devices::Usb;
    using namespace Windows::Graphics;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
}

using namespace IteIt68051Plugin;

namespace winrt::TanagerPlugin::implementation
{
const unsigned char it68051i2cAddress = 0x48;

TanagerDevice::TanagerDevice(winrt::hstring deviceId) :
    m_usbDevice(nullptr),
    m_deviceId(deviceId),
    hdmiChip(
        [&](uint8_t address, uint8_t value) // I2C write
        { m_pDriver->writeRegisterByte(it68051i2cAddress, address, value); },
        [&](uint8_t address) // I2C read
        { return m_pDriver->readRegisterByte(it68051i2cAddress, address); })
	{
		m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();

		if (!m_usbDevice)
		{
			throw_hresult(E_FAIL);
		}

		m_fpga.SetUsbDevice(m_usbDevice);
		m_pDriver = std::make_shared<I2cDriver>(m_usbDevice);
        m_fpga.SysReset(); // Blocks until FPGA is ready
		hdmiChip.Initialize();

        Logger().LogNote(L"Initializing Tanager Device: " + m_deviceId);
	}

	TanagerDevice::~TanagerDevice()
    {
	}

	std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> TanagerDevice::EnumerateDisplayInputs()
	{
		return std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
		{
			winrt::make<TanagerDisplayInputHdmi>(this->weak_from_this()),
			winrt::make<TanagerDisplayInputDisplayPort>(this->weak_from_this())
		};
	}

	void TanagerDevice::FpgaWrite(unsigned short address, std::vector<byte> data)
	{
		return m_fpga.Write(address, data);
	}

	std::vector<byte> TanagerDevice::FpgaRead(unsigned short address, UINT16 size)
	{
		return m_fpga.Read(address, size);
	}

	std::vector<byte> TanagerDevice::ReadEndPointData(UINT32 dataSize)
	{
		return m_fpga.ReadEndPointData(dataSize);
	}

	void TanagerDevice::FlashFpgaFirmware(winrt::hstring filePath)
	{
        m_fpga.FlashFpgaFirmware(filePath);
	}

	void TanagerDevice::FlashFx3Firmware(winrt::hstring filePath)
	{
        m_fpga.FlashFx3Firmware(filePath);
	}

	FirmwareVersionInfo TanagerDevice::GetFirmwareVersionInfo()
	{
		return m_fpga.GetFirmwareVersionInfo();
	}

    void TanagerDevice::SelectDisplayPortEDID(USHORT value)
    {
        m_fpga.SelectDisplayPortEDID(value);
    }

    bool TanagerDevice::IsVideoLocked()
    {
        return hdmiChip.IsVideoLocked();
    }

    void TanagerDevice::I2cWriteData(uint16_t i2cAddress, uint8_t address, std::vector<byte> data)
    {
        // Send this down in chunks
        const uint8_t writeBlockSize = 0x20;
        for (uint8_t i = 0, remaining = data.size(); remaining > 0; i += writeBlockSize, remaining -= min(writeBlockSize, remaining))
        {
            uint8_t amountToWrite = min(writeBlockSize, remaining);
            m_pDriver->writeRegister(i2cAddress, address + i, static_cast<uint32_t>(amountToWrite), data.data() + i);
            Sleep(100);
        }
    }

    std::mutex& TanagerDevice::SelectHdmi()
    {
        hdmiChip.SelectHdmi();
        return m_changingPortsLocked;
    }

    std::mutex& TanagerDevice::SelectDisplayPort()
    {
        hdmiChip.SelectDisplayPort();
        return m_changingPortsLocked;
    }

    std::unique_ptr<IteIt68051Plugin::VideoTiming> TanagerDevice::GetVideoTiming()
    {
        return hdmiChip.GetVideoTiming();
    }

    std::unique_ptr<IteIt68051Plugin::AviInfoframe> TanagerDevice::GetAviInfoframe()
    {
        return hdmiChip.GetAviInfoframe();
    }

    std::unique_ptr<IteIt68051Plugin::ColorInformation> TanagerDevice::GetColorInformation()
    {
        return hdmiChip.GetColorInformation();
    }

    winrt::hstring TanagerDevice::GetDeviceId()
    {
        return m_deviceId;
    }

    winrt::Windows::Foundation::IAsyncAction TanagerDevice::UpdateFirmwareAsync()
    {
        co_await winrt::resume_background();

        FlashFpgaFirmware(FpgaFirmwareFileName);
        FlashFx3Firmware(Fx3FirmwareFileName);
    }

    MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState TanagerDevice::GetFirmwareState()
    {
        auto versionInfo = GetFirmwareVersionInfo();
        if (versionInfo.GetFpgaFirmwareVersion() < MinimumFpgaVersion || versionInfo.GetFx3FirmwareVersion() < MinimumFx3Version)
        {
            return MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState::UpdateRequired;
        }
        else
        {
            return MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState::UpToDate;
        }
    }

} // namespace winrt::TanagerPlugin::implementation
