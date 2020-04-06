#include "pch.h"
#include "SamplePlugin.GraphicsCapturePlugin.h"
#include "SamplePlugin.GraphicsCapturePlugin.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    GraphicsCapturePlugin::GraphicsCapturePlugin()
    {
        m_softGpuAdapter = SoftGpu::GetOrCreate();
    }

    void GraphicsCapturePlugin::InitializeCaptureInput(uint32_t displayId)
    {
        switch (displayId)
        {
        case 0:
            {
                winrt::com_ptr<ISoftGpuApi> softGpuApi(nullptr);
                winrt::com_ptr<ISoftGpuConfig> softGpuConfig(nullptr);

                m_softGpuAdapter->GetSoftGpuApi(softGpuApi.put());
                m_softGpuAdapter->GetSoftGpuConfig(softGpuConfig.put());

                winrt::com_ptr<ISoftGpuMonitor> monitor;
                softGpuConfig->CreateMonitorFromType(L"Monitor_1", MONITOR_1080P_, monitor.put());

                winrt::com_ptr<ISoftGpuAdapter> adapter;
                m_softGpuAdapter->GetSoftGpuAdapter(adapter.put());

                winrt::com_ptr<ISoftGpuTarget> target;
                adapter->GetTarget(displayId, target.put());

                target->ConnectMonitor(monitor.get());

                m_softGpuAdapter->AddMonitor(displayId, monitor.detach());

            }
            break;

        default:
            break;
        }
    }

    uint32_t GraphicsCapturePlugin::GetCaptureInputCount()
    {
        return 1;
    }

    void GraphicsCapturePlugin::GetCaptureInputDisplayIds(array_view<uint32_t> captureInputUniqueId)
    {
        captureInputUniqueId[0] = 0;
    }
}