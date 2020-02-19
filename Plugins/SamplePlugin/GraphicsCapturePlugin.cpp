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
        return 5;
    }
    void GraphicsCapturePlugin::GetCaptureInputDisplayIds(array_view<uint32_t> captureInputUniqueId)
    {
        captureInputUniqueId[0] = 1;
        captureInputUniqueId[1] = 2;
        captureInputUniqueId[2] = 3;
        captureInputUniqueId[3] = 4;
        captureInputUniqueId[4] = 5;
    }
}
