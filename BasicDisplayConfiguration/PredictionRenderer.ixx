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
    using namespace winrt::Microsoft::Graphics::Canvas;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::Windows::Storage::Streams;
}

namespace PredictionRenderer {

    struct BasicDisplayConfigurationFrame : winrt::implements<BasicDisplayConfigurationFrame, winrt::IRawFrame, winrt::IRawFrameRenderable>
    {
        BasicDisplayConfigurationFrame(winrt::ILogger const& logger);

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

    struct BasicDisplayConfigurationFrameSet : winrt::implements<BasicDisplayConfigurationFrameSet, winrt::IRawFrameSet>
    {
        BasicDisplayConfigurationFrameSet(winrt::ILogger const& logger);

        winrt::IVector<winrt::IRawFrame> Frames();
        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

    private:
        const winrt::ILogger m_logger{nullptr};
        
        std::vector<winrt::IRawFrame> m_frames;
        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
    };

    struct PredictionData : winrt::implements<PredictionData, winrt::IPredictionData>
    {
        PredictionData(winrt::ILogger const& logger);

        winrt::IRawFrameSet FrameSet();

        winrt::IMap<winrt::hstring, winrt::IInspectable> Properties();

    private:
        const winrt::ILogger m_logger{nullptr};

        winrt::IRawFrameSet m_frameSet{nullptr};
        winrt::IMap<winrt::hstring, winrt::IInspectable> m_properties;
    };

    struct Prediction : winrt::implements<Prediction, winrt::IPrediction>
    {
        Prediction(winrt::ILogger const& logger);

        winrt::IAsyncOperation<winrt::IPredictionData> FinalizePredictionAsync();

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
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface Surface;
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
    /// Contains all the information describing a frame from a display source. Frame providers will implement
    /// a class derived from this one that unlocks the frame on destruction.
    /// </summary>
    export struct FrameInformation
    {
        // Pre-blend stage information
        winrt::float4 BackgroundColor;
        std::vector<PlaneInformation> Planes;

        // Describes the current mode for transformation
        winrt::Size TargetModeSize;
        winrt::Size SourceModeSize;
        StretchMode SourceToTargetStretch;

        // Describes the post-blend color pipeline
        winrt::Windows::Devices::Display::Core::DisplayWireFormat WireFormat;
        std::vector<float> GammaLut;
        winrt::float4x4 ColorMatrixXyz;
    };

    /// <summary>
    /// The PredictionRenderer class intends to provide a complete software implementation of an idealized WDDM DDI-compatible
    /// display pipeline. It implements MPO plane blending for a variety of surface formats, color spaces, and alpha modes.
    /// It also implements the post-blend stages of the color pipeline.
    /// </summary>
    export class PredictionRenderer
    {
    public:
        enum class RenderMode
        {
            Target,
            SourceOnly
        };

        void Render(const winrt::CanvasDrawingSession& drawingSession);
        winrt::CanvasBitmap Render(const winrt::ICanvasResourceCreator& resourceCreator);

    private:
        RenderMode m_mode = RenderMode::Target;
        std::shared_ptr<FrameInformation> m_frameInfo;
    };

} // namespace PredictionRenderer

module :private;

namespace PredictionRenderer {

    PredictionData::PredictionData(winrt::ILogger const& logger) : m_logger(logger)
    {
        m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();
        m_frameSet = winrt::make<BasicDisplayConfigurationFrameSet>(m_logger);
    }

    IFrameData PredictionData::FrameData()
    {
        return m_frameData;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> PredictionData::Properties()
    {
        return m_properties;
    }

    Prediction::Prediction(winrt::ILogger const& logger) : m_logger(logger)
    {
    }

    winrt::IAsyncOperation<winrt::IPredictionData> Prediction::FinalizePredictionAsync()
    {
        // This operation is expected to be heavyweight, as tools are moving a lot of memory. So
        // we return the thread control and resume this function on the thread pool.
        co_await winrt::resume_background();

        // Create the prediction data object
        auto predictionData = winrt::make<PredictionData>(m_logger);

        // Set any desired format defaults - tools may override these
        {
            auto formatDesc = predictionData.FrameData().FormatDescription();

            formatDesc.Eotf = winrt::Windows::Devices::Display::Core::DisplayWireFormatEotf::Sdr;
            formatDesc.PixelEncoding = winrt::Windows::Devices::Display::Core::DisplayWireFormatPixelEncoding::Rgb444;

            predictionData.FrameData().FormatDescription(formatDesc);
        }

        // Invoke any tools registering as display setup (format, resolution, etc.)
        if (m_displaySetupCallback)
        {
            m_displaySetupCallback(*this, predictionData);
        }

        // Perform any changes to the format description required before buffer allocation
        {
            auto formatDesc = predictionData.FrameData().FormatDescription();
            formatDesc.Stride = formatDesc.BitsPerPixel * predictionData.FrameData().Resolution().Width;
            predictionData.FrameData().FormatDescription(formatDesc);
        }

        // From the data set in the predictionData, allocate buffers
        auto resolution = predictionData.FrameData().Resolution();
        auto desc = predictionData.FrameData().FormatDescription();

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

        predictionData.FrameSet().Data(pixelBuffer);

        // Invoke any tools registering as render setup
        if (m_renderSetupCallback)
        {
            m_renderSetupCallback(*this, predictionData);
        }

        // Invoke any tools registering as rendering
        if (m_renderLoopCallback)
        {
            m_renderLoopCallback(*this, predictionData);
        }

        co_return predictionData.as<winrt::IPredictionData>();
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

    void PredictionRenderer::Render(const winrt::CanvasDrawingSession& drawingSession)
    {
        auto colorBrush = winrt::Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, m_frameInfo->BackgroundColor);

        winrt::float3x2 sourceToTarget;

        if (m_mode == RenderMode::Target)
        {
            // We are rendering the full target, so include the source -> target transform
            switch (m_frameInfo->SourceToTargetStretch)
            {
            case StretchMode::Identity:
                sourceToTarget = winrt::float3x2::identity();
                break;
            case StretchMode::Center:
                sourceToTarget = winrt::make_float3x2_translation(
                    (float)((m_frameInfo->TargetModeSize.Width + m_frameInfo->SourceModeSize.Width) / 2),
                    (float)((m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height) / 2));
                break;
            case StretchMode::Fill:
                sourceToTarget = winrt::make_float3x2_scale(
                    (float)m_frameInfo->TargetModeSize.Width / m_frameInfo->SourceModeSize.Width,
                    (float)m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height);
                break;
            case StretchMode::FillToAspectRatio:
                float Ratio =
                    min((float)m_frameInfo->TargetModeSize.Width / m_frameInfo->SourceModeSize.Width,
                        (float)m_frameInfo->TargetModeSize.Height / m_frameInfo->SourceModeSize.Height);
                sourceToTarget = winrt::make_float3x2_scale(Ratio, Ratio) *
                                 winrt::make_float3x2_translation(
                                     (m_frameInfo->TargetModeSize.Width + m_frameInfo->SourceModeSize.Width * Ratio) / 2,
                                     (m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height * Ratio) / 2);
                break;
            }

            colorBrush.ColorHdr(m_frameInfo->BackgroundColor);
            drawingSession.FillRectangle(
                winrt::Rect(0, 0, (float)m_frameInfo->TargetModeSize.Width, (float)m_frameInfo->TargetModeSize.Height), colorBrush);
        }
        else
        {
            sourceToTarget = winrt::float3x2::identity();
        }

        CanvasAutoTransform FrameTransform(drawingSession, sourceToTarget);

        // Render the target bounds in the background color first
        for (auto& plane : m_frameInfo->Planes)
        {
            CanvasAutoTransform PlaneTransform(drawingSession, plane.TransformMatrix);

            // Render the plane
            if (plane.ColorMode == PlaneColorType::RGB)
            {
                // Get a D2D bitmap for the DXGI surface
                auto planeBitmap = winrt::CanvasBitmap::CreateFromDirect3D11Surface(drawingSession, plane.Surface, 96, plane.AlphaMode);

                drawingSession.DrawImage(
                    planeBitmap,
                    plane.DestinationRect.has_value() ? plane.DestinationRect.value() : winrt::Rect(winrt::Point(), m_frameInfo->SourceModeSize),
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
                    plane.DestinationRect.has_value() ? plane.DestinationRect.value() : winrt::Rect(winrt::Point(), m_frameInfo->SourceModeSize),
                    plane.SourceRect.has_value() ? plane.SourceRect.value() : winrt::Rect(winrt::Point(), planeSource.Size()),
                    1.0F,
                    plane.InterpolationMode);
            }
        }

        // We need to flush before we release the FrameInformation, which releases the surfaces
        drawingSession.Flush();
    }

    winrt::CanvasBitmap PredictionRenderer::Render(const winrt::ICanvasResourceCreator& resourceCreator)
    {
        auto renderTarget = winrt::CanvasRenderTarget(
            resourceCreator,
            (float)m_frameInfo->TargetModeSize.Width,
            (float)m_frameInfo->TargetModeSize.Height,
            96,
            winrt::DirectXPixelFormat::R16G16B16A16Float,
            winrt::CanvasAlphaMode::Ignore);

        {
            auto drawingSession = renderTarget.CreateDrawingSession();

            Render(drawingSession);

            drawingSession.Close();
        }

        return renderTarget;
    }

} // namespace PredictionRenderer