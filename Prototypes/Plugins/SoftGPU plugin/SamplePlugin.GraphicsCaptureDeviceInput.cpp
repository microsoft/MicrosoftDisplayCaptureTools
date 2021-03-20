#include "pch.h"
#include "SamplePlugin.GraphicsCaptureDeviceInput.h"
#include "SamplePlugin.GraphicsCaptureDeviceInput.g.cpp"

#include <random>

namespace winrt::SamplePlugin::implementation
{
    GraphicsCaptureDeviceInput::GraphicsCaptureDeviceInput()
    {
    }

    GraphicsCaptureDeviceInput::~GraphicsCaptureDeviceInput()
    {
        if (m_frameProvider) m_frameProvider.detach();
        if (m_target) m_target.detach();
        if (m_adapter) m_adapter.detach();
    }

    void GraphicsCaptureDeviceInput::InitializeFromId(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin)
    {
        m_uniqueId = captureInputUniqueId;
        m_plugin = &plugin;

        m_softGpu = SoftGpu::GetOrCreate();

        m_softGpu->GetSoftGpuAdapter(m_adapter.put());

        m_softGpu->GetSoftGpuPeekApi(m_peekApi.put());

        m_adapter->GetTarget(m_uniqueId, m_target.put());

        BSTR adapterUniqueName = nullptr;
        m_adapter->GetUniqueName(&adapterUniqueName);

        // TODO, this needs to be changed from source id = 0 to match the proper target id
        m_peekApi->CreateFrameProviderFromSource(adapterUniqueName, 0, m_frameProvider.put());
        m_frameProvider->CreateResourcesForCpuAccess();
    }
    SamplePlugin::DeviceInputState GraphicsCaptureDeviceInput::GetState()
    {
        return DeviceInputState::Active;
    }
    SamplePlugin::CaptureCapabilities GraphicsCaptureDeviceInput::GetCaptureCaps()
    {
        CaptureCapabilities captureCaps = { 0 };
        captureCaps.getImage = true;
        captureCaps.tolerance = 0.0f;

        return captureCaps;
    }
    SamplePlugin::GraphicsCapturedFrame GraphicsCaptureDeviceInput::CaptureFrame()
    {
        winrt::com_ptr<IFrameProviderFrame> frame;
        m_frameProvider->LockCurrentFrame(frame.put());

        BYTE* frameData = nullptr;
        UINT framePitch = 0;
        frame->GetPlaneSurfaceCpu(0, &frameData, &framePitch);

        SIZE targetSize = {};
        frame->GetTargetSize(&targetSize);

        FrameCharacteristics frameCharacteristics = { 0 };
        frameCharacteristics.format = PixelFormat::BGRA8;
        frameCharacteristics.height = targetSize.cy;
        frameCharacteristics.width = targetSize.cx;
        frameCharacteristics.stride = framePitch;
        frameCharacteristics.byteCount = frameCharacteristics.stride * frameCharacteristics.height;

        winrt::array_view<uint8_t const> byteArray(frameData, frameData + frameCharacteristics.byteCount);

        return SamplePlugin::GraphicsCapturedFrame(frameCharacteristics, byteArray);
    }
    uint32_t GraphicsCaptureDeviceInput::UniqueId()
    {
        return m_uniqueId;
    }
}
