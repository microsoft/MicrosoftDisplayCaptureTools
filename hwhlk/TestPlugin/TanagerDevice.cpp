#include "pch.h"
#include "TanagerDevice.h"

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;

namespace winrt::TestPlugin::implementation
{
	TanagerDevice::TanagerDevice(winrt::param::hstring deviceId)
		: m_usbDevice(nullptr)
	{
		m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();
		if (m_usbDevice == nullptr)
		{
			throw_hresult(E_FAIL);
		}
		m_fpga.SetUsbDevice(m_usbDevice);

		m_pDriver = std::make_shared<I2cDriver>(m_usbDevice);

		const unsigned char it68051i2cAddress = 0x48;
		m_pHdmiChip = std::make_shared<IteIt68051>(it68051i2cAddress, m_pDriver);

		m_pHdmiChip->Initialize();
	}

	TanagerDevice::~TanagerDevice()
	{
	}

	std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> TanagerDevice::EnumerateDisplayInputs()
	{
		return std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
		{
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::hdmi),
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::displayPort),
		};
	}

	void TanagerDevice::TriggerHdmiCapture()
	{
		throw winrt::hresult_not_implemented();
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

	void TanagerDevice::FlashFpgaFirmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFpgaFirmware(uri);
	}

	void TanagerDevice::FlashFx3Firmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFx3Firmware(uri);
	}

	FirmwareVersionInfo TanagerDevice::GetFirmwareVersionInfo()
	{
		return m_fpga.GetFirmwareVersionInfo();
	}

	TanagerDisplayInput::TanagerDisplayInput(std::weak_ptr<TanagerDevice> parent, TanagerDisplayInputPort port)
		: m_parent(parent),
		  m_port(port)
	{
	}

	hstring TanagerDisplayInput::Name()
	{
		switch (m_port)
		{
		case TanagerDisplayInputPort::hdmi:
		{
			return L"HDMI";
		}
		case TanagerDisplayInputPort::displayPort:
		{
			return L"DisplayPort";
		}
		}
		
		winrt::throw_hresult(E_UNEXPECTED);
	}

	Windows::Devices::Display::Core::DisplayTarget TanagerDisplayInput::MapCaptureInputToDisplayPath()
	{
		throw winrt::hresult_not_implemented();
	}

	MicrosoftDisplayCaptureTools::CaptureCard::CaptureCapabilities TanagerDisplayInput::GetCapabilities()
	{
		throw winrt::hresult_not_implemented();
	}

	MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture TanagerDisplayInput::CaptureFrame(MicrosoftDisplayCaptureTools::CaptureCard::CaptureTrigger)
	{
		throw winrt::hresult_not_implemented();
	}

	void TanagerDisplayInput::FinalizeDisplayState()
	{
	}


}
