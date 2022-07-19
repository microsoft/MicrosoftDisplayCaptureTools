#pragma once
#include "Framework.Core.g.h"

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    const std::wstring c_CapturePluginDefaultName        = L".ControllerFactory"; 
    const std::wstring c_ConfigurationToolboxDefaultName = L".ToolboxFactory";
    const std::wstring c_DisplayEngineDefaultName       = L".DisplayEngineFactory";

    struct Core : CoreT<Core>
    {
        Core();
        Core(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        void LoadCapturePlugin(hstring const& pluginPath, hstring const& className);
        void LoadCapturePlugin(hstring const& pluginPath);

        void LoadToolbox(hstring const& toolboxPath, hstring const& className);
        void LoadToolbox(hstring const& toolboxPath);

        void LoadDisplayManager(hstring const& displayEnginePath, hstring const& className);
        void LoadDisplayManager(hstring const& displayEnginePath);

        void LoadConfigFile(hstring const& configFilePath);
        void RunTest();
        com_array<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool> GetLoadedTools();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController GetCaptureCard();
        winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngine GetDisplayEngine();

        hstring Version()
        {
            return L"0.1";
        };

    private:
        // Iterate through Toolboxes and consolidate a single list of all tools from all sources.
        void UpdateToolList();

    private:
        // The capture card object represented by the capture plugin
        CaptureCard::IController m_captureCard = nullptr;

        // The object which manages the display under test
        Display::IDisplayEngine m_displayEngine = nullptr;

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
        std::recursive_mutex m_testLock;

        // The logging system for this framework instance
        const ILogger m_logger;
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct Core : CoreT<Core, implementation::Core>
    {
    };
}
