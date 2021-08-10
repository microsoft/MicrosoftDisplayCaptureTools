#pragma once
#include "Core.Framework.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Core::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        Framework(hstring const& pluginPath);
        void OpenToolbox(hstring const& toolboxPath);
        void RunPictTest();

    private:
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput ChooseDisplay();

    private:
        std::vector<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox> m_toolboxes;
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController m_captureCard;
        boolean m_runSoftwareOnly;
    };
    
    struct TestRun
    {
        std::vector<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool> toolRunList;
        std::vector<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool> toolOrderedRunList;

        bool operator()(winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool a, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool b) const;
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Core::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
