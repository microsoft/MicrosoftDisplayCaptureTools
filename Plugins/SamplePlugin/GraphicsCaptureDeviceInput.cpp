#include "pch.h"
#include "GraphicsCaptureDeviceInput.h"
#include "GraphicsCaptureDeviceInput.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    GraphicsCaptureDeviceInput::GraphicsCaptureDeviceInput(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin) : 
        m_uniqueId(captureInputUniqueId),
        m_plugin(&plugin)
    {
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
        FrameCharacteristics frameCharacteristics = { 0 };
        frameCharacteristics.format = PixelFormat::RGB8;
        frameCharacteristics.height = 1080;
        frameCharacteristics.width = 1920;
        frameCharacteristics.bytes = frameCharacteristics.height * frameCharacteristics.width * sizeof(uint8_t) * 3;

        return SamplePlugin::GraphicsCapturedFrame(frameCharacteristics);
    }
    uint32_t GraphicsCaptureDeviceInput::UniqueId()
    {
        return m_uniqueId;
    }
}
