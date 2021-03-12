#include "pch.h"
#include "ConfigurationTools.ConfigurationToolbox.h"
#include "ConfigurationTools.ConfigurationToolbox.g.cpp"

namespace winrt::ConfigurationTools::implementation
{
    hstring ConfigurationToolbox::Name()
    {
        throw hresult_not_implemented();
    }
    void ConfigurationToolbox::Name(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    void ConfigurationToolbox::GetSupportedTools(array_view<hstring> tools)
    {
        throw hresult_not_implemented();
    }
    ConfigurationTools::ConfigurationTool ConfigurationToolbox::GetTool(hstring const& toolName)
    {
        throw hresult_not_implemented();
    }
}
