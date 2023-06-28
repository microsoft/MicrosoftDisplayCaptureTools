module;

#include "winrt/Windows.Foundation.Numerics.h"
#include "winrt/Microsoft.Graphics.Canvas.h"
#include "winrt/Microsoft.Graphics.Canvas.Brushes.h"
#include "winrt/Windows.Devices.Display.Core.h"
#include "winrt/Windows.UI.h"

#include <dxgi.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Microsoft::Graphics::Canvas;

export module PredictionRenderer;

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
    winrt::com_ptr<IDXGISurface> Surface;
    CanvasAlphaMode AlphaMode = CanvasAlphaMode::Ignore;
    PlaneColorType ColorMode = PlaneColorType::RGB;
    DXGI_COLOR_SPACE_TYPE ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    float3x2 TransformMatrix = float3x2::identity();
    Rect SourceRect = {};
    bool HasSourceRect = false;
    float SdrWhiteLevel = 80.0F;
};

/// <summary>
/// Describes the source -> target stretch mode.
/// </summary>
export enum class StretchMode { Identity, Center, Fill, FillToAspectRatio };

/// <summary>
/// Contains all the information describing a frame from a display source. Frame providers will implement
/// a class derived from this one that unlocks the frame on destruction.
/// </summary>
export struct FrameInformation abstract
{
    virtual ~FrameInformation()
    {
    }

    LONGLONG FrameSnapTime;

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
/// Provides RAII for pushing/popping a matrix transform onto a D2D context.
/// </summary>
class CanvasAutoTransform
{
public:
    CanvasAutoTransform(const CanvasAutoTransform&) = delete;
    CanvasAutoTransform& operator=(const CanvasAutoTransform&) = delete;

    CanvasAutoTransform(const CanvasDrawingSession& drawingSession, const float3x2& originalTransform, const float3x2& newTransform) :
        m_drawingSession(drawingSession), m_originalTransform(originalTransform)
    {
        m_drawingSession.Transform(newTransform);
        m_isPushed = true;
    }

    CanvasAutoTransform(const CanvasDrawingSession& drawingSession, const float3x2& IntermediateTransform, bool CombineWithOriginal = true) :
        m_drawingSession(drawingSession)
    {
        m_originalTransform = m_drawingSession.Transform();
        if (CombineWithOriginal)
        {
            m_drawingSession.Transform(IntermediateTransform * m_originalTransform);
        }
        else
        {
            m_drawingSession.Transform(IntermediateTransform);
        }
        m_isPushed = true;
    }

    void Pop()
    {
        if (m_isPushed)
        {
            m_drawingSession.Transform(m_originalTransform);
            m_isPushed = false;
        }
    }

    float3x2 GetOriginal() const
    {
        return m_originalTransform;
    }

    ~CanvasAutoTransform()
    {
        Pop();
    }

private:
    const CanvasDrawingSession& m_drawingSession;
    float3x2 m_originalTransform;
    bool m_isPushed = false;
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

    void Render(const CanvasDrawingSession& drawingSession)
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
                ComPtr<ID2D1Bitmap1> PlaneBitmap;
                ThrowHR(Resources.D2DContext->CreateBitmapFromDxgiSurface(
                    plane.Surface.Get(),
                    D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, plane.AlphaMode)),
                    &PlaneBitmap));

                drawingSession.DrawImage(
                    PlaneBitmap.Get(), nullptr, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, plane.HasSourceRect ? &plane.SourceRect : nullptr);
            }
            else
            {
                // Create an image source, which knows how to perform certain color-space conversions
                IDXGISurface* Surfaces[] = {plane.Surface.Get()};
                ComPtr<ID2D1ImageSource> PlaneSource;
                ThrowHR(Resources.D2DContext->CreateImageSourceFromDxgi(
                    Surfaces, ARRAYSIZE(Surfaces), plane.ColorSpace, D2D1_IMAGE_SOURCE_FROM_DXGI_OPTIONS_LOW_QUALITY_PRIMARY_CONVERSION, &PlaneSource));

                drawingSession.DrawImage(PlaneSource.Get(), D2D1_INTERPOLATION_MODE_LINEAR);
            }
        }

        // We need to flush before we release the FrameInformation, which releases the surfaces
        drawingSession.Flush();
    }

private:
    RenderMode m_mode = RenderMode::Target;
    std::shared_ptr<FrameInformation> m_frameInfo;
};

} // namespace PredictionRenderer