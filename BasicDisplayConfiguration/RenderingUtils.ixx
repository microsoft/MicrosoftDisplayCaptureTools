export module RenderingUtils;

import "pch.h";

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Devices::Display;
using namespace winrt::Windows::Devices::Display::Core;
using namespace winrt::Microsoft::Graphics::Canvas;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;

namespace ABI 
{
    using namespace ABI::Microsoft::Graphics::Canvas;
}

namespace RenderingUtils {

export enum class GammaType {G10, G22, G2084, GHLG, G24, G709};

export GammaType GetGammaTypeForEotf(const DisplayWireFormatEotf eotf)
{
    switch (eotf)
    {
    case DisplayWireFormatEotf::HdrSmpte2084:
        return GammaType::G2084;
    case DisplayWireFormatEotf::Sdr:
        return GammaType::G709;
    default:
        throw winrt::hresult_invalid_argument();
    }
}

export GammaType GetGammaTypeForColorSpace(const DXGI_COLOR_SPACE_TYPE colorSpace)
{
    switch (colorSpace)
    {
    case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
        return GammaType::G10;
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:
    case DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601:
    case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709:
    case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020:
    case DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020:
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020:
        return GammaType::G22;
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020:
    case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020:
    case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020:
        return GammaType::G2084;
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_GHLG_TOPLEFT_P2020:
    case DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020:
        return GammaType::GHLG;
    case DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709:
    case DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P2020:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P709:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_LEFT_P2020:
    case DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_TOPLEFT_P2020:
        return GammaType::G24;
    default:
        throw winrt::hresult_invalid_argument();
    }
}

export std::vector<float> CreateGammaTransferCurve(
    const GammaType sourceGamma, 
    const GammaType destGamma,
    unsigned long gammaStops)
{
    auto gammaArray = std::vector<float>(gammaStops);

    if (sourceGamma == GammaType::G10)
    {
        // Re-gamma operation
        switch (destGamma)
        {
        case GammaType::G10:
            for (unsigned int i = 0; i < gammaStops; i++)
            {
                gammaArray[i] = (float)i / (gammaStops - 1);
            }
            break;
        case GammaType::G22:
            for (unsigned int i = 0; i < gammaStops; i++)
            {
                float stop = (float)i / (gammaStops - 1);
                gammaArray[i] = stop <= 0.0031308f ? stop * 12.92f : 1.055f * std::powf(stop, 1.0f/2.4f) - 0.055f;
            }
            break;
        case GammaType::G709:
            for (unsigned int i = 0; i < gammaStops; i++)
            {
                float stop = (float)i / (gammaStops - 1);
                gammaArray[i] = stop < 0.018f ? stop * 4.5f : 1.099f * std::powf(stop, 0.45f) - 0.099f;
            }
            break;
        default:
            // Right now this explicitly only supports linear and 2.2
            throw winrt::hresult_invalid_argument();
        }
    }
    else if (destGamma == GammaType::G10)
    {
        // De-gamma operation
        switch (sourceGamma)
        {
        case GammaType::G10:
            for (unsigned long i = 0; i < gammaStops; i++)
            {
                gammaArray[i] = (float)i / (gammaStops - 1);
            }
            break;
        case GammaType::G22:
            for (unsigned long i = 0; i < gammaStops; i++)
            {
                float stop = (float)i / (gammaStops - 1);
                gammaArray[i] = stop <= 0.04045f ? stop / 12.92f : std::powf((stop + 0.055f) / 1.055f, 2.4f);
            }
            break;
        case GammaType::G709:
            for (unsigned long i = 0; i < gammaStops; i++)
            {
                float stop = (float)i / (gammaStops - 1);
                gammaArray[i] = stop < 0.081f ? stop / 4.5f : std::powf((stop + 0.099f) / 1.099f, 1.0f / 0.45f);
            }
            break;
        default:
            // Right now this explicitly only supports linear and 2.2
            throw winrt::hresult_invalid_argument();
        }
    }
    else
    {
        // Right now this explicitly only supports going to/from linear
        throw winrt::hresult_invalid_argument();
    }

    return gammaArray;
}

export winrt::com_ptr<IDXGISurface> GetNativeDxgiSurface(const winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface& surface)
{
    auto surfaceAccess = surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

    winrt::com_ptr<IDXGISurface> dxgiSurface;
    winrt::check_hresult(surfaceAccess->GetInterface(IID_PPV_ARGS(dxgiSurface.put())));
    return dxgiSurface;
}

export CanvasVirtualBitmap CreateVirtualBitmapFromDxgiSurface(
    const CanvasDrawingSession& drawingSession,
    const winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface& surface,
    DXGI_COLOR_SPACE_TYPE colorSpace,
    D2D1_IMAGE_SOURCE_FROM_DXGI_OPTIONS options)
{
    auto dxgiSurface = GetNativeDxgiSurface(surface);
    auto drawingSessionInterop = drawingSession.as<ABI::ICanvasResourceWrapperNative>();

    // We need to get the ID2D1DeviceContext3 from the CanvasDrawingSession
    winrt::com_ptr<ID2D1DeviceContext3> deviceContext;
    winrt::check_hresult(drawingSessionInterop->GetNativeResource(
        drawingSession.Device().as<ABI::ICanvasDevice>().get(), 0.0f, IID_PPV_ARGS(deviceContext.put())));

    // Create the image source from the DXGI surface
    winrt::com_ptr<ID2D1ImageSource> imageSource;
    IDXGISurface* surfaces[1] = {dxgiSurface.get()};
    winrt::check_hresult(deviceContext->CreateImageSourceFromDxgi(surfaces, _countof(surfaces), colorSpace, options, imageSource.put()));

    // ABI::ICanvasFactoryNative is on the activation factory for the CanvasDevice class
    auto factory = winrt::get_activation_factory<CanvasDevice, ABI::ICanvasFactoryNative>();

    // Create the CanvasVirtualBitmap using the factory to wrap the ID2D1ImageSource
    winrt::com_ptr<::IInspectable> resultObject;
    winrt::check_hresult(factory->GetOrCreate(
        drawingSession.Device().as<ABI::ICanvasDevice>().get(), imageSource.as<::IUnknown>().get(), 0.0f, resultObject.put()));
    return resultObject.as<CanvasVirtualBitmap>();
}

/// <summary>
/// Provides RAII for pushing/popping a matrix transform onto a D2D context.
/// </summary>
export class CanvasAutoTransform
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

} // namespace PredictionRenderer