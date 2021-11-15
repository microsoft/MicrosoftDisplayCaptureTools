#pragma once

#include "Controller.h"
#include "IteIt68051.h"

namespace winrt::TestPlugin::implementation
{
    class TanagerDevice :
        public IMicrosoftCaptureBoard,
        std::enable_shared_from_this<TanagerDevice>
    {
    public:
        TanagerDevice(winrt::param::hstring deviceId);
        ~TanagerDevice();

        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs() override;
        void TriggerHdmiCapture() override;
        void FpgaWrite(unsigned short address, std::vector<byte> data) override;
        std::vector<byte> FpgaRead(unsigned short address, UINT16 data) override;
        std::vector<byte> ReadEndPointData(UINT32 dataSize) override;
        void FlashFpgaFirmware(Windows::Foundation::Uri uri) override;
        void FlashFx3Firmware(Windows::Foundation::Uri uri) override;
        FirmwareVersionInfo GetFirmwareVersionInfo() override;

    private:
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        std::shared_ptr<I2cDriver> m_pDriver;
        std::shared_ptr<IteIt68051> m_pHdmiChip;
        Fx3FpgaInterface m_fpga;
    };

    enum class TanagerDisplayInputPort
    {
        hdmi,
        displayPort,
    };

    struct TanagerDisplayInput : implements<TanagerDisplayInput, MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
    public:
        TanagerDisplayInput(std::weak_ptr<TanagerDevice> parent, TanagerDisplayInputPort port);
        hstring Name();
        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();
        MicrosoftDisplayCaptureTools::CaptureCard::CaptureCapabilities GetCapabilities();
        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame(MicrosoftDisplayCaptureTools::CaptureCard::CaptureTrigger trigger);
        void FinalizeDisplayState();

    private:
        std::weak_ptr<TanagerDevice> m_parent;
        TanagerDisplayInputPort m_port;
    };
}

