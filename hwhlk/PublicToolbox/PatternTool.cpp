#include "pch.h"
#include "PatternTool.h"

namespace winrt::ConfigurationTools::implementation
{
    PatternTool::PatternTool()
    {
        // Define the default configuration
        m_currentConfig = Configurations::Black;
    }

    hstring PatternTool::Name()
    {
        return L"PatternTool";
    }

    ConfigurationTools::ConfigurationToolCategory PatternTool::Category()
    {
        return ConfigurationTools::ConfigurationToolCategory::Render;
    }

    ConfigurationTools::ConfigurationToolRequirements PatternTool::Requirements()
    {
        auto reqs = ConfigurationTools::ConfigurationToolRequirements();
        reqs.ContributedComparisonTolerance = 0.f;
        reqs.MaxComparisonTolerance = 0.f;

        return reqs;
    }

    std::map<std::wstring, Configurations> MapNameToConfiguration =
    {
        {L"Black", Configurations::Black},
        {L"White", Configurations::White},
        {L"Red",   Configurations::Red},
        {L"Green", Configurations::Green},
        {L"Blue",  Configurations::Blue}
    };

    com_array<hstring> PatternTool::GetSupportedConfigurations()
    {
        auto configNames = std::vector<hstring>();
        for (auto tool : MapNameToConfiguration)
        {
            configNames.push_back(hstring(tool.first));
        }

        return com_array<hstring>(configNames);
    }

    void PatternTool::SetConfiguration(hstring const& configuration)
    {
        m_currentConfig = MapNameToConfiguration[std::wstring(configuration)];
    }

    void PatternTool::ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target)
    {
        throw hresult_not_implemented();
    }

    void PatternTool::ApplyToSoftwareReference(DisplayStateReference::StaticReference const& reference)
    {
        throw hresult_not_implemented();
    }
}
