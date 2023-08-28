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
        winrt::CanvasAlphaMode AlphaMode = winrt::CanvasAlphaMode::Ignore;
        PlaneColorType ColorMode = PlaneColorType::RGB;
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
    /// Contains all the information describing a frame from a display source. Frame providers will implement
    /// a class derived from this one that unlocks the frame on destruction.
    /// </summary>
    export struct FrameInformation
    {
        FrameInformation();

        // Pre-blend stage information
        winrt::float4 BackgroundColor = {0.f, 0.f, 0.f, 1.f};
        std::vector<PlaneInformation> Planes;

        // Describes the current mode for transformation
        winrt::Size TargetModeSize = {480, 640};
        winrt::Size SourceModeSize = {480, 640};
        StretchMode SourceToTargetStretch = StretchMode::Identity;
        RenderMode RenderMode = RenderMode::Target;

        // Describes the post-blend color pipeline
        winrt::DisplayWireFormat WireFormat = nullptr;
        std::vector<float> GammaLut;
        winrt::float4x4 ColorMatrixXyz;
    };

    export struct Frame : winrt::implements<Frame, winrt::IRawFrame, winrt::IRawFrameRenderable>
    {
        Frame(winrt::ILogger const& logger);

        // Functions from IRawFrame
        winrt::IBuffer Data();
        winrt::FrameFormatDescription FormatDescription();
        void FormatDescription(winrt::FrameFormatDescription const& description);
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

        // Functions from IRawFrameRenderable
        winrt::IAsyncOperation<winrt::SoftwareBitmap> GetRenderableApproximationAsync();
        winrt::hstring GetPixelInfo(uint32_t x, uint32_t y);

    private:
        const winrt::ILogger m_logger{nullptr};

        winrt::IBuffer m_data{nullptr};
        winrt::FrameFormatDescription m_description;
        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
    };

    export struct FrameSet : winrt::implements<FrameSet, winrt::IRawFrameSet>
    {
        FrameSet(winrt::ILogger const& logger);

        winrt::IVector<winrt::IRawFrame> Frames();
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

    private:
        const winrt::ILogger m_logger{nullptr};
        
        std::vector<winrt::IRawFrame> m_frames;
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

    private:
        const winrt::ILogger m_logger{nullptr};

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

    Prediction::Prediction(winrt::ILogger const& logger) : m_logger(logger)
    {
    }

    // Compose the data collected for each frame into the final output frame
    winrt::IAsyncOperation<winrt::IRawFrame> RenderPredictionFrame(FrameInformation& frameInformation, winrt::CanvasDevice device, winrt::ILogger const& logger)
    {
        co_await winrt::resume_background();

        auto frame = winrt::make<Frame>(logger);

        auto renderTarget = winrt::CanvasRenderTarget(
            device,
            (float)frameInformation.TargetModeSize.Width,
            (float)frameInformation.TargetModeSize.Height,
            96,
            winrt::DirectXPixelFormat::R16G16B16A16Float,
            winrt::CanvasAlphaMode::Ignore);

        {
            auto drawingSession = renderTarget.CreateDrawingSession();

            auto colorBrush = winrt::Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, frameInformation.BackgroundColor);
            colorBrush.ColorHdr(frameInformation.BackgroundColor);

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
                    winrt::Rect(0, 0, (float)frameInformation.TargetModeSize.Width, (float)frameInformation.TargetModeSize.Height), colorBrush);
            }
            else
            {
                sourceToTarget = winrt::float3x2::identity();
            }

            CanvasAutoTransform FrameTransform(drawingSession, sourceToTarget);

            // Render the target bounds in the background color first
            for (auto& plane : frameInformation.Planes)
            {
                CanvasAutoTransform PlaneTransform(drawingSession, plane.TransformMatrix);

                // Render the plane
                if (plane.ColorMode == PlaneColorType::RGB)
                {
                    // Get a D2D bitmap for the DXGI surface
                    auto planeBitmap = winrt::CanvasBitmap::CreateFromDirect3D11Surface(drawingSession, plane.Surface, 96, plane.AlphaMode);

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

            // We need to flush before we release the FrameInformation, which releases the surfaces
            drawingSession.Flush();

            drawingSession.Close();
        }

        // renderTarget is now a CanvasBitmap - it needs to be connected to the Frame object we return


        co_return frame;
    }

    // Simple function to allow co_awaiting on a collection. Note that this can be a collection of IAsyncActions _or_ IAsyncOperations,
    // the difference being whether you have to keep the collection around for results checking.
    template <typename T>
    winrt::IAsyncAction when_all_container(T const& container)
    {
        for (auto&& action : container) co_await action;
    }

    winrt::IAsyncOperation<winrt::IRawFrameSet> Prediction::FinalizePredictionAsync()
    {
        // This operation is expected to be heavyweight, as tools are moving a lot of memory. So
        // we return the thread control and resume this function on the thread pool.
        co_await winrt::resume_background();

        // Create the prediction data object
        auto predictionData = winrt::make_self<PredictionData>(m_logger);
        auto iPredictionData = predictionData.as<winrt::IPredictionData>();

        // TODO: add a callback for test setup that sets things like whether to use warp, how many frames, etc.

        bool useWarp = false;
        if (predictionData->Properties().HasKey(L"UseWarp"))
        {
            useWarp = winrt::unbox_value<bool>(predictionData->Properties().Lookup(L"UseWarp"));
        }

        uint32_t frameCount = 1;
        if (predictionData->Properties().HasKey(L"FrameCount"))
        {
            frameCount = winrt::unbox_value<uint32_t>(predictionData->Properties().Lookup(L"FrameCount"));
        }

        predictionData->Frames().resize(frameCount);

        auto canvasDevice = winrt::CanvasDevice::GetSharedDevice(useWarp);

        // TODO: add a reference to the specific underlying device back to the prediction data stuff somehow

        // Invoke any tools registering as display setup (format, resolution, etc.)
        if (m_displaySetupCallback)
        {
            m_displaySetupCallback(*this, iPredictionData);
        }

        /*
        // Perform any changes to the format description required before buffer allocation
        {
            auto formatDesc = predictionData->FrameData().FormatDescription();
            formatDesc.Stride = formatDesc.BitsPerPixel * predictionData->FrameData().Resolution().Width;
            predictionData->FrameData().FormatDescription(formatDesc);
        }

        // From the data set in the predictionData, allocate buffers
        auto resolution = predictionData->FrameData().Resolution();
        auto desc = predictionData->FrameData().FormatDescription();

        if (resolution.Width == 0 || resolution.Height == 0)
        {
            std::wstringstream buf{};
            buf << L"Resolution of (" << resolution.Width << L", " << resolution.Height << L") not valid!";

            m_logger.LogError(buf.str());
        }

        if (desc.BitsPerPixel == 0 || desc.Stride == 0)
        {
            m_logger.LogError(L"BitsPerPixel and Stride must be defined sizes for us to reserve buffers!");
        }

        // reserve enough memory for the output frame.
        auto pixelBuffer = winrt::Buffer(resolution.Height * desc.Stride);
        pixelBuffer.

        predictionData->FrameSet().Data(pixelBuffer);

        */

        // Invoke any tools registering as render setup
        if (m_renderSetupCallback)
        {
            m_renderSetupCallback(*this, iPredictionData);
        }

        // Invoke any tools registering as rendering
        if (m_renderLoopCallback)
        {
            m_renderLoopCallback(*this, iPredictionData);
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
            co_await when_all_container(frameRenderTasks);

            for (auto& frame : frameRenderTasks)
            {
                predictedFrames.Frames().Append(frame.GetResults());
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