#pragma once

#include "CaptureCardPlugin.g.h"

namespace winrt::MicrosoftHardwareVerification::implementation
{
    struct CaptureCardPlugin : CaptureCardPluginT<CaptureCardPlugin>
    {
        CaptureCardPlugin() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::MicrosoftHardwareVerification::factory_implementation
{
    struct CaptureCardPlugin : CaptureCardPluginT<CaptureCardPlugin, implementation::CaptureCardPlugin>
    {
    };
}
