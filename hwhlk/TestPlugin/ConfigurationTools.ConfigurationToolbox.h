#pragma once
#include "ConfigurationTools.ConfigurationToolbox.g.h"

namespace winrt::ConfigurationTools::implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox>
    {
        ConfigurationToolbox() = default;

        hstring Name();
        void Name(hstring const& value);
        void GetSupportedTools(array_view<hstring> tools);
        ConfigurationTools::ConfigurationTool GetTool(hstring const& toolName);
    };
}
namespace winrt::ConfigurationTools::factory_implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox, implementation::ConfigurationToolbox>
    {
    };
}
