export module RenderingUtils;

import "pch.h";

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Microsoft::Graphics::Canvas;

namespace ABI 
{
    using namespace ABI::Microsoft::Graphics::Canvas;
}

namespace RenderingUtils {

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