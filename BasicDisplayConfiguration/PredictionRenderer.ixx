export module PredictionRenderer;

import "pch.h";
import RenderingUtils;

using namespace RenderingUtils;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Microsoft::Graphics::Canvas;

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
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface Surface;
    CanvasAlphaMode AlphaMode = CanvasAlphaMode::Ignore;
    PlaneColorType ColorMode = PlaneColorType::RGB;
    DXGI_COLOR_SPACE_TYPE ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    float3x2 TransformMatrix = float3x2::identity();
    std::optional<Rect> SourceRect = {};
    std::optional<Rect> DestinationRect = {};
    float SdrWhiteLevel = 80.0F;
    CanvasImageInterpolation InterpolationMode = CanvasImageInterpolation::Linear;
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
    float4 BackgroundColor;
    std::vector<PlaneInformation> Planes;

    // Describes the current mode for transformation
    Size TargetModeSize;
    Size SourceModeSize;
    StretchMode SourceToTargetStretch;

    // Describes the post-blend color pipeline
    winrt::Windows::Devices::Display::Core::DisplayWireFormat WireFormat;
    std::vector<float> GammaLut;
    float4x4 ColorMatrixXyz;
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

    void Render(const CanvasDrawingSession& drawingSession);

private:
    RenderMode m_mode = RenderMode::Target;
    std::shared_ptr<FrameInformation> m_frameInfo;
};

} // namespace PredictionRenderer

module : private;

using namespace PredictionRenderer;

void ::PredictionRenderer::PredictionRenderer::Render(const CanvasDrawingSession& drawingSession)
{
    auto colorBrush = Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, m_frameInfo->BackgroundColor);

    float3x2 sourceToTarget;

    if (m_mode == RenderMode::Target)
    {
        // We are rendering the full target, so include the source -> target transform
        switch (m_frameInfo->SourceToTargetStretch)
        {
        case StretchMode::Identity:
            sourceToTarget = float3x2::identity();
            break;
        case StretchMode::Center:
            sourceToTarget = make_float3x2_translation(
                (float)((m_frameInfo->TargetModeSize.Width + m_frameInfo->SourceModeSize.Width) / 2),
                (float)((m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height) / 2));
            break;
        case StretchMode::Fill:
            sourceToTarget = make_float3x2_scale(
                (float)m_frameInfo->TargetModeSize.Width / m_frameInfo->SourceModeSize.Width,
                (float)m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height);
            break;
        case StretchMode::FillToAspectRatio:
            float Ratio =
                min((float)m_frameInfo->TargetModeSize.Width / m_frameInfo->SourceModeSize.Width,
                    (float)m_frameInfo->TargetModeSize.Height / m_frameInfo->SourceModeSize.Height);
            sourceToTarget = make_float3x2_scale(Ratio, Ratio) *
                                make_float3x2_translation(
                                    (m_frameInfo->TargetModeSize.Width + m_frameInfo->SourceModeSize.Width * Ratio) / 2,
                                    (m_frameInfo->TargetModeSize.Height + m_frameInfo->SourceModeSize.Height * Ratio) / 2);
            break;
        }

        colorBrush.ColorHdr(m_frameInfo->BackgroundColor);
        drawingSession.FillRectangle(
            Rect(0, 0, (float)m_frameInfo->TargetModeSize.Width, (float)m_frameInfo->TargetModeSize.Height), colorBrush);
    }
    else
    {
        sourceToTarget = float3x2::identity();
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
            auto planeBitmap = CanvasBitmap::CreateFromDirect3D11Surface(drawingSession, plane.Surface, 96, plane.AlphaMode);

            drawingSession.DrawImage(
                planeBitmap,
                plane.DestinationRect.has_value() ? plane.DestinationRect.value() : Rect(Point(), m_frameInfo->SourceModeSize),
                plane.SourceRect.has_value() ? plane.SourceRect.value() : Rect(Point(), planeBitmap.Size()),
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
                plane.DestinationRect.has_value() ? plane.DestinationRect.value() : Rect(Point(), m_frameInfo->SourceModeSize),
                plane.SourceRect.has_value() ? plane.SourceRect.value() : Rect(Point(), planeSource.Size()),
                1.0F,
                plane.InterpolationMode);
        }
    }

    // We need to flush before we release the FrameInformation, which releases the surfaces
    drawingSession.Flush();
}
