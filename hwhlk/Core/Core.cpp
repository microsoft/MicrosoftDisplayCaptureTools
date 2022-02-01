#include "pch.h"
#include "Core.h"
#include "Framework.Core.g.cpp"

namespace winrt
{
    using namespace winrt::Windows::Data::Json;
    using namespace winrt::Windows::Storage;
    using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::MicrosoftDisplayCaptureTools::Display;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::Libraries;
}

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    void Core::LoadPlugin(hstring const& pluginPath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the capture card from the provided path.
        m_captureCard = LoadInterfaceFromPath<CaptureCard::IController>(pluginPath, className);

        wprintf(L"Plugin Loaded: %s\n", m_captureCard.Name().c_str());
    }

    void Core::LoadToolbox(hstring const& toolboxPath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the toolbox from the provided path.
        m_toolboxes.push_back(LoadInterfaceFromPath<ConfigurationTools::IConfigurationToolbox>(toolboxPath, className));

        wprintf(L"Toolbox Opened: %s\n", m_toolboxes[0].Name().c_str());

        UpdateToolList();
    }

    void Framework::implementation::Core::LoadDisplayManager(hstring const& displayManagerPath, hstring const& className)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        // Load the toolbox from the provided path.
        m_displayManager = LoadInterfaceFromPath<Display::IDisplayEngine>(displayManagerPath, className);

        wprintf(L"DisplayManager Loaded: %s\n", m_displayManager.Name().c_str());
    }

    void Core::LoadConfigFile(hstring const& configFile)
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);

        auto jsonObject = JsonObject();
        auto isJsonString = JsonObject::TryParse(configFile, jsonObject);

        if (!isJsonString)
        {
            auto file = StorageFile::GetFileFromPathAsync(configFile).get();
            auto text = winrt::FileIO::ReadTextAsync(file).get();
            if (!JsonObject::TryParse(text, jsonObject))
            {
                // Parsing the config file failed!
                // TODO - log and fail
                throw winrt::hresult_not_implemented();
            }
        }

        m_configFile = jsonObject;

        // Parse component information out of the config file
        {
            auto componentConfigDataValue = m_configFile.TryLookup(L"Components");

            if (!componentConfigDataValue || componentConfigDataValue.ValueType() == winrt::JsonValueType::Null)
            {
                // No data was found.
                // TODO - log this case
                return;
            }

            auto componentConfigData = componentConfigDataValue.GetObjectW();

            // Attempt to load the DisplayManager
            auto displayManagerValue = componentConfigData.TryLookup(L"DisplayEngine");
            if (displayManagerValue && displayManagerValue.ValueType() == winrt::JsonValueType::Object)
            {
                auto displayManager = displayManagerValue.GetObjectW();

                // Load the toolbox from the provided path.
                m_displayManager = LoadInterfaceFromPath<Display::IDisplayEngine>(
                    displayManager.GetNamedString(L"Path"),
                    displayManager.GetNamedString(L"Class"));

                wprintf(L"DisplayManager Loaded: %s\n", m_displayManager.Name().c_str());

                auto displayManagerConfig = displayManager.TryLookup(L"Settings");
                m_displayManager.SetConfigData(displayManagerConfig);
            }

            // Attempt to load the CapturePlugin
            auto capturePluginValue = componentConfigData.TryLookup(L"CapturePlugin");
            if (capturePluginValue && capturePluginValue.ValueType() == winrt::JsonValueType::Object)
            {
                auto capturePlugin = capturePluginValue.GetObjectW();

                // Load the toolbox from the provided path.
                m_captureCard = LoadInterfaceFromPath<CaptureCard::IController>(
                    capturePlugin.GetNamedString(L"Path"),
                    capturePlugin.GetNamedString(L"Class"));

                wprintf(L"Plugin Loaded: %s\n", m_captureCard.Name().c_str());

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

                    // Load the toolbox from the provided path.
                    auto newToolbox = LoadInterfaceFromPath<ConfigurationTools::IConfigurationToolbox>(
                        toolboxConfigEntry.GetNamedString(L"Path"),
                        toolboxConfigEntry.GetNamedString(L"Class"));

                    m_toolboxes.push_back(newToolbox);

                    wprintf(L"Toolbox Opened: %s\n", m_toolboxes[0].Name().c_str());

                    auto toolboxConfig = toolboxConfigEntry.TryLookup(L"Settings");
                    newToolbox.SetConfigData(toolboxConfig);
                }

                UpdateToolList();
            }
        }

        // Parse test system information out of the config file
        {
            auto testSystemConfigDataValue = m_configFile.TryLookup(L"TestSystem");

            if (!testSystemConfigDataValue || testSystemConfigDataValue.ValueType() == winrt::JsonValueType::Null)
            {
                // No data was found.
                // TODO - log this case
                return;
            }

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

                    // Attempt to initialize the DisplayEngine with this
                    // TODO - this needs to be adjusted to handle multiple concurrent DisplayEngines for
                    //        multiple displays
                    m_displayManager.InitializeForStableMonitorId(displayId);

                    m_targetMap[pluginInputName] = displayId;
                }
            }
        }
    }

    void Core::RunTest()
    {
        // Ensure that a test can't start while a component is still being loaded.
        std::scoped_lock lock(m_testLock);
        
        winrt::hstring testName = L"";

        for (auto tool : m_toolList)
        {
            tool.Apply(m_displayManager);
            testName = testName + tool.GetConfiguration() + L"_";
        }

        // Make sure the capture card is ready
        // TODO: As with the DisplayEngine note below, this should be done on _every_ target in the map.
        auto captureInput = m_captureCard.EnumerateDisplayInputs()[0];
        captureInput.FinalizeDisplayState();

        // Start the render
        // TODO: allow a map of multiple DisplayEngines here, there is a mapping of display Engine to capture card inputs
        //       and a test run should make sure to start operations on all of them.
        auto renderer = m_displayManager.StartRender();

        // TODO: make this configurable, this is the amount of time we are waiting for display settings to 
        //       stabilize after the 'StartRender' call causes a mode change and the rendering to start
        //std::this_thread::sleep_for(std::chrono::seconds(5));

        // Capture the frame.
        auto capturedFrame = captureInput.CaptureFrame();
        auto predictedFrame = m_displayManager.GetPrediction();

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

        return m_displayManager;
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
