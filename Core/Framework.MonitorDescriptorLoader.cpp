#include "pch.h"
#include "Framework.MonitorDescriptorLoader.h"
#include "Framework.MonitorDescriptorLoader.g.cpp"
#include "EDIDDescriptor.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor MonitorDescriptorLoader::LoadDescriptorFromFile(hstring const& filePath)
    {
        // TODO: based on the file, load the appropriate descriptor type
        return winrt::MicrosoftDisplayCaptureTools::Framework::Utilities::EDIDDescriptor::CreateEDIDFromFile(filePath);
    }
    winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor MonitorDescriptorLoader::LoadDescriptorFromBuffer(winrt::Windows::Storage::Streams::IBuffer const& data)
    {
        std::vector<uint8_t> vec(data.data(), data.data() + data.Length());
        return winrt::make<winrt::MicrosoftDisplayCaptureTools::Framework::Utilities::EDIDDescriptor>(vec);
    }
    winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor MonitorDescriptorLoader::LoadDescriptorFromVectorView(winrt::Windows::Foundation::Collections::IVectorView<uint8_t> const& data)
    {
        return winrt::make<winrt::MicrosoftDisplayCaptureTools::Framework::Utilities::EDIDDescriptor>(data);
    }
}
