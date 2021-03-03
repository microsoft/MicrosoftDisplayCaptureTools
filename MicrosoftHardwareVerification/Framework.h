#pragma once

#include "Framework.g.h"

namespace winrt::MicrosoftHardwareVerification::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        Framework(hstring const& pluginPath);
        hstring Name();
        void Name(hstring const& value);
        void OpenToolbox(hstring const& toolboxPath);
        void RunPictTest();
    };
}

namespace winrt::MicrosoftHardwareVerification::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
