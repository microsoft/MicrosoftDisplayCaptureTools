#include "pch.h"
#include "GraphicsCaptureDeviceInput.h"
#include "GraphicsCaptureDeviceInput.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    GraphicsCaptureDeviceInput::GraphicsCaptureDeviceInput(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& parentCapturePlugin)
    {
        throw hresult_not_implemented();
    }
    uint32_t GraphicsCaptureDeviceInput::GetId()
    {
        throw hresult_not_implemented();
    }
    SamplePlugin::CaptureCapabilities GraphicsCaptureDeviceInput::GetCaptureCaps()
    {
        throw hresult_not_implemented();
    }
    SamplePlugin::GraphicsCapturedFrame GraphicsCaptureDeviceInput::CaptureFrame()
    {
        throw hresult_not_implemented();
    }
}
