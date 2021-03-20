#pragma once
#include "SamplePlugin.GraphicsCapturePlugin.g.h"
#include "SoftGPULoader.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin>
    {
        GraphicsCapturePlugin();

        void InitializeCaptureInput(uint32_t displayId);
        uint32_t GetCaptureInputCount();
        void GetCaptureInputDisplayIds(array_view<uint32_t> captureInputUniqueId);

    private:
        std::shared_ptr<SoftGpu> m_softGpuAdapter;
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCapturePlugin : GraphicsCapturePluginT<GraphicsCapturePlugin, implementation::GraphicsCapturePlugin>
    {
    };
}
