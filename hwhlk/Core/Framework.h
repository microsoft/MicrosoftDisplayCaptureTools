#pragma once
#include "Framework.g.h"

namespace winrt::Core::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        Framework(hstring const& pluginPath);
        void OpenToolbox(hstring const& toolboxPath);
        void RunPictTest();

        std::vector<winrt::ConfigurationTools::ConfigurationToolbox> m_toolboxes;
    };
}
namespace winrt::Core::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
