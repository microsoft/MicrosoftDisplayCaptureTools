#include "pch.h"
#include "Core.h"
#include "Logger.h"
#include "Framework.Core.g.cpp"

namespace winrt
{
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
    Core::Core(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) : m_logger(logger)
    {
        m_logger.LogNote(L"Initializing MicrosoftDisplayCaptureTools v" + this->Version());
    }

    void Core::LoadCapturePlugin(hstring const& pluginPath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the capture card from the provided path.
        auto captureCardFactory = LoadInterfaceFromPath<CaptureCard::IControllerFactory>(pluginPath, className);

        m_captureCard = captureCardFactory.CreateController(m_logger);

        m_logger.LogNote(L"Using Capture Plugin: " + m_captureCard.Name() + L", Version " + m_captureCard.Version());
    }

    void Core::LoadCapturePlugin(hstring const& pluginPath)
    {
        // Create the className string from 
        winrt::hstring className = std::path(pluginPath.c_str()).stem().c_str();
        LoadCapturePlugin(pluginPath, className + c_CapturePluginDefaultName);
    }

    void Core::LoadToolbox(hstring const& toolboxPath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the toolbox from the provided path.
        auto toolboxFactory = LoadInterfaceFromPath<ConfigurationTools::IConfigurationToolboxFactory>(toolboxPath, className);

        m_toolboxes.push_back(toolboxFactory.CreateConfigurationToolbox(m_logger));

        m_logger.LogNote(L"Using Toolbox: " + m_toolboxes[0].Name() + L", Version " + m_toolboxes[0].Version());

        UpdateToolList();
    }

    void Core::LoadToolbox(hstring const& toolboxPath)
    {
        // Create the className string from
        winrt::hstring className = std::path(toolboxPath.c_str()).stem().c_str();
        LoadToolbox(toolboxPath, className + c_ConfigurationToolboxDefaultName);
    }

    void Core::LoadDisplayManager(hstring const& displayEnginePath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the toolbox from the provided path.
        auto displayEngineFactory = LoadInterfaceFromPath<Display::IDisplayEngineFactory>(displayEnginePath, className);

        m_displayEngine = displayEngineFactory.CreateDisplayEngine(m_logger);

        m_logger.LogNote(L"Using DisplayManager: " + m_displayEngine.Name() + L", Version " + m_displayEngine.Version());
    }

    void Core::LoadDisplayManager(hstring const& displayEnginePath)
    {
        // Create the className string from
        winrt::hstring className = std::path(displayEnginePath.c_str()).stem().c_str();
        LoadDisplayManager(displayEnginePath, className + c_CapturePluginDefaultName);
    }

    void Core::LoadConfigFile(hstring const& configFile)
    {
        m_logger.LogNote(L"Loading configuration...");

        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

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

                    LoadCapturePlugin(capturePlugin.GetNamedString(L"Path"), capturePlugin.GetNamedString(L"Class"));
                    ;
                    auto captureCardConfig = capturePlugin.TryLookup(L"Settings");
                    m_captureCard.SetConfigData(captureCardConfig);
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
                auto testSystemConfigData = testSystemConfigDataValue.GetObjectW();

                auto displayInputMappingObject = testSystemConfigData.TryLookup(L"DisplayInputMapping");
                if (displayInputMappingObject && displayInputMappingObject.ValueType() == winrt::JsonValueType::Array)
                {
                    auto displayInputMappingArray = displayInputMappingObject.GetArray();

                    for (auto displayInputMapping : displayInputMappingArray)
                    {
                        auto mapping = displayInputMapping.GetObjectW();

                        auto pluginInputName = mapping.GetNamedString(L"PluginInputName");
                        auto displayId = mapping.GetNamedString(L"DisplayId");

                        m_targetMap[pluginInputName] = displayId;
                    }
                }
            }
        }
    }

    void Core::RunTest()
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);
        
        // Make sure the capture card is ready
        // TODO: As with the DisplayEngine note below, this should be done on _every_ target in the map.
        auto captureInput = m_captureCard.EnumerateDisplayInputs()[0];
        captureInput.FinalizeDisplayState();

        // Reset the display manager to the correct 
        auto displayId = m_targetMap[captureInput.Name()];
        m_displayEngine.InitializeForStableMonitorId(displayId);

        winrt::hstring testName = L"";

        for (auto tool : m_toolList)
        {
            tool.Apply(m_displayEngine);
            testName = testName + tool.GetConfiguration() + L"_";
        }

        // Start the render
        // TODO: allow a map of multiple DisplayEngines here, there is a mapping of display Engine to capture card inputs
        //       and a test run should make sure to start operations on all of them.
        auto renderer = m_displayEngine.StartRender();

        // TODO: make this configurable, this is the amount of time we are waiting for display settings to 
        //       stabilize after the 'StartRender' call causes a mode change and the rendering to start
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Capture the frame.
        auto capturedFrame = captureInput.CaptureFrame();
        auto predictedFrame = m_displayEngine.GetPrediction();

        // TODO: build a uniquely identifying string from the currently selected tools

        capturedFrame.CompareCaptureToPrediction(testName, predictedFrame);
    }

    com_array<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool> Core::GetLoadedTools()
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        return winrt::com_array<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>(m_toolList);
    }

    winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController Framework::implementation::Core::GetCaptureCard()
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        return m_captureCard;
    }

    winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngine Framework::implementation::Core::GetDisplayEngine()
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        return m_displayEngine;
    }

    void Framework::implementation::Core::UpdateToolList()
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
}
