#include "pch.h"
#include "GraphicsCaptureDeviceInput.h"
#include "GraphicsCaptureDeviceInput.g.cpp"

#include <random>

namespace winrt::SamplePlugin::implementation
{
    static uint32_t GetStrides(PixelFormat format, uint32_t width)
    {
        switch (format)
        {
        case PixelFormat::RGB16:
            return width * sizeof(uint16_t) * 3;
        case PixelFormat::RGB8:
            return width * sizeof(uint8_t) * 3;
        default:
            return 0;
        }
    }

    void GraphicsCaptureDeviceInput::InitializeFromId(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin)
    {
        throw hresult_not_implemented();
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
        FrameCharacteristics frameCharacteristics = { 0 };
        if (m_uniqueId > 4) frameCharacteristics.format = PixelFormat::RGB16;   //prototyping to make sure I can get both pixel formats across
        else frameCharacteristics.format = PixelFormat::RGB8;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(100, 1000);

        frameCharacteristics.height = dist(gen);
        frameCharacteristics.width = dist(gen);
        frameCharacteristics.stride = GetStrides(frameCharacteristics.format, frameCharacteristics.width);
        frameCharacteristics.byteCount = frameCharacteristics.stride * frameCharacteristics.height;

        winrt::array_view<uint8_t const> arrView = { 0 };
        return SamplePlugin::GraphicsCapturedFrame(frameCharacteristics, arrView);
    }
    uint32_t GraphicsCaptureDeviceInput::UniqueId()
    {
        return m_uniqueId;
    }
}
