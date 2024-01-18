#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::GenericCaptureCardPlugin::DataProcessing
{
enum class ComputeShaders
{
    // Format Conversion Shaders
    // -------------------------------------------
    // These shaders will convert incoming data from the specified format to scRGB.
    //
    // The output of these shaders should be linear scRGB 444, 16+ bpc floats with full range.
    //
    sRGB_8bpc_to_scRGB_16bpc,

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

    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame ProcessDataToFrame(
        winrt::Windows::Media::Capture::CapturedFrame frame);

    double ComputePSNR(winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame target,
				   winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame capture);

private:
    FrameProcessor();

    FrameProcessor(const FrameProcessor&) = delete;
    FrameProcessor& operator=(const FrameProcessor&) = delete;

    winrt::com_ptr<ID3D11ComputeShader> GetShader(ComputeShaders shaderToLoad);
    void ClearShaderState();

    template <typename PixelDataType>
    winrt::Windows::Storage::Streams::IBuffer GetBufferFromTexture(ID3D11Texture2D* texture);

    winrt::com_ptr<ID3D11Device> m_d3dDevice{nullptr};
    winrt::com_ptr<ID3D11DeviceContext> m_d3dDeviceContext{nullptr};
    std::map<ComputeShaders, winrt::com_ptr<ID3D11ComputeShader>> m_shaderCache;

    std::mutex m_d3dRenderingMutex;
};

struct Frame : winrt::implements<Frame, 
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrame, 
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameRenderable>
{
    Frame();

    // Functions from IRawFrame
    winrt::Windows::Storage::Streams::IBuffer Data();
    winrt::Windows::Devices::Display::Core::DisplayWireFormat DataFormat();
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> Properties();
    winrt::Windows::Graphics::SizeInt32 Resolution();

    // Functions from IRawFrameRenderable
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Graphics::Imaging::SoftwareBitmap> GetRenderableApproximationAsync();
    winrt::hstring GetPixelInfo(uint32_t x, uint32_t y);

    // Local-only members
    void SetBuffer(winrt::Windows::Storage::Streams::IBuffer data);
    void DataFormat(winrt::Windows::Devices::Display::Core::DisplayWireFormat const& description);
    void Resolution(winrt::Windows::Graphics::SizeInt32 const& resolution);
    void SetImageApproximation(winrt::Windows::Graphics::Imaging::SoftwareBitmap bitmap);

private:
    winrt::Windows::Storage::Streams::IBuffer m_data{nullptr};
    winrt::Windows::Devices::Display::Core::DisplayWireFormat m_format{nullptr};
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_properties;
    winrt::Windows::Graphics::SizeInt32 m_resolution{0, 0};

    winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{nullptr};
};

struct DisplayCapture : winrt::implements<DisplayCapture, 
    winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture, 
    winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet>
{
    // Constructor for creating a single-frame capture
    DisplayCapture(
        winrt::Windows::Media::Capture::CapturedFrame frame,
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> extendedProps);

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

} // namespace winrt::MicrosoftDisplayCaptureTools::GenericCaptureCardPlugin::DataProcessing
