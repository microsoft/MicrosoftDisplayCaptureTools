#include "pch.h"
#include "ConfigurationTools.ConfigurationTool.h"
#include "ConfigurationTools.ConfigurationTool.g.cpp"

namespace winrt::ConfigurationTools::implementation
{
    hstring ConfigurationTool::Name()
    {
        throw hresult_not_implemented();
    }
    ConfigurationTools::ConfigurationToolCategory ConfigurationTool::Category()
    {
        throw hresult_not_implemented();
    }
    void ConfigurationTool::Category(ConfigurationTools::ConfigurationToolCategory const& value)
    {
        throw hresult_not_implemented();
    }
    ConfigurationTools::ConfigurationToolRequirements ConfigurationTool::Requirements()
    {
        throw hresult_not_implemented();
    }
    void ConfigurationTool::Requirements(ConfigurationTools::ConfigurationToolRequirements const& value)
    {
        throw hresult_not_implemented();
    }
    com_array<hstring> ConfigurationTool::GetSupportedConfigurations()
    {
        throw hresult_not_implemented();
    }
    void ConfigurationTool::SetConfiguration(hstring const& configuration)
    {
        throw hresult_not_implemented();
    }
    void ConfigurationTool::ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target)
    {
        throw hresult_not_implemented();
    }
    void ConfigurationTool::ApplyToSoftwareReference(DisplayStateReference::StaticReference const& reference)
    {
        throw hresult_not_implemented();
    }
}
