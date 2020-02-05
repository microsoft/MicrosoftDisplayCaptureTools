#pragma once
#include "GraphicsCaptureDeviceInput.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput>
    {
        GraphicsCaptureDeviceInput() = default;

        uint32_t GetId();
        SamplePlugin::CaptureCapabilities GetCaptureCaps();
        SamplePlugin::GraphicsCapturedFrame CaptureFrame();
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput, implementation::GraphicsCaptureDeviceInput>
    {
    };
}
