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
        std::shared_ptr<winrt::CaptureCard::IController> m_captureCard;
    };
    
    struct TestRun
    {
        std::vector<winrt::ConfigurationTools::IConfigurationTool> toolRunList;
        std::vector<winrt::ConfigurationTools::IConfigurationTool> toolOrderedRunList;

        bool operator()(winrt::ConfigurationTools::IConfigurationTool a, winrt::ConfigurationTools::IConfigurationTool b) const;
    };
}
namespace winrt::Core::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
