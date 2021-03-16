#pragma once
#include "Framework.g.h"

namespace winrt::Core::implementation
{
    struct TestRun
    {
        std::vector<winrt::ConfigurationTools::ConfigurationTool> toolRunList;

        std::vector<winrt::ConfigurationTools::ConfigurationTool> toolOrderedRunList;
    };

    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        Framework(hstring const& pluginPath);
        void OpenToolbox(hstring const& toolboxPath);
        void RunPictTest();

        std::vector<winrt::ConfigurationTools::ConfigurationToolbox> m_toolboxes;
        std::shared_ptr<winrt::CaptureCard::IController> m_captureCard;
    };
}
namespace winrt::Core::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
