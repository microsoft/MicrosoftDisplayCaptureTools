#pragma once
#include "Framework.Core.g.h"

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    // Constant names used for automatically discovering installed plugins for this framework. These assume that items are
    // installed via the preferred mechanism, our nuget packages.
    const std::wstring c_CapturePluginDirectory        = L"CaptureCards";
    const std::wstring c_ConfigurationToolboxDirectory = L"Toolboxes";
    const std::wstring c_DisplayEngineDirectory        = L"DisplayEngines";

    const std::wstring c_CapturePluginDefaultName        = L".ControllerFactory"; 
    const std::wstring c_ConfigurationToolboxDefaultName = L".ToolboxFactory";
    const std::wstring c_DisplayEngineDefaultName        = L".DisplayEngineFactory";

    const std::wstring c_CoreFrameworkName = L"MicrosoftDisplayCaptureTools.dll";

    // Struct to ensure that the framework can be locked down to prevent component changes (i.e. loading new components).
    // This is implemented with a basic refcount - m_lockCount within the Core object - wrapped in this winrt object that 
    // can be passed over ABI boundaries.
    struct TestLock : implements<TestLock, winrt::Windows::Foundation::IClosable>
    {
        TestLock(std::atomic_int32_t* lock) : m_lockCount(lock)
        {
            (*m_lockCount)++;
        };

        ~TestLock()
        {
            Close();
        };

        void Close()
        {
            (*m_lockCount)--;
        }

    private:
        // Loading components/config files is not allowed while a test is running and vice-versa.
        std::atomic_int32_t* m_lockCount;
    };

    struct SourceToSinkMapping : implements<SourceToSinkMapping, ISourceToSinkMapping>
    {
        SourceToSinkMapping(winrt::Windows::Devices::Display::Core::DisplayTarget const& source, CaptureCard::IDisplayInput const& sink);
        ~SourceToSinkMapping();

        winrt::Windows::Devices::Display::Core::DisplayTarget Source();
        CaptureCard::IDisplayInput Sink();

    private:
        const winrt::Windows::Devices::Display::Core::DisplayTarget m_source;
        const CaptureCard::IDisplayInput m_sink;
    };

    struct Core : CoreT<Core>
    {
        Core();
        Core(Framework::ILogger const& logger, Framework::IRuntimeSettings const& runtimeSettings);

        CaptureCard::IController LoadCapturePlugin(hstring const& pluginPath, hstring const& className);
        CaptureCard::IController LoadCapturePlugin(hstring const& pluginPath);

        ConfigurationTools::IConfigurationToolbox LoadToolbox(hstring const& toolboxPath, hstring const& className);
        ConfigurationTools::IConfigurationToolbox LoadToolbox(hstring const& toolboxPath);

        Display::IDisplayEngine LoadDisplayEngine(hstring const& displayEnginePath, hstring const& className);
        Display::IDisplayEngine LoadDisplayEngine(hstring const& displayEnginePath);

        void LoadConfigFile(hstring const& configFilePath);

        // Search through the file structure relative to the core binary to determine installed components.
        void DiscoverInstalledPlugins();

        winrt::Windows::Foundation::IClosable LockFramework();

        com_array<ConfigurationTools::IConfigurationToolbox> GetConfigurationToolboxes();
        com_array<CaptureCard::IController> GetCaptureCards();
        com_array<Display::IDisplayEngine> GetDisplayEngines();
        
        winrt::Windows::Foundation::Collections::IVector<Framework::ISourceToSinkMapping> GetSourceToSinkMappings(
            bool regenerateMappings,
            Display::IDisplayEngine displayEngine,
            ConfigurationTools::IConfigurationToolbox toolbox,
            CaptureCard::IController captureCard,
            CaptureCard::IDisplayInput displayInput);

        MicrosoftDisplayCaptureTools::Framework::Version Version()
        {
            return MicrosoftDisplayCaptureTools::Framework::Version(0, 1, 0);
        };

    private:

        bool IsFrameworkLocked()
        {
            return m_lockCount > 0;
        }

        std::vector<ConfigurationTools::IConfigurationTool> GetAllTools(ConfigurationTools::IConfigurationToolbox specificToolbox);

        winrt::hstring GetNamespaceForPlugin(winrt::hstring const& pluginPath);

    private:
        // A list of all capture card plugins wthat have been loaded
        std::vector<CaptureCard::IController> m_captureCards;

        // A list of all DisplayEngines that have been loaded (generally only one will be used at a time)
        std::vector<Display::IDisplayEngine> m_displayEngines;

        // A list of all ConfigurationToolboxes that have been loaded
        std::vector<ConfigurationTools::IConfigurationToolbox> m_toolboxes;

        // A map parsed from the configuration file which identifies which DisplayTargets match up with which IDisplayInputs
        // from the IController plugin.
        std::vector<ISourceToSinkMapping> m_displayMappingsFromFile;

        // The logging system for this framework instance
        const ILogger m_logger;

        // The runtime settings wrapper for this framework instance
        const IRuntimeSettings m_runtimeSettings;

        // Has a test locked components
        std::atomic_int32_t m_lockCount = 0;
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct Core : CoreT<Core, implementation::Core>
    {
    };
}
