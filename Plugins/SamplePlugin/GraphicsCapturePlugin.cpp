#include "pch.h"
#include "GraphicsCapturePlugin.h"
#include "GraphicsCapturePlugin.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    SamplePlugin::GraphicsCaptureDeviceInput GraphicsCapturePlugin::InitializeCaptureInput(int32_t displayId)
    {
        (void)displayId;

        throw hresult_not_implemented();
    }
}
