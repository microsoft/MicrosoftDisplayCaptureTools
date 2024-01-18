#pragma once
#include "Framework.MonitorDescriptorLoader.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    struct MonitorDescriptorLoader
    {
        MonitorDescriptorLoader() = default;

        static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor LoadDescriptorFromFile(hstring const& filePath);
        static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor LoadDescriptorFromBuffer(winrt::Windows::Storage::Streams::IBuffer const& data);
        static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor LoadDescriptorFromVectorView(winrt::Windows::Foundation::Collections::IVectorView<uint8_t> const& data);
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct MonitorDescriptorLoader : MonitorDescriptorLoaderT<MonitorDescriptorLoader, implementation::MonitorDescriptorLoader>
    {
    };
}
