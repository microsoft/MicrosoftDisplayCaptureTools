﻿#pragma once
#include "Framework.Core.g.h"

#include "winrt/MicrosoftDisplayCaptureTools.CaptureCard.h"
#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    const std::wstring c_CapturePluginDefaultName        = L".ControllerFactory"; 
    const std::wstring c_ConfigurationToolboxDefaultName = L".ToolboxFactory";
    const std::wstring c_DisplayEngineDefaultName        = L".DisplayEngineFactory";

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
        SourceToSinkMapping(CaptureCard::IDisplayInput const& sink, winrt::Windows::Devices::Display::Core::DisplayTarget const& source);
        ~SourceToSinkMapping();

        CaptureCard::IDisplayInput Sink();
        winrt::Windows::Devices::Display::Core::DisplayTarget Source();

    private:
        const CaptureCard::IDisplayInput m_sink;
        const winrt::Windows::Devices::Display::Core::DisplayTarget m_source;
    };

    struct Core : CoreT<Core>
    {
        Core();
        Core(Framework::ILogger const& logger);

        Core::~Core()
        {
        }

        CaptureCard::IController LoadCapturePlugin(hstring const& pluginPath, hstring const& className);
        CaptureCard::IController LoadCapturePlugin(hstring const& pluginPath);

        ConfigurationTools::IConfigurationToolbox LoadToolbox(hstring const& toolboxPath, hstring const& className);
        ConfigurationTools::IConfigurationToolbox LoadToolbox(hstring const& toolboxPath);

        Display::IDisplayEngine LoadDisplayManager(hstring const& displayEnginePath, hstring const& className);
        Display::IDisplayEngine LoadDisplayManager(hstring const& displayEnginePath);

        void LoadConfigFile(hstring const& configFilePath);

        winrt::Windows::Foundation::IClosable LockFramework();

        com_array<ConfigurationTools::IConfigurationTool> GetLoadedTools();
        com_array<CaptureCard::IController> GetCaptureCards();
        Display::IDisplayEngine GetDisplayEngine();

        
        winrt::Windows::Foundation::Collections::IVector<Framework::ISourceToSinkMapping> GetSourceToSinkMappings(bool regenerateMappings);

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

        bool IsFrameworkLocked()
        {
            return m_lockCount > 0;
        }

    private:
        // The capture card object represented by the capture plugin
        std::vector<CaptureCard::IController> m_captureCards;

        // The object which manages the displays under test
        Display::IDisplayEngine m_displayEngine;

        // A list of all ConfigurationToolboxes that have been loaded
        std::vector<ConfigurationTools::IConfigurationToolbox> m_toolboxes;

        // A list of all tools loaded from all toolboxes
        std::vector<ConfigurationTools::IConfigurationTool> m_toolList;

        // A JSON object representing the configfile
        winrt::Windows::Data::Json::JsonObject m_configFile;

        // A map parsed from the configuration file which identifies which DisplayTargets match up with which IDisplayInputs
        // from the IController plugin.
        std::vector<ISourceToSinkMapping> m_displayMappingsFromFile;

        // The logging system for this framework instance
        const ILogger m_logger;

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
