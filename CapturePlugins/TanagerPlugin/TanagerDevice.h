#pragma once

namespace winrt::TanagerPlugin::implementation
{
    // The psnr limit we use to determine if a match is good enough to be considered a match.
    constexpr double PsnrLimit = 50.0;

    // This is a temporary limit while we're bringing up some of the hardware on board.
    constexpr uint32_t MaxDescriptorByteSize = 512;

    class TanagerDevice :
        public IMicrosoftCaptureBoard,
        public std::enable_shared_from_this<TanagerDevice>
    {
        inline static constinit const PCWSTR FpgaFirmwareFileName = L"TanagerFpgaFirmware.bin";
        inline static constinit const PCWSTR Fx3FirmwareFileName = L"TanagerFx3Firmware.bin";
        inline static constinit const std::tuple<uint8_t, uint8_t, uint8_t> MinimumFpgaVersion{(uint8_t)0, (uint8_t)0, (uint8_t)0};
        inline static constinit const std::tuple<uint8_t, uint8_t, uint8_t> MinimumFx3Version{(uint8_t)0, (uint8_t)0, (uint8_t)0};

    public:
        TanagerDevice(winrt::hstring deviceId);
        ~TanagerDevice();

        winrt::hstring GetDeviceId() override;
        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs() override;
        void FpgaWrite(unsigned short address, std::vector<byte> data) override;
        std::vector<byte> FpgaRead(unsigned short address, UINT16 data) override;
        std::vector<byte> ReadEndPointData(UINT32 dataSize) override;
        void SelectDisplayPortEDID(USHORT value);
        void I2cWriteData(uint16_t i2cAddress, uint8_t address, std::vector<byte> data);

        // Firmware updates
        void FlashFpgaFirmware(winrt::hstring filePath) override;
        void FlashFx3Firmware(winrt::hstring filePath) override;
        FirmwareVersionInfo GetFirmwareVersionInfo() override;
        winrt::Windows::Foundation::IAsyncAction UpdateFirmwareAsync() override;
        MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState GetFirmwareState() override;

        bool IsVideoLocked();
        std::mutex& SelectHdmi();
        std::mutex& SelectDisplayPort();

        std::unique_ptr<IteIt68051Plugin::VideoTiming> GetVideoTiming();
        std::unique_ptr<IteIt68051Plugin::AviInfoframe> GetAviInfoframe();
        std::unique_ptr<IteIt68051Plugin::ColorInformation> GetColorInformation(bool synchronizeInputAndOutputDepths = false);

    private:
        winrt::hstring m_deviceId;
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        std::shared_ptr<I2cDriver> m_pDriver;
        IteIt68051Plugin::IteIt68051 hdmiChip;
        Fx3FpgaInterface m_fpga;
        std::mutex m_changingPortsLocked;
    };

    struct TanagerDisplayInputHdmi : implements<TanagerDisplayInputHdmi, MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
    public:
        TanagerDisplayInputHdmi(std::weak_ptr<TanagerDevice> parent);

        ~TanagerDisplayInputHdmi();

        hstring Name();
        void SetDescriptor(MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor);
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger GetCaptureTrigger();
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities GetCapabilities();
        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame();
        void FinalizeDisplayState();
        void SetEdid(std::vector<byte> edid);

    private:
        std::weak_ptr<TanagerDevice> m_parent;
        std::shared_ptr<TanagerDevice> m_strongParent;
        std::atomic_bool m_hasDescriptorChanged = false;
    };

    struct TanagerDisplayInputDisplayPort : implements<TanagerDisplayInputDisplayPort, MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
    public:
        TanagerDisplayInputDisplayPort(std::weak_ptr<TanagerDevice> parent);

        ~TanagerDisplayInputDisplayPort();

        hstring Name();
        void SetDescriptor(MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor);
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger GetCaptureTrigger();
        MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities GetCapabilities();
        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame();
        void FinalizeDisplayState();
        void SetEdid(std::vector<byte> edid);

    private:
        std::weak_ptr<TanagerDevice> m_parent;
        std::shared_ptr<TanagerDevice> m_strongParent;
        std::atomic_bool m_hasDescriptorChanged = false;
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
}

