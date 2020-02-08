#include "pch.h"
#include "GraphicsCapturePlugin.h"
#include "GraphicsCapturePlugin.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    SamplePlugin::GraphicsCaptureDeviceInput GraphicsCapturePlugin::InitializeCaptureInput(uint32_t displayId)
    {
        throw hresult_not_implemented();
    }
    uint32_t GraphicsCapturePlugin::GetCaptureInputCount()
    {
        return 1;
    }
    void GraphicsCapturePlugin::GetCaptureInputDisplayIds(array_view<uint32_t> captureInputUniqueId)
    {
        captureInputUniqueId[0] = 65500;
    }
}
