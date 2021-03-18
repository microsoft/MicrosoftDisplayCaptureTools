#include "pch.h"
#include "ConfigurationTools.ConfigurationToolbox.h"
#include "ConfigurationTools.ConfigurationToolbox.g.cpp"

namespace winrt::ConfigurationTools::implementation
{
    hstring ConfigurationToolbox::Name()
    {
        return L"Software Test Plugin Toolbox";
    }
    com_array<hstring> ConfigurationToolbox::GetSupportedTools()
    {
        throw hresult_not_implemented();
    }
    ConfigurationTools::IConfigurationTool ConfigurationToolbox::GetTool(hstring const& toolName)
    {
        throw hresult_not_implemented();
    }
}
