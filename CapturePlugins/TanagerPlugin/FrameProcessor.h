#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::TanagerPlugin::DataProcessing
{

enum class ComputeShaders
{
    // Sampling shaders
    // -------------------------------------------
    // These represent the first stage of the pipeline, where the raw data is sampled from the Tanager's memory. 
    // This incoming data can be RGB or YCbCr, 444, 422, or 420, and various bit depths. 
    // 
    // The output of these shaders should be gamma-encoded RGB or YCbCr 444, 16+ bpc uints with either limited or full
    // range.
    //
    // Notes: in the future we expect to add parameters to these shaders to allow for different upsampling parameters
    //
    Sampler_444_8bpc,
    Sampler_444_10bpc,
    Sampler_422_8bpc,
    Sampler_422_10bpc,
    Sampler_420_8bpc,
    Sampler_420_10bpc,

    // De-Quantization shaders
    // -------------------------------------------
    // This shader will take the incoming data and will scale it based upon per-channel paramaters in a constant buffer.
    // This is to be used for making limited-range data full-range. This also scales the data from 0-2^(N-1) to 0-1.
    // 
    // The output of this shader should be gamma-encoded RGB or YCbCr 444, 16+ bpc floats with full range.
    //
    Dequantizer,

    // Color format conversion shaders
    // -------------------------------------------
    // These shaders will convert incoming data from YCbCr to RGB based on the standard conversion matrices, and will
    // be skipped for RGB data.
    // 
    // The output of these shaders should be gamma-encoded RGB 444, 16+ bpc floats with full range.
    //
    Ycbcr_ITUR_BT601,
    Ycbcr_ITUR_BT709,
    Ycbcr_ITUR_BT2020,

    // Transfer function shaders
    // -------------------------------------------
    // These shaders will convert incoming data from the specified transfer function to linear.
    // 
    // The output of these shaders should be linear RGB 444, 16+ bpc floats with full range.
    //
    Linearize_ITUR_BT601,
    Linearize_ITUR_BT709,
    Linearize_ITUR_BT2020,
    Linearize_opRGB,

    // Colorspace conversion shaders
    // -------------------------------------------
    // These shaders will convert incoming data from the specified colorspace to scRGB fp16. These should also produce a
    // secondary output of the same data in sRGB 8bpc uint. This secondary output is used for debugging purposes and it is
    // expected for it to only be an approximation of the data in the primary output.
    // 
    // The output of these shaders should be scRGB 16bpc floats _and_ sRGB 8bpc uints.
    //
    Color_ITUR_601,
    Color_ITUR_709,
    Color_ITUR_2020,
    Color_opRGB,
    Color_source_defined,

    // Frame Difference Computation
    // -------------------------------------------
    // This shader will take two frames and compute the squared difference between the individual pixels in a series of 
    // buckets that will then be summed together on the CPU side to compute a PSNR.
    //
    FrameSquaredDifferenceBucketSum,

    // Skip
    // -------------------------------------------
    // Not a real shader, this is just a hint used to indicate that the pipeline stage should be skipped.
    Skip,
};

class FrameProcessor
{
public:
    static FrameProcessor& GetInstance()
    {
        static FrameProcessor instance;
        return instance;
    }

    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame ProcessDataToFrame(IteIt68051Plugin::VideoTiming* timing,
                     IteIt68051Plugin::AviInfoframe* aviInfoframe,
                     IteIt68051Plugin::ColorInformation* colorInfo,
                     uint8_t* data,
                     uint32_t size);

    double ComputePSNR(winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame target,
              winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame capture);

private:
    FrameProcessor();

    FrameProcessor(const FrameProcessor&) = delete;
    FrameProcessor& operator=(const FrameProcessor&) = delete;

    winrt::com_ptr<ID3D11ComputeShader> GetShader(ComputeShaders shaderToLoad);
    void ClearShaderState();

    static ComputeShaders GetSamplerShader(IteIt68051Plugin::VideoTiming* timing,
        								   IteIt68051Plugin::AviInfoframe* aviInfoframe,
        								   IteIt68051Plugin::ColorInformation* colorInfo);

    static ComputeShaders GetDequantizerShader(IteIt68051Plugin::VideoTiming* timing,
        									   IteIt68051Plugin::AviInfoframe* aviInfoframe,
        									   IteIt68051Plugin::ColorInformation* colorInfo);

    static ComputeShaders GetColorFormatShader(IteIt68051Plugin::VideoTiming* timing,
        									   IteIt68051Plugin::AviInfoframe* aviInfoframe,
        									   IteIt68051Plugin::ColorInformation* colorInfo);

    static ComputeShaders GetTransferFunctionShader(IteIt68051Plugin::VideoTiming* timing,
        									        IteIt68051Plugin::AviInfoframe* aviInfoframe,
        									        IteIt68051Plugin::ColorInformation* colorInfo);

    static ComputeShaders GetColorspaceShader(IteIt68051Plugin::VideoTiming* timing,
        									  IteIt68051Plugin::AviInfoframe* aviInfoframe,
        									  IteIt68051Plugin::ColorInformation* colorInfo);

    winrt::com_ptr<ID3D11Device> m_d3dDevice{nullptr};
    winrt::com_ptr<ID3D11DeviceContext> m_d3dDeviceContext{nullptr};
    std::map<ComputeShaders, winrt::com_ptr<ID3D11ComputeShader>> m_shaderCache;

    std::mutex m_d3dRenderingMutex;
};

struct TanagerDisplayCapture : winrt::implements<TanagerDisplayCapture,
    winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture, 
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet>
{
    // Constructor for creating a single-frame capture
    TanagerDisplayCapture(
        std::vector<byte> pixels,
        IteIt68051Plugin::VideoTiming* timing,
        IteIt68051Plugin::AviInfoframe* aviInfoframe,
        IteIt68051Plugin::ColorInformation* colorInfo);

    // Methods from IDisplayCapture
    bool CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet prediction);
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet GetFrameData();
    winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> ExtendedProperties();

    // Methods from IRawFrameSet
    winrt::Windows::Foundation::Collections::IVector<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame> Frames();
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> Properties();

private:
    winrt::Windows::Foundation::Collections::IVector<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame> m_frames;

    // IRawFrameSet properties (generic metadata about the capture)
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_properties{nullptr};

    // DisplayCapture properties (Tanager-specific metadata)
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_extendedProps{nullptr};
};

struct Frame : winrt::implements<Frame,
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame, 
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameRenderable>
{
    Frame(
        winrt::Windows::Graphics::SizeInt32 const& resolution,
        winrt::Windows::Storage::Streams::IBuffer const& data,
        winrt::Windows::Graphics::Imaging::SoftwareBitmap const& bitmap);

    // Functions from IRawFrame
    winrt::Windows::Storage::Streams::IBuffer Data();
    winrt::Windows::Devices::Display::Core::DisplayWireFormat DataFormat();
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> Properties();
    winrt::Windows::Graphics::SizeInt32 Resolution();

    // Functions from IRawFrameRenderable
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Graphics::Imaging::SoftwareBitmap> GetRenderableApproximationAsync();
    winrt::hstring GetPixelInfo(uint32_t x, uint32_t y);

private:
    const winrt::Windows::Graphics::SizeInt32 m_resolution;
    const winrt::Windows::Storage::Streams::IBuffer m_data;
    const winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap;

    const winrt::Windows::Devices::Display::Core::DisplayWireFormat m_format{nullptr};

    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_properties;
};
} // namespace winrt::MicrosoftDisplayCaptureTools::TanagerPlugin::DataProcessing