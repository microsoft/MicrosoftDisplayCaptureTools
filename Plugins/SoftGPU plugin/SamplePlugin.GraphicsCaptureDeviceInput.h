#pragma once
#include "SamplePlugin.GraphicsCaptureDeviceInput.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput>
    {
        GraphicsCaptureDeviceInput();
        ~GraphicsCaptureDeviceInput();

        void InitializeFromId(uint32_t captureInputUniqueId, SamplePlugin::GraphicsCapturePlugin const& plugin);
        SamplePlugin::DeviceInputState GetState();
        SamplePlugin::CaptureCapabilities GetCaptureCaps();
        SamplePlugin::GraphicsCapturedFrame CaptureFrame();
        uint32_t UniqueId();

    private:
        uint32_t m_uniqueId;
        const SamplePlugin::GraphicsCapturePlugin* m_plugin;

        std::shared_ptr<SoftGpu> m_softGpu;

        winrt::com_ptr<ISoftGpuAdapter> m_adapter;
        winrt::com_ptr<IPeekApi> m_peekApi;
        winrt::com_ptr<IFrameProvider> m_frameProvider;
        winrt::com_ptr<ISoftGpuTarget> m_target;
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCaptureDeviceInput : GraphicsCaptureDeviceInputT<GraphicsCaptureDeviceInput, implementation::GraphicsCaptureDeviceInput>
    {
    };
}
