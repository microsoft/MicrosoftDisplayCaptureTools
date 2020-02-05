#pragma once
#include "GraphicsCapturePlugin.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin>
    {
        GraphicsCapturePlugin() = default;

        SamplePlugin::GraphicsCaptureDeviceInput InitializeCaptureInput(int32_t displayId);
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin, implementation::GraphicsCapturePlugin>
    {
    };
}
