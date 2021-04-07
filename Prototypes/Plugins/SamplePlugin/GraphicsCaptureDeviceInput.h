#pragma once
#include "GraphicsCaptureDeviceInput.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput>
    {
        GraphicsCaptureDeviceInput() = default;

        void InitializeFromId(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin);
        SamplePlugin::DeviceInputState GetState();
        SamplePlugin::CaptureCapabilities GetCaptureCaps();
        SamplePlugin::GraphicsCapturedFrame CaptureFrame();
        uint32_t UniqueId();

    private:
        uint32_t m_uniqueId = 0;
        SamplePlugin::GraphicsCapturePlugin* m_plugin = nullptr;
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput, implementation::GraphicsCaptureDeviceInput>
    {
    };
}
