#pragma once
#include "GraphicsCaptureDeviceInput.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput>
    {
        GraphicsCaptureDeviceInput() = default;

        GraphicsCaptureDeviceInput(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin);
        SamplePlugin::CaptureCapabilities GetCaptureCaps();
        SamplePlugin::GraphicsCapturedFrame CaptureFrame();
        uint32_t UniqueId();

    private:
        uint32_t m_uniqueId;
        const SamplePlugin::GraphicsCapturePlugin* m_plugin;
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput, implementation::GraphicsCaptureDeviceInput>
    {
    };
}
