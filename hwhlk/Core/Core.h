#pragma once
#include "MicrosoftDisplayCaptureTools.Framework.Core.g.h"

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    struct Core : CoreT<Core>
    {
        Core() = default;

        void LoadPlugin(hstring const& pluginPath, hstring const& className);
        void LoadToolbox(hstring const& toolboxPath, hstring const& className);
        void LoadDisplayManager(hstring const& displayManagerPath, hstring const& className);
        void LoadConfigFile(hstring const& configFilePath);
        void RunTest();
        com_array<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool> GetLoadedTools();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController GetCaptureCard();
        winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngine GetDisplayEngine();

    private:
        // Iterate through Toolboxes and consolidate a single list of all tools from all sources.
        void UpdateToolList();

    private:
        // The capture card object represented by the capture plugin
        CaptureCard::IController m_captureCard = nullptr;

        // The object which manages the display under test
        Display::IDisplayEngine m_displayManager = nullptr;

        // A list of all ConfigurationToolboxes that have been loaded
        std::vector<ConfigurationTools::IConfigurationToolbox> m_toolboxes;

        // A list of all tools loaded from all toolboxes
        std::vector<ConfigurationTools::IConfigurationTool> m_toolList;

        // A JSON object representing the configfile
        winrt::Windows::Data::Json::JsonObject m_configFile;

        // A map parsed from the configuration file which identifies which DisplayTargets match up with which IDisplayInputs
        // from the IController plugin.
        std::map<winrt::hstring, winrt::hstring> m_targetMap;

        // Loading components/config files is not allowed while a test is running and vice-versa.
        std::mutex m_testLock;
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct Core : CoreT<Core, implementation::Core>
    {
    };
}
