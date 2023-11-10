export module PredictionRenderer;

import "pch.h";
import RenderingUtils;

using namespace RenderingUtils;

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Numerics;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Microsoft::Graphics::Canvas;
    using namespace winrt::Microsoft::Graphics::Canvas::Effects;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::Windows::Storage::Streams;
}

namespace PredictionRenderer {

    /// <summary>
    /// Represents the type of plane described by a PlaneInformation.
    /// </summary>
    export enum class PlaneType { BasePlane, MultiOverlayPlane, LegacyOverlayPlane, CursorPlane };

    /// <summary>
    /// The color mode of a plane.
    /// </summary>
    export enum class PlaneColorType { RGB, YCbCr };

    /// <summary>
    /// Describes the information for a specific plane in a frame
    /// </summary>
    export struct PlaneInformation
    {
        PlaneType Type = PlaneType::BasePlane;
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface Surface = nullptr;
        winrt::CanvasAlphaMode AlphaMode = winrt::CanvasAlphaMode::Premultiplied;
        PlaneColorType ColorType = PlaneColorType::RGB;
        DXGI_COLOR_SPACE_TYPE ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
        winrt::float3x2 TransformMatrix = winrt::float3x2::identity();
        std::optional<winrt::Rect> SourceRect = {};
        std::optional<winrt::Rect> DestinationRect = {};
        float SdrWhiteLevel = 80.0F;
        winrt::CanvasImageInterpolation InterpolationMode = winrt::CanvasImageInterpolation::Linear;
    };

    /// <summary>
    /// Describes the source -> target stretch mode.
    /// </summary>
    export enum class StretchMode { Identity, Center, Fill, FillToAspectRatio };

    /// <summary>
    /// Rendering mode
    /// </summary>
    export enum class RenderMode { Target, SourceOnly };

    /// <summary>
    /// The set of information describing a frame and how it can be drawn.
    /// </summary>
    export struct FrameInformation
    {
        FrameInformation();

        // Pre-blend stage information
        winrt::float4 BackgroundColor = {0.f, 0.f, 0.f, 1.f};
        std::vector<PlaneInformation> Planes;

        // Describes the current mode for transformation
        winrt::Size TargetModeSize = {640, 480};
        winrt::Size SourceModeSize = {640, 480};
        StretchMode SourceToTargetStretch = StretchMode::Identity;
        RenderMode RenderMode = RenderMode::Target;

        // Describes the post-blend color pipeline
        winrt::DisplayWireFormat WireFormat = nullptr;
        std::vector<float> GammaLut;
        winrt::float4x4 ColorMatrixXyz = 
            {1.f, 0.f, 0.f, 0.f,
             0.f, 1.f, 0.f, 0.f,
             0.f, 0.f, 1.f, 0.f,
             0.f, 0.f, 0.f, 1.f};
    };

    export struct Frame : winrt::implements<Frame, winrt::IRawFrame, winrt::IRawFrameRenderable>
    {
        Frame(winrt::ILogger const& logger);

        // Functions from IRawFrame
        winrt::IBuffer Data();
        winrt::DisplayWireFormat DataFormat();
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();
        winrt::SizeInt32 Resolution();

        // Functions from IRawFrameRenderable
        winrt::IAsyncOperation<winrt::SoftwareBitmap> GetRenderableApproximationAsync();
        winrt::hstring GetPixelInfo(uint32_t x, uint32_t y);

        // Local-only members
        void SetBuffer(winrt::IBuffer data);
        void DataFormat(winrt::DisplayWireFormat const& description);
        void Resolution(winrt::SizeInt32 const& resolution);
        void SetImageApproximation(winrt::SoftwareBitmap bitmap);

    private:
        const winrt::ILogger m_logger{nullptr};

        winrt::IBuffer m_data{nullptr};
        winrt::DisplayWireFormat m_format{nullptr};
        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
        winrt::SizeInt32 m_resolution;

        winrt::SoftwareBitmap m_bitmap{nullptr};
    };

    export struct FrameSet : winrt::implements<FrameSet, winrt::IRawFrameSet>
    {
        FrameSet(winrt::ILogger const& logger);

        winrt::IVector<winrt::IRawFrame> Frames();
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

    private:
        const winrt::ILogger m_logger{nullptr};
        
        winrt::IVector<winrt::IRawFrame> m_frames;
        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
    };

    export struct PredictionData : winrt::implements<PredictionData, winrt::IPredictionData>
    {
        PredictionData(winrt::ILogger const& logger);

        // IPredictionData implementation

        // Properties defined for this implementation include:
        //
        // Name         Type     Default     Note
        // --------------------------------------------------------------------------------------------------------
        // UseWarp      bool     false       indicates if the drawing system will use WARP or on a physical device
        // FrameCount   int      1           how many frames to allocate
        //
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

        // Members specific to this implementation and only accessible to IConfigurationTools in this binary.
        std::vector<FrameInformation>& Frames();
        winrt::CanvasDevice Device();

    private:
        const winrt::ILogger m_logger{nullptr};

        winrt::CanvasDevice m_device{nullptr};

        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
        std::vector<PredictionRenderer::FrameInformation> m_frames;
    };

    /// <summary>
    /// The goal of this entire module is to provide a complete software implementation of an idealized WDDM DDI-compatible
    /// display pipeline. It implements MPO plane blending for a variety of surface formats, color spaces, and alpha modes.
    /// It also implements the post-blend stages of the color pipeline.
    /// 
    /// This implementation is spread over a few different structures in this module, but is largely situated in this
    /// Prediction struct, which passes around a 'PredictionData' struct through different tools to gather properties before
    /// composing those changes to a set of frames with FinalizePredictionAsync.
    /// </summary>
    export struct Prediction : winrt::implements<Prediction, winrt::IPrediction>
    {
        Prediction(winrt::ILogger const& logger);

        winrt::IAsyncOperation<winrt::IRawFrameSet> FinalizePredictionAsync();

        winrt::event_token DisplaySetupCallback(winrt::EventHandler<winrt::IPredictionData> const& handler);
        void DisplaySetupCallback(winrt::event_token const& token) noexcept;

        winrt::event_token RenderSetupCallback(winrt::EventHandler<winrt::IPredictionData> const& handler);
        void RenderSetupCallback(winrt::event_token const& token) noexcept;

        winrt::event_token RenderLoopCallback(winrt::EventHandler<winrt::IPredictionData> const& handler);
        void RenderLoopCallback(winrt::event_token const& token) noexcept;
        

    private:
        const winrt::ILogger m_logger{nullptr};

        winrt::event<winrt::EventHandler<winrt::IPredictionData>> m_displaySetupCallback;
        winrt::event<winrt::EventHandler<winrt::IPredictionData>> m_renderSetupCallback;
        winrt::event<winrt::EventHandler<winrt::IPredictionData>> m_renderLoopCallback;
    };
} // namespace PredictionRenderer

module: private;

namespace PredictionRenderer {

    FrameInformation::FrameInformation()
    {
        // Initialize each frame with a single plane (which by default is the base plane)
        Planes = std::vector<PlaneInformation>(1);

        // Initialize a default wire format for the frame
        WireFormat = winrt::DisplayWireFormat(
            winrt::DisplayWireFormatPixelEncoding::Rgb444,
            24, // bits per pixel
            winrt::DisplayWireFormatColorSpace::BT709,
            winrt::DisplayWireFormatEotf::Sdr,
            winrt::DisplayWireFormatHdrMetadata::None);
    }

    PredictionData::PredictionData(winrt::ILogger const& logger) : m_logger(logger)
    {
        m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();

        // Initialize the frames vector with a single frame - setup tools may add more
        m_frames = std::vector<PredictionRenderer::FrameInformation>(1);
    }

    std::vector<PredictionRenderer::FrameInformation>& PredictionData::Frames()
    {
        return m_frames;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> PredictionData::Properties()
    {
        return m_properties;
    }

    winrt::CanvasDevice PredictionData::Device()
    {
        if (!m_device)
        {
            bool useWarp = false;
            if (m_properties.HasKey(L"UseWarp"))
            {
                useWarp = winrt::unbox_value<bool>(m_properties.Lookup(L"UseWarp"));
            }

            m_logger.LogNote(std::format(L"Creating prediction renderer on {} device", useWarp ? L"WARP" : L"default"));

            m_device = winrt::CanvasDevice::GetSharedDevice(useWarp);
        }

        return m_device;
    }

    Prediction::Prediction(winrt::ILogger const& logger) : m_logger(logger)
    {
    }

    Frame::Frame(winrt::ILogger const& logger) : m_logger(logger)
    {
    }

    winrt::IBuffer Frame::Data()
    {
        return m_data;
    }

    winrt::DisplayWireFormat Frame::DataFormat()
    {
        return m_format;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> Frame::Properties()
    {
        return m_properties;
    }

    winrt::SizeInt32 Frame::Resolution()
    {
        return m_resolution;
    }

    winrt::IAsyncOperation<winrt::SoftwareBitmap> Frame::GetRenderableApproximationAsync()
    {
        // In this implementation, this approximation is created as a side effect of rendering the prediction. So here the result can
        // just be returned.
        co_return m_bitmap;
    }

    winrt::hstring Frame::GetPixelInfo(uint32_t x, uint32_t y)
    {
        // TODO: implement this - the intent is that because the above returns only an approximation, this should return a description
        // in string form of what the original pixel values are for a given address.
        throw winrt::hresult_not_implemented();
    }

    void Frame::SetBuffer(winrt::IBuffer buffer)
    {
        m_data = buffer;
    }

    void Frame::DataFormat(winrt::DisplayWireFormat const& description)
    {
        m_format = description;
    }

    void Frame::Resolution(winrt::SizeInt32 const& resolution)
    {
        m_resolution = resolution;
    }

    void Frame::SetImageApproximation(winrt::SoftwareBitmap bitmap)
    {
        m_bitmap = bitmap;
    }

    FrameSet::FrameSet(winrt::ILogger const& logger) : m_logger(logger)
    {
        m_frames = winrt::single_threaded_vector<winrt::IRawFrame>();
    }

    winrt::IVector<winrt::IRawFrame> FrameSet::Frames()
    {
        return m_frames;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> FrameSet::Properties()
    {
        return m_properties;
    }

    //
    // Compose the data collected for each frame into the final output data
    //
    // The way this works is that we define a surface on the GPU device that represents the frame data, this is a 3-channel,
    // 16-bits-per-channel, floating point surface that should represent a higher precision-level than required for any part
    // of the pipeline. This surface is nominally linear gamma, and the operations of a GPU display pipeline will happen
    // entirely in this space. Including degammma and regamma as appropriate. Configurable quantization steps will be inserted
    // between each stage of this pipeline so that various rounding and precision behaviors can be emulated.
    // 
    // Note: This represents an idealized WDDM-specified pipeline. It is known that physical hardware may have variations from
    //       the pipeline here, but the end result should exactly match.
    //
    // /-------------------------------- Operations on GPU (or Software-emulated GPU) ---------------------------\/-- CPU --\
    // |                                                                                                         ||         |
    // Plane 0 -- Plane Composition --- Degamma --- RGB->XYZ --- Color 3x4 --- XYZ->RGB --- ReGamma --- Encoding -- masking -- output
    //              /  /
    //             /  /
    // Plane 1 ----  /
    //              /
    //      .      /
    //      .     /
    //      .    /
    //          /
    // Plane X -
    //
    winrt::IAsyncOperation<winrt::IRawFrame> RenderPredictionFrame(FrameInformation& frameInformation, winrt::CanvasDevice device, winrt::ILogger const& logger)
    {
        co_await winrt::resume_background();

        auto planeCompositingTarget = winrt::CanvasRenderTarget(
            device,
            (float)frameInformation.TargetModeSize.Width,
            (float)frameInformation.TargetModeSize.Height,
            96,
            winrt::DirectXPixelFormat::R16G16B16A16Float,
            winrt::CanvasAlphaMode::Premultiplied);
        {
            auto drawingSession = planeCompositingTarget.CreateDrawingSession();
            drawingSession.EffectBufferPrecision(winrt::CanvasBufferPrecision::Precision16Float);

            auto backgroundBrush = winrt::Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, frameInformation.BackgroundColor);
            backgroundBrush.ColorHdr(frameInformation.BackgroundColor);

            winrt::float3x2 sourceToTarget;

            if (frameInformation.RenderMode == RenderMode::Target)
            {
                // We are rendering the full target, so include the source -> target transform
                switch (frameInformation.SourceToTargetStretch)
                {
                case StretchMode::Identity:
                    sourceToTarget = winrt::float3x2::identity();
                    break;
                case StretchMode::Center:
                    sourceToTarget = winrt::make_float3x2_translation(
                        (float)((frameInformation.TargetModeSize.Width + frameInformation.SourceModeSize.Width) / 2),
                        (float)((frameInformation.TargetModeSize.Height + frameInformation.SourceModeSize.Height) / 2));
                    break;
                case StretchMode::Fill:
                    sourceToTarget = winrt::make_float3x2_scale(
                        (float)frameInformation.TargetModeSize.Width / frameInformation.SourceModeSize.Width,
                        (float)frameInformation.TargetModeSize.Height + frameInformation.SourceModeSize.Height);
                    break;
                case StretchMode::FillToAspectRatio:
                    float Ratio =
                        min((float)frameInformation.TargetModeSize.Width / frameInformation.SourceModeSize.Width,
                            (float)frameInformation.TargetModeSize.Height / frameInformation.SourceModeSize.Height);

                    sourceToTarget = winrt::make_float3x2_scale(Ratio, Ratio) *
                                     winrt::make_float3x2_translation(
                                         (frameInformation.TargetModeSize.Width + frameInformation.SourceModeSize.Width * Ratio) / 2,
                                         (frameInformation.TargetModeSize.Height + frameInformation.SourceModeSize.Height * Ratio) / 2);
                    break;
                }

                drawingSession.FillRectangle(
                    winrt::Rect(0, 0, (float)frameInformation.TargetModeSize.Width, (float)frameInformation.TargetModeSize.Height), backgroundBrush);
            }
            else
            {
                sourceToTarget = winrt::float3x2::identity();
            }

            {
                CanvasAutoTransform FrameTransform(drawingSession, sourceToTarget);

                // Render the target bounds in the background color first
                for (auto& plane : frameInformation.Planes)
                {
                    CanvasAutoTransform PlaneTransform(drawingSession, plane.TransformMatrix);

                    // Render the plane
                    if (plane.ColorType == PlaneColorType::RGB)
                    {
                        // Get a Win2D bitmap for the plane's surface
                        auto planeBitmap =
                            winrt::CanvasBitmap::CreateFromDirect3D11Surface(drawingSession, plane.Surface, 96, plane.AlphaMode);
                        
                        drawingSession.DrawImage(
                            planeBitmap,
                            plane.DestinationRect.has_value() ? plane.DestinationRect.value()
                                                              : winrt::Rect(winrt::Point(), frameInformation.SourceModeSize),
                            plane.SourceRect.has_value() ? plane.SourceRect.value() : winrt::Rect(winrt::Point(), planeBitmap.Size()),
                            1.0F,
                            plane.InterpolationMode);
                    }
                    else
                    {
                        // Create an image source, which knows how to perform certain color-space conversions
                        auto planeSource = CreateVirtualBitmapFromDxgiSurface(
                            drawingSession, plane.Surface, plane.ColorSpace, D2D1_IMAGE_SOURCE_FROM_DXGI_OPTIONS_LOW_QUALITY_PRIMARY_CONVERSION);

                        drawingSession.DrawImage(
                            planeSource,
                            plane.DestinationRect.has_value() ? plane.DestinationRect.value()
                                                              : winrt::Rect(winrt::Point(), frameInformation.SourceModeSize),
                            plane.SourceRect.has_value() ? plane.SourceRect.value() : winrt::Rect(winrt::Point(), planeSource.Size()),
                            1.0F,
                            plane.InterpolationMode);
                    }
                }
            }

            drawingSession.Flush();
            drawingSession.Close();
        }

        // At this point, the prediction is a GPU-bound FP16 surface that has had the individual planes composited onto it.

        // Post plane blend rendering pipeline
        // 1. RGB->XYZ
        // 2. Color Matrix
        // 3. XYZ->RGB
        // 4. Regamma
        //
        // Note: Inserting 'quantization' steps in between each step so that we can accurately reflect hardware pipelines.
        auto postBlendTarget = winrt::CanvasRenderTarget(
            device,
            (float)frameInformation.TargetModeSize.Width,
            (float)frameInformation.TargetModeSize.Height,
            96,
            winrt::DirectXPixelFormat::R16G16B16A16Float,
            winrt::CanvasAlphaMode::Premultiplied);
        {
            auto drawingSession = postBlendTarget.CreateDrawingSession();
            drawingSession.EffectBufferPrecision(winrt::CanvasBufferPrecision::Precision16Float);

            auto backgroundBrush = winrt::Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, {0.F, 0.F, 0.F, 1.F});
            drawingSession.FillRectangle(
                winrt::Rect(0, 0, (float)frameInformation.TargetModeSize.Width, (float)frameInformation.TargetModeSize.Height), backgroundBrush);

            // 1. RBG->XYZ
            auto rgbxyzMatrixEffect = winrt::ColorMatrixEffect();
            winrt::Matrix5x4 rgbxyz{};

            rgbxyz.M11 = 0.4124564f; rgbxyz.M12 = 0.3575761f; rgbxyz.M13 = 0.1804375f; rgbxyz.M14 = 0.f; 
            rgbxyz.M21 = 0.2126729f; rgbxyz.M22 = 0.7151522f; rgbxyz.M23 = 0.0721750f; rgbxyz.M24 = 0.f;
            rgbxyz.M31 = 0.0193339f; rgbxyz.M32 = 0.1191920f; rgbxyz.M33 = 0.9503041f; rgbxyz.M34 = 0.f;
            rgbxyz.M41 = 0.f;        rgbxyz.M42 = 0.f;        rgbxyz.M43 = 0.f;        rgbxyz.M44 = 1.f;

            rgbxyzMatrixEffect.ColorMatrix(rgbxyz);
            rgbxyzMatrixEffect.Source(planeCompositingTarget);

            // 2. Color Matrix
            auto cscMatrix = winrt::ColorMatrixEffect();
            winrt::Matrix5x4 csc{};
            csc.M11 = frameInformation.ColorMatrixXyz.m11;
            csc.M12 = frameInformation.ColorMatrixXyz.m12;
            csc.M13 = frameInformation.ColorMatrixXyz.m13;
            csc.M21 = frameInformation.ColorMatrixXyz.m21;
            csc.M22 = frameInformation.ColorMatrixXyz.m22;
            csc.M23 = frameInformation.ColorMatrixXyz.m23;
            csc.M31 = frameInformation.ColorMatrixXyz.m31;
            csc.M32 = frameInformation.ColorMatrixXyz.m32;
            csc.M33 = frameInformation.ColorMatrixXyz.m33;
            csc.M44 = 1.f;
            cscMatrix.ColorMatrix(csc);
            cscMatrix.Source(rgbxyzMatrixEffect);

            // 3. XYZ->RGB
            auto xyzrgbMatrixEffect = winrt::ColorMatrixEffect();
            winrt::Matrix5x4 xyzrgb{};

            xyzrgb.M11 =  3.2404542f; xyzrgb.M12 = -1.5371385f; xyzrgb.M13 = -0.4985314f; xyzrgb.M14 = 0.f;
            xyzrgb.M21 = -0.9692660f; xyzrgb.M22 =  1.8760108f; xyzrgb.M23 =  0.0415560f; xyzrgb.M24 = 0.f;
            xyzrgb.M31 =  0.0556434f; xyzrgb.M32 = -0.2040259f; xyzrgb.M33 =  1.0572252f; xyzrgb.M34 = 0.f;
            xyzrgb.M41 =  0.f;        xyzrgb.M42 =  0.f;        xyzrgb.M43 =  0.f;        xyzrgb.M44 = 1.f;

            xyzrgbMatrixEffect.ColorMatrix(xyzrgb);
            xyzrgbMatrixEffect.Source(cscMatrix);

            // 4. Regamma
            auto regammaEffect = winrt::DiscreteTransferEffect();
            regammaEffect.Source(xyzrgbMatrixEffect);
            // TODO: define the actual gamma tables

            // Commit the color pipeline
            drawingSession.DrawImage(xyzrgbMatrixEffect);

            drawingSession.Flush();
            drawingSession.Close();
        }

        // Post color pipeline migration to wire format
        // 1. Format Conversion RGB->YUV (if applicable)
        // 2. Range Conversion Full->Studio (if applicable)
        //
        // Note: Inserting 'quantization' steps in between each step so that we can accurately reflect hardware pipelines.
        auto wireFormatTarget = winrt::CanvasRenderTarget(
            device,
            (float)frameInformation.TargetModeSize.Width,
            (float)frameInformation.TargetModeSize.Height,
            96,
            winrt::DirectXPixelFormat::R16G16B16A16Float,
            winrt::CanvasAlphaMode::Premultiplied);
        {
            auto drawingSession = postBlendTarget.CreateDrawingSession();
            drawingSession.EffectBufferPrecision(winrt::CanvasBufferPrecision::Precision16Float);

            // 1. Format Conversion RGB->YUV (if applicable)
            // TODO

            // 2. Range Conversion Full->Studio (if applicable)
            // TODO

            drawingSession.Flush();
            drawingSession.Close();
        }
        
        // Transit GPU surface to CPU-Accessible for returning.
        // 1. Create an output frame object
        // 2. Start copying over a GPU surface to the frame object for render previews
        // 3. Copy GPU surface to CPU-accessible memory
        // 4. Slice CPU-accessible pixel data to the destination type (starting from 16 bit-per-channel floats)
        // 5. Ensure that the copy started in 2 is finished
        {
            // 1. Create an output frame object
            auto frame = winrt::make_self<Frame>(logger);
            frame->Resolution(winrt::SizeInt32(postBlendTarget.SizeInPixels().Width, postBlendTarget.SizeInPixels().Height));
            frame->DataFormat(frameInformation.WireFormat);

            // 2. Create a renderable preview of the post-color pipeline image to be used as preview
            auto CreateSoftwareBitmapAsync = [&]() -> winrt::IAsyncOperation<winrt::SoftwareBitmap> {
                auto renderablePreview = winrt::CanvasRenderTarget(
                    device,
                    (float)frameInformation.TargetModeSize.Width,
                    (float)frameInformation.TargetModeSize.Height,
                    96,
                    winrt::DirectXPixelFormat::R8G8B8A8UIntNormalizedSrgb,
                    winrt::CanvasAlphaMode::Premultiplied);
                {
                    auto drawingSession = renderablePreview.CreateDrawingSession();
                    drawingSession.DrawImage(postBlendTarget);

                    drawingSession.Flush();
                    drawingSession.Close();
                }

                auto buffer =
                    winrt::Windows::Security::Cryptography::CryptographicBuffer::CreateFromByteArray(renderablePreview.GetPixelBytes());

                auto softwareBitmap = winrt::SoftwareBitmap::CreateCopyFromBuffer(
                    buffer,
                    winrt::BitmapPixelFormat::Rgba8,
                    frameInformation.TargetModeSize.Width,
                    frameInformation.TargetModeSize.Height);

                // Return the SoftwareBitmap
                co_return softwareBitmap;
            };

            auto softwareBitmapAsync = CreateSoftwareBitmapAsync();

            // 3. Copy GPU surface to CPU-accessible memory
            auto frameBytes = postBlendTarget.GetPixelBytes();

            // 4. Slice CPU-accessible pixel data to the destination type (starting from 16 bit-per-channel floats)
            auto frameBytesBufferWriter = winrt::DataWriter();
            frameBytesBufferWriter.WriteBytes(frameBytes);

            // TODO: allocate only what's actually needed for the output values, this will need to involve defining a 2-byte float -> integer function
            frame->SetBuffer(frameBytesBufferWriter.DetachBuffer());

            // 5. Ensure that the copy started in 2 is finished
            frame->SetImageApproximation(softwareBitmapAsync.get());

            auto rawFrame = frame.as<winrt::IRawFrame>();
            co_return rawFrame;
        }
    }

    /// <summary>
    /// Allow co_awaiting on a collection. Note that this can be a collection of IAsyncActions _or_ IAsyncOperations,
    /// the difference being whether you have to keep the collection around for results checking. Borrowed from
    /// winrt::when_all
    /// </summary>
    template <typename T>
    winrt::IAsyncAction when_all_container(T const& container)
    {
        for (auto&& action : container) co_await action;
    }

    winrt::IAsyncOperation<winrt::IRawFrameSet> Prediction::FinalizePredictionAsync()
    {
        // This operation is expected to be heavyweight, as tools are moving a lot of memory. So
        // we return the thread control and resume this function on the thread pool. The intent
        // is that during actual test operation, this can be queued up and happening behind the
        // scenes while the actual device output and capture is happening.
        co_await winrt::resume_background();

        // Create the prediction data object
        auto predictionData = winrt::make_self<PredictionData>(m_logger);

        // TODO: add a callback for test setup that sets things like whether to use warp, how many frames, etc.

        uint32_t frameCount = 1;
        if (predictionData->Properties().HasKey(L"FrameCount"))
        {
            frameCount = winrt::unbox_value<uint32_t>(predictionData->Properties().Lookup(L"FrameCount"));
        }

        predictionData->Frames().resize(frameCount);

        auto canvasDevice = predictionData->Device();

        // TODO: add a reference to the specific underlying device back to the prediction data stuff somehow

        // TODO: Should have options for per-frame and collective callbacks

        // TODO: rename pattern tool to something like BasePlanBasePlanePattern

        // Invoke any tools registering as display setup (format, resolution, etc.)
        if (m_displaySetupCallback)
        {
            m_displaySetupCallback(*this, predictionData.as<winrt::IPredictionData>());
        }

        // Invoke any tools registering as render setup
        if (m_renderSetupCallback)
        {
            m_renderSetupCallback(*this, predictionData.as<winrt::IPredictionData>());
        }

        // Invoke any tools registering as rendering
        if (m_renderLoopCallback)
        {
            m_renderLoopCallback(*this, predictionData.as<winrt::IPredictionData>());
        }

        // Create the output frame collection
        auto predictedFrames = winrt::make<FrameSet>(m_logger);
        predictedFrames.Frames().Clear();

        {
            // Render the predicted frames concurrently
            std::vector<winrt::IAsyncOperation<winrt::IRawFrame>> frameRenderTasks;
            for (auto& frame : predictionData->Frames())
            {
                frameRenderTasks.push_back(RenderPredictionFrame(frame, canvasDevice, m_logger));
            }

            // Wait for all of the render tasks to complete and collect the results
            for (auto& frameRenderTask : frameRenderTasks)
            {
                auto frame = co_await frameRenderTask;
                predictedFrames.Frames().Append(frame);
            }
        }

        co_return predictedFrames.as<winrt::IRawFrameSet>();
    }

    winrt::event_token Prediction::DisplaySetupCallback(winrt::EventHandler<winrt::IPredictionData> const& handler)
    {
        return m_displaySetupCallback.add(handler);
    }

    void Prediction::DisplaySetupCallback(winrt::event_token const& token) noexcept
    {
        m_displaySetupCallback.remove(token);
    }

    winrt::event_token Prediction::RenderSetupCallback(winrt::EventHandler<winrt::IPredictionData> const& handler)
    {
        return m_renderSetupCallback.add(handler);
    }

    void Prediction::RenderSetupCallback(winrt::event_token const& token) noexcept
    {
        m_renderSetupCallback.remove(token);
    }

    winrt::event_token Prediction::RenderLoopCallback(winrt::EventHandler<winrt::IPredictionData> const& handler)
    {
        return m_renderLoopCallback.add(handler);
    }

    void Prediction::RenderLoopCallback(winrt::event_token const& token) noexcept
    {
        m_renderLoopCallback.remove(token);
    }

} // namespace PredictionRenderer