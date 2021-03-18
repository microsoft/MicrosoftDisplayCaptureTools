#pragma once
#include "ConfigurationTools.ConfigurationToolbox.g.h"

namespace winrt::ConfigurationTools::implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox>
    {
        ConfigurationToolbox() = default;

        hstring Name();
        com_array<hstring> GetSupportedTools();
        ConfigurationTools::IConfigurationTool GetTool(hstring const& toolName);
    };
}
namespace winrt::ConfigurationTools::factory_implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox, implementation::ConfigurationToolbox>
    {
    };
}
