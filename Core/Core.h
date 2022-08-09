#pragma once
#include "Framework.Core.g.h"

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    const std::wstring c_CapturePluginDefaultName        = L".ControllerFactory"; 
    const std::wstring c_ConfigurationToolboxDefaultName = L".ToolboxFactory";
    const std::wstring c_DisplayEngineDefaultName        = L".DisplayEngineFactory";

    // Struct to ensure that all components are locked and can't be modified while a user is running tests
    struct TestLock : implements<TestLock, winrt::Windows::Foundation::IClosable>
    {
        TestLock(std::atomic_bool* testLock) : m_isLocked(testLock)
        {
            *m_isLocked = true;
        };

        ~TestLock()
        {
            Close();
        };

        void Close()
        {
            *m_isLocked = false;
        }

        bool IsLocked()
        {
            return *m_isLocked;
        };

    private:
        // Loading components/config files is not allowed while a test is running and vice-versa.
        std::atomic_bool* m_isLocked;
    };

    struct SourceToSinkMapping : implements <SourceToSinkMapping,ISourceToSinkMapping>
    {
        SourceToSinkMapping(CaptureCard::IDisplayInput sink, winrt::Windows::Devices::Display::Core::DisplayTarget source);

        CaptureCard::IDisplayInput GetSink();
        winrt::Windows::Devices::Display::Core::DisplayTarget GetSource();

    private:
        const CaptureCard::IDisplayInput m_sink;
        const winrt::Windows::Devices::Display::Core::DisplayTarget m_source;
    };

    struct Core : CoreT<Core>
    {
        Core();
        Core(Framework::ILogger const& logger);

        void LoadCapturePlugin(hstring const& pluginPath, hstring const& className);
        void LoadCapturePlugin(hstring const& pluginPath);

        void LoadToolbox(hstring const& toolboxPath, hstring const& className);
        void LoadToolbox(hstring const& toolboxPath);

        void LoadDisplayManager(hstring const& displayEnginePath, hstring const& className);
        void LoadDisplayManager(hstring const& displayEnginePath);

        void LoadConfigFile(hstring const& configFilePath);

        winrt::Windows::Foundation::IClosable LockFramework();

        com_array<ConfigurationTools::IConfigurationTool> GetLoadedTools();
        CaptureCard::IController GetCaptureCard();
        Display::IDisplayEngine GetDisplayEngine();

        
        com_array<Framework::ISourceToSinkMapping> GetSourceToSinkMappings(bool regenerateMappings);

        hstring Version()
        {
            return L"0.1";
        };

        winrt::MicrosoftDisplayCaptureTools::Framework::ILogger Logger()
        {
            return m_logger;
        }

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

        // The logging system for this framework instance
        const ILogger m_logger;

        // Has a test locked components
        std::atomic_bool m_isLocked = false;
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct Core : CoreT<Core, implementation::Core>
    {
    };
}
