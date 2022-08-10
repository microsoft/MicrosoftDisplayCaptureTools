#include "pch.h"
#include "Core.h"
#include "Logger.h"
#include "Framework.Core.g.cpp"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Data::Json;
    using namespace winrt::Windows::Storage;
    using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::MicrosoftDisplayCaptureTools::Display;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities;
    using namespace winrt::MicrosoftDisplayCaptureTools::Libraries;
}

namespace std
{
    using namespace std::filesystem;
}

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    // Constructor that uses the default logger
    Core::Core() : m_logger(winrt::make<winrt::Logger>().as<ILogger>())
    {
        m_logger.LogNote(L"Initializing MicrosoftDisplayCaptureTools v" + this->Version());
    }

    // Constructor taking a caller-defined logging class
    Core::Core(ILogger const& logger) : m_logger(logger)
    {
        m_logger.LogNote(L"Initializing MicrosoftDisplayCaptureTools v" + this->Version());
    }

    IController Core::LoadCapturePlugin(hstring const& pluginPath, hstring const& className)
    {
        // Ensure that a component can't be changed if a test has locked the framework
        if (m_isLocked)
        {
            m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
            throw winrt::hresult_illegal_method_call();
        }

        // Load the capture card from the provided path.
        auto captureCardFactory = LoadInterfaceFromPath<IControllerFactory>(pluginPath, className);

        auto captureCardController = captureCardFactory.CreateController(m_logger);
        m_captureCards.push_back(captureCardController);

        m_logger.LogNote(L"Using Capture Plugin: " + captureCardController.Name() + L", Version " + captureCardController.Version());

        return captureCardController;
    }

    IController Core::LoadCapturePlugin(hstring const& pluginPath)
    {
        // Create the className string from 
        winrt::hstring className = std::path(pluginPath.c_str()).stem().c_str();
        return LoadCapturePlugin(pluginPath, className + c_CapturePluginDefaultName);
    }

    IConfigurationToolbox Core::LoadToolbox(hstring const& toolboxPath, hstring const& className)
    {
        // Ensure that a component can't be changed if a test has locked the framework
        if (m_isLocked)
        {
            m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
            throw winrt::hresult_illegal_method_call();
        }

        // Load the toolbox from the provided path.
        auto toolboxFactory = LoadInterfaceFromPath<IConfigurationToolboxFactory>(toolboxPath, className);

        auto toolbox = toolboxFactory.CreateConfigurationToolbox(m_logger);

        m_toolboxes.push_back(toolbox);

        m_logger.LogNote(L"Using Toolbox: " + m_toolboxes[0].Name() + L", Version " + m_toolboxes[0].Version());

        UpdateToolList();

        return toolbox;
    }

    IConfigurationToolbox Core::LoadToolbox(hstring const& toolboxPath)
    {
        // Create the className string from
        winrt::hstring className = std::path(toolboxPath.c_str()).stem().c_str();
        return LoadToolbox(toolboxPath, className + c_ConfigurationToolboxDefaultName);
    }

    IDisplayEngine Core::LoadDisplayManager(hstring const& displayEnginePath, hstring const& className)
    {
        // Ensure that a component can't be changed if a test has locked the framework
        if (m_isLocked)
        {
            m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
            throw winrt::hresult_illegal_method_call();
        }

        // Load the toolbox from the provided path.
        auto displayEngineFactory = LoadInterfaceFromPath<IDisplayEngineFactory>(displayEnginePath, className);

        m_displayEngine = displayEngineFactory.CreateDisplayEngine(m_logger);

        m_logger.LogNote(L"Using DisplayManager: " + m_displayEngine.Name() + L", Version " + m_displayEngine.Version());

        return m_displayEngine;
    }

    IDisplayEngine Core::LoadDisplayManager(hstring const& displayEnginePath)
    {
        // Create the className string from
        winrt::hstring className = std::path(displayEnginePath.c_str()).stem().c_str();
        return LoadDisplayManager(displayEnginePath, className + c_CapturePluginDefaultName);
    }

    void Core::LoadConfigFile(hstring const& configFile)
    {
        m_logger.LogNote(L"Loading configuration...");

        // Ensure that a component can't be changed if a test has locked the framework
        if (m_isLocked)
        {
            m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
            throw winrt::hresult_illegal_method_call();
        }

        // First try to see if the string passed in is a self-contained json file
        auto jsonObject = JsonObject();
        auto hasParsed = JsonObject::TryParse(configFile, jsonObject);

        // if no, treat the argument as a fully qualified path
        if (!hasParsed)
        {
            try
            {
                auto file = StorageFile::GetFileFromPathAsync(configFile).get();
                auto text = winrt::FileIO::ReadTextAsync(file).get();
                hasParsed = JsonObject::TryParse(text, jsonObject);
            }
            catch (...)
            {
                // we're going to try again, so don't bubble exeptions from this up.
            }
        }
       
        // if neither of the previous has worked, try a final time as a local path
        if (!hasParsed)
        {
            auto cwd = std::filesystem::current_path();
            winrt::hstring path = winrt::hstring(cwd.c_str()) + L"\\" + configFile;

            auto file = StorageFile::GetFileFromPathAsync(path).get();
            auto text = winrt::FileIO::ReadTextAsync(file).get();
            hasParsed = JsonObject::TryParse(text, jsonObject);
        }

        if (!hasParsed)
        {
            m_logger.LogError(L"Loading Configuration Failed.");
            throw winrt::hresult_error();
        }

        m_configFile = jsonObject;

        // Dump the config file to the log - this is to make debugging easier
        m_logger.LogConfig(m_configFile.ToString());

        // Parse component information out of the config file
        {
            auto componentConfigDataValue = m_configFile.TryLookup(L"Components");

            if (!componentConfigDataValue || componentConfigDataValue.ValueType() == winrt::JsonValueType::Null)
            {
                // No data was found.
                m_logger.LogWarning(L"Configuration doesn't specify components to load.");
            }
            else
            {
                auto componentConfigData = componentConfigDataValue.GetObjectW();

                // Attempt to load the DisplayManager
                auto displayEngineValue = componentConfigData.TryLookup(L"DisplayEngine");
                if (displayEngineValue && displayEngineValue.ValueType() == winrt::JsonValueType::Object)
                {
                    auto displayEngine = displayEngineValue.GetObjectW();

                    LoadDisplayManager(displayEngine.GetNamedString(L"Path"), displayEngine.GetNamedString(L"Class"));

                    auto displayEngineConfig = displayEngine.TryLookup(L"Settings");
                    m_displayEngine.SetConfigData(displayEngineConfig);
                }

                // Attempt to load the CapturePlugin
                auto capturePluginValue = componentConfigData.TryLookup(L"CapturePlugin");
                if (capturePluginValue && capturePluginValue.ValueType() == winrt::JsonValueType::Object)
                {
                    auto capturePlugin = capturePluginValue.GetObjectW();

                    auto captureCard = LoadCapturePlugin(capturePlugin.GetNamedString(L"Path"), capturePlugin.GetNamedString(L"Class"));

                    auto captureCardConfig = capturePlugin.TryLookup(L"Settings");
                    captureCard.SetConfigData(captureCardConfig);
                }

                // Attempt to load any ConfigurationToolboxes, if applicable
                auto configurationToolboxesValue = componentConfigData.TryLookup(L"ConfigurationToolboxes");
                if (configurationToolboxesValue && configurationToolboxesValue.ValueType() == winrt::JsonValueType::Array)
                {
                    auto configurationToolboxes = configurationToolboxesValue.GetArray();

                    for (auto toolbox : configurationToolboxes)
                    {
                        auto toolboxConfigEntry = toolbox.GetObjectW();

                        LoadToolbox(toolboxConfigEntry.GetNamedString(L"Path"), toolboxConfigEntry.GetNamedString(L"Class"));

                        auto toolboxConfig = toolboxConfigEntry.TryLookup(L"Settings");
                        m_toolboxes.back().SetConfigData(toolboxConfig);
                    }

                    UpdateToolList();
                }
            }
        }

        // Parse test system information out of the config file
        {
            auto testSystemConfigDataValue = m_configFile.TryLookup(L"TestSystem");

            if (!testSystemConfigDataValue || testSystemConfigDataValue.ValueType() == winrt::JsonValueType::Null)
            {
                // No data was found.
                m_logger.LogWarning(L"Configuration does not specify test system setup.");
            }
            else
            {
                // TODO: assert that we do in fact have at least a displayEngine and a capture plugin, otherwise these mappings don't make sense.
                auto testSystemConfigData = testSystemConfigDataValue.GetObjectW();

                auto displayInputMappingObject = testSystemConfigData.TryLookup(L"DisplayInputMapping");
                if (displayInputMappingObject && displayInputMappingObject.ValueType() == winrt::JsonValueType::Array)
                {
                    auto displayInputMappingArray = displayInputMappingObject.GetArray();

                    for (auto displayInputMapping : displayInputMappingArray)
                    {
                        auto mapping = displayInputMapping.GetObjectW();

                        auto pluginName = mapping.GetNamedString(L"CapturePluginName");
                        auto pluginInputName = mapping.GetNamedString(L"PluginInputName");
                        auto displayId = mapping.GetNamedString(L"DisplayId");

                        m_logger.LogConfig(hstring(L"Display: ") + displayId + L" connected to " + pluginName + L"." + pluginInputName);

                        IDisplayInput foundInput = nullptr;
                        for (auto&& plugin : m_captureCards)
                        {
                            if (plugin.Name() == pluginName)
                            {
                                for (auto&& input : plugin.EnumerateDisplayInputs())
                                {
                                    if (input.Name() == pluginInputName)
                                    {
                                        foundInput = input;

                                        // TODO: Remove this call - if the machine is going to be set up via config file, then the caller is responsible
                                        //       for ensuring that the system is setup before trying to load the config file.
                                        foundInput.FinalizeDisplayState();
                                    }
                                }
                            }
                        }

                        if (!foundInput)
                        {
                            m_logger.LogError(L"Display input mapped from config file not found.");
                        }

                        m_displayMapping[foundInput] = m_displayEngine.InitializeOutput(displayId);
                    }
                }
            }
        }
    }

    com_array<IConfigurationTool> Core::GetLoadedTools()
    {
        return com_array<IConfigurationTool>(m_toolList);
    }

    com_array<IController> Core::GetCaptureCards()
    {
        return com_array<IController>(m_captureCards);
    }

    IDisplayEngine Core::GetDisplayEngine()
    {
        return m_displayEngine;
    }

    IVector<ISourceToSinkMapping> Core::GetSourceToSinkMappings(bool regenerateMappings)
    {
        auto mappings = winrt::single_threaded_vector<ISourceToSinkMapping>();

        // TODO: delete existing mapping and regerenate them if regenerateMappings
        for (auto&& entry : m_displayMapping)
        {
            mappings.Append(winrt::make<SourceToSinkMapping>(entry.first, entry.second));
        }

        return mappings;
    }

    void Core::UpdateToolList()
    {
        if (m_toolboxes.empty()) return;

        m_toolList.clear();

        for (auto&& toolbox : m_toolboxes)
        {
            auto toolList = toolbox.GetSupportedTools();
            for (auto toolName : toolList)
            {
                m_toolList.push_back(toolbox.GetTool(toolName));
            }
        }
    }

    winrt::Windows::Foundation::IClosable Core::LockFramework()
    {
        if (m_isLocked)
        {
            m_logger.LogAssert(L"Attempted to lock framework while already locked!");
            throw winrt::hresult_illegal_method_call();
        }

        return winrt::make<TestLock>(&m_isLocked);
    }

    SourceToSinkMapping::SourceToSinkMapping(IDisplayInput const& sink, IDisplayOutput const& source) : m_sink(sink), m_source(source)
    {
    }

    IDisplayInput SourceToSinkMapping::GetSink()
    {
        return m_sink;
    }

    IDisplayOutput SourceToSinkMapping::GetSource()
    {
        return m_source;
    }
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
