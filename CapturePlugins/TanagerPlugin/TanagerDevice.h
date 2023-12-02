#pragma once

namespace winrt::TanagerPlugin::implementation
{
    // This is a temporary limit while we're bringing up some of the hardware on board.
    constexpr uint32_t MaxDescriptorByteSize = 512;

    // The Tanager device has two display inputs: HDMI and DisplayPort. However these inputs are both
    // put through a single ITE chip, which exposes the data as HDMI to the host from either input. 
    // This means that we only need to handle HDMI data formats, which are enumerated below. These 
    // formats each correspond to different data packet structures that we will load compute shaders
    // to translate into a common RGB or YUV floating point format.
    enum class HdmiRawCaptureFormats
    {
        RGB_444_8bpc_Full,
        RGB_444_8bpc_Limited,
        RGB_444_10bpc_Full,
        RGB_444_10bpc_Limited,
        RGB_444_12bpc_Full,
        RGB_444_12bpc_Limited,
        RGB_444_16bpc_Full,
        RGB_444_16bpc_Limited,
        YCbCr_444_8bpc_Limited,
        YCbCr_444_10bpc_Limited,
        YCbCr_444_12bpc_Limited,
        YCbCr_444_16bpc_Limited,
        YCbCr_422_8bpc_Limited,
        YCbCr_422_10bpc_Limited,
        YCbCr_422_12bpc_Limited,
        YCbCr_422_16bpc_Limited,
        YCbCr_420_8bpc_Limited,
        YCbCr_420_10bpc_Limited,
        YCbCr_420_12bpc_Limited,
        YCbCr_420_16bpc_Limited,
    };

    static HdmiRawCaptureFormats GetFormatFromInfoFrame(winrt::Windows::Storage::Streams::IBuffer infoFrame, uint32_t bitsPerPixel);

    class TanagerD3D
    {
    public:
        TanagerD3D(winrt::com_ptr<ID3D11Device> d3dDevice, winrt::com_ptr<ID3D11DeviceContext> d3dDeviceContext) :
            m_d3dDevice(d3dDevice),
            m_d3dDeviceContext(d3dDeviceContext)
        {
        }

        std::mutex& RenderingMutex()
        {
            return m_d3dRenderingMutex;
		}

        winrt::com_ptr<ID3D11ComputeShader> GetRawDataTranslateShader(HdmiRawCaptureFormats const& type);

        winrt::com_ptr<ID3D11ComputeShader> GetDiffSumShader();

        winrt::com_ptr<ID3D11Device> GetDevice()
        {
			return m_d3dDevice;
		}

		winrt::com_ptr<ID3D11DeviceContext> GetDeviceContext()
		{
            return m_d3dDeviceContext;
        }

    private:
        std::map<HdmiRawCaptureFormats, winrt::com_ptr<ID3D11ComputeShader>> m_computeShaderCache;

        winrt::com_ptr<ID3D11ComputeShader> m_diffSumShader;
        winrt::com_ptr<ID3D11Device> m_d3dDevice{nullptr};
        winrt::com_ptr<ID3D11DeviceContext> m_d3dDeviceContext{nullptr};
        std::mutex m_d3dRenderingMutex;
    };

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
        IteIt68051Plugin::VideoTiming GetVideoTiming();

        IteIt68051Plugin::aviInfoframe GetAviInfoframe();
        IteIt68051Plugin::ColorInformation GetColorInformation();

        std::shared_ptr<TanagerD3D> GetD3D();

    private:
        winrt::hstring m_deviceId;
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        std::shared_ptr<I2cDriver> m_pDriver;
        IteIt68051Plugin::IteIt68051 hdmiChip;
        Fx3FpgaInterface m_fpga;
        std::mutex m_changingPortsLocked;

        std::shared_ptr<TanagerD3D> m_d3d{nullptr};
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

    struct TanagerDisplayCapture
        : implements<TanagerDisplayCapture, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet>
    {
        // Constructor for creating a single-frame capture
        TanagerDisplayCapture(
            std::shared_ptr<TanagerD3D> D3DInstance,
            std::vector<byte> pixels,
            winrt::Windows::Graphics::SizeInt32 resolution,
            winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> extendedProps);

        // Methods from IDisplayCapture
        bool CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet prediction);
        winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet GetFrameData();
        winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> ExtendedProperties();

        // Methods from IRawFrameSet
        winrt::Windows::Foundation::Collections::IVector<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame> Frames();
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> Properties();

    private:
        std::shared_ptr<TanagerD3D> m_D3DInstance{nullptr};
        winrt::Windows::Foundation::Collections::IVector<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame> m_frames;

        // IRawFrameSet properties (generic metadata about the capture)
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_properties{nullptr};

        // DisplayCapture properties (Tanager-specific metadata)
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_extendedProps{nullptr};
    };

    struct Frame
        : winrt::implements<Frame, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameRenderable>
    {
        Frame(
            std::shared_ptr<TanagerD3D> D3DInstance,
            HdmiRawCaptureFormats const& type,
            winrt::Windows::Graphics::SizeInt32 const& resolution,
            winrt::Windows::Devices::Display::Core::DisplayWireFormat const& format,
            winrt::Windows::Storage::Streams::IBuffer const& data);

        // TODO: change the buffer name to pixeldata or pixelbuffer
        //       add some utility to save out the data as json or something

        // Functions from IRawFrame
        winrt::Windows::Storage::Streams::IBuffer Data();
        winrt::Windows::Devices::Display::Core::DisplayWireFormat DataFormat();
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> Properties();
        winrt::Windows::Graphics::SizeInt32 Resolution();

        // Functions from IRawFrameRenderable
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Graphics::Imaging::SoftwareBitmap> GetRenderableApproximationAsync();
        winrt::hstring GetPixelInfo(uint32_t x, uint32_t y);

        // Local-only members
        void DataFormat(winrt::Windows::Devices::Display::Core::DisplayWireFormat const& description);
        void Resolution(winrt::Windows::Graphics::SizeInt32 const& resolution);

    private:
        std::shared_ptr<TanagerD3D> m_D3DInstance{nullptr};

        const HdmiRawCaptureFormats m_type;
        const winrt::Windows::Storage::Streams::IBuffer m_data{nullptr};
        const winrt::Windows::Devices::Display::Core::DisplayWireFormat m_format{nullptr};
        const winrt::Windows::Graphics::SizeInt32 m_resolution{0, 0};

        winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{nullptr};
        winrt::com_ptr<ID3D11Texture2D> m_comparisonTexture{nullptr};

        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_properties;
    };
}

