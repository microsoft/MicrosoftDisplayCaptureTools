#pragma once
#include "GraphicsCapturePlugin.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin>
    {
        GraphicsCapturePlugin() = default;

        SamplePlugin::GraphicsCaptureDeviceInput InitializeCaptureInput(uint32_t displayId);
        uint32_t GetCaptureInputCount();
        void GetCaptureInputDisplayIds(array_view<uint32_t> displayIds);
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin, implementation::GraphicsCapturePlugin>
    {
    };
}
