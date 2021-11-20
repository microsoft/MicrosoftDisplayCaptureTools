#pragma once

namespace winrt::TanagerPlugin::implementation
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
        void SetDescriptor(MicrosoftDisplayCaptureTools::Display::IMonitorDescriptor descriptor);
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger GetCaptureTrigger();
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities GetCapabilities();
        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame();
        void FinalizeDisplayState();

    private:
        std::weak_ptr<TanagerDevice> m_parent;
        TanagerDisplayInputPort m_port;
    };

    struct CaptureTrigger : implements<CaptureTrigger, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger>
    {
    private:
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType m_type{
            winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType::Immediate};

        uint32_t m_time{0};

    public:
        CaptureTrigger() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType Type()
        {
            return m_type;
        }
        void Type(winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType type)
        {
            m_type = type;
        }

        uint32_t TimeToCapture() 
        {
            return m_time;
        }
        void TimeToCapture(uint32_t time)
        {
            m_time = time;
        }
    };

    struct TanagerCaptureCapabilities : implements<TanagerCaptureCapabilities, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities>
    {
        TanagerCaptureCapabilities() = default;

        bool CanReturnRawFramesToHost();
        bool CanReturnFramesToHost();
        bool CanCaptureFrameSeries();
        bool CanHotPlug();
        bool CanConfigureEDID();
        bool CanConfigureDisplayID();
        uint32_t GetMaxDescriptorSize();
    };
}

