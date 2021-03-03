#pragma once

#include "ConfigurationTools.g.h"

namespace winrt::MicrosoftHardwareVerification::implementation
{
    struct ConfigurationTools : ConfigurationToolsT<ConfigurationTools>
    {
        ConfigurationTools() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::MicrosoftHardwareVerification::factory_implementation
{
    struct ConfigurationTools : ConfigurationToolsT<ConfigurationTools, implementation::ConfigurationTools>
    {
    };
}
