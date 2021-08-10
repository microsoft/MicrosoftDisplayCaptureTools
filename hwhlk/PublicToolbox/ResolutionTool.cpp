#include "pch.h"
#include "ResolutionTool.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

using namespace winrt::MicrosoftDisplayCaptureTools;

namespace winrt::Toolbox::implementation
{
    ResolutionTool::ResolutionTool()
    {
        // Define the default configuration
        m_currentConfig = ResolutionToolConfigurations::_1080p;
    }

    hstring ResolutionTool::Name()
    {
        return L"ResolutionTool";
    }

    ConfigurationTools::ConfigurationToolCategory ResolutionTool::Category()
    {
        return ConfigurationTools::ConfigurationToolCategory::DisplaySetup;
    }

    ConfigurationTools::ConfigurationToolRequirements ResolutionTool::Requirements()
    {
        auto reqs = ConfigurationTools::ConfigurationToolRequirements();
        reqs.ContributedComparisonTolerance = 0.f;
        reqs.MaxComparisonTolerance = 0.f;

        return reqs;
    }

    std::map<std::wstring, ResolutionToolConfigurations> MapNameToConfiguration =
    {
        {L"1080p", ResolutionToolConfigurations::_1080p},
        {L"2160p", ResolutionToolConfigurations::_2160p}
    };

    com_array<hstring> ResolutionTool::GetSupportedConfigurations()
    {
        auto configNames = std::vector<hstring>();
        for (auto tool : MapNameToConfiguration)
        {
            configNames.push_back(hstring(tool.first));
        }

        return com_array<hstring>(configNames);
    }

    void ResolutionTool::SetConfiguration(hstring const& configuration)
    {
        m_currentConfig = MapNameToConfiguration[std::wstring(configuration)];
    }

    void ResolutionTool::ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target)
    {
    }

    void ResolutionTool::ApplyToReference(DisplayStateReference::IStaticReference const& reference)
    {
        auto frameInfo = reference.FrameInfo();

        switch (m_currentConfig)
        {
        case ResolutionToolConfigurations::_1080p:
            frameInfo.width = 1920;
            frameInfo.height = 1080;
            break;
        case ResolutionToolConfigurations::_2160p:
            frameInfo.width = 3840;
            frameInfo.height = 2160;
            break;
        default:
            Log::Error(L"An unsupported resolution has been chosen");
        }

        reference.FrameInfo(frameInfo);
    }
}
