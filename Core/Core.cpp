﻿#include "pch.h"
#include "Core.h"
#include "Framework.Core.g.cpp"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Data::Json;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Graphics;
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

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
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

                    m_displayMappingsFromFile[foundInput] = m_displayEngine.InitializeOutput(displayId);
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

    if (regenerateMappings)
    {
        // If the framework is requested to regenerate display-to-capture mappings, a display capture plugin and a display engine
        // are both required.
        if (m_captureCards.empty() || !m_displayEngine)
        {
            m_logger.LogAssert(L"Cannot generate display to capture mappings without a display engine and a capture card.");
            throw winrt::hresult_illegal_method_call();
        }

        {
            // Prevent component changes while we are attempting auto configuration
            // TODO: this should be fixed and changed to a shared_lock system... right now a previous lock could be released halfway through.
            IClosable lock = nullptr;
            if (!m_isLocked)
            {
                lock = LockFramework();
            }

            // Create a display manager
            auto manager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);

            // Get all display inputs, add to 2 unassigned lists (supports edid, doesn't support edid)
            std::vector<std::tuple<winrt::IController, winrt::IDisplayInput>> unassignedInputs_EDID, unassignedInputs_NoEDID;
            for (auto&& card : m_captureCards)
            {
                for (auto&& input : card.EnumerateDisplayInputs())
                {
                    if (input.GetCapabilities().CanConfigureEDID() && input.GetCapabilities().CanHotPlug())
                    {
                        // This input can HPD with a specified EDID
                        unassignedInputs_EDID.push_back({card, input});
                    }
                    else
                    {
                        // This input cannot HPD with a specified EDID
                        unassignedInputs_NoEDID.push_back({card, input});
                    }
                }
            }

            std::vector<winrt::DisplayTarget> mappedTargets;

            // For all edid inputs
            //    HPD custom EDID, wait, iterate through unassigned targets for descriptor matches
            uint32_t serialNum = 0xFFFFAAAA;
            for (auto [card, input] : unassignedInputs_EDID)
            {
                // Create a standard EDID, and give it a specific serial number
                auto standardEDID = EDIDDescriptor::CreateStandardEDID();
                standardEDID.SerialNumber(serialNum);

                // Set the EDID to the capture device plugin and instruct it to hotplug with it.
                input.SetDescriptor(standardEDID);
                input.FinalizeDisplayState();

                // Enumerate all new targets - the previous FinalizeDisplayState call should only return once the display is visible to windows. So this call
                // should include the newly plugged-in display.
                auto newtargets = manager.GetCurrentTargets();

                for (auto&& target : newtargets)
                {
                    auto monitor = target.TryGetMonitor();

                    if (monitor)
                    {
                        // Try to get the EDID from this monitor - the display we are looking for will have the same EDID we plugged in earlier
                        auto retrievedEDID = winrt::make<EDIDDescriptor>(monitor.GetDescriptor(DisplayMonitorDescriptorKind::Edid));

                        if (standardEDID.IsSame(retrievedEDID))
                        {
                            {
                                auto pluginName = input.Name();
                                auto pluginInputName = card.Name();
                                auto displayId = target.StableMonitorId();

                                m_logger.LogConfig(hstring(L"Display: ") + displayId + L" connected to " + pluginName + L"." + pluginInputName);
                            }

                            // We found the match, add it to the mappings with this input
                            auto mapping = winrt::make<SourceToSinkMapping>(input, m_displayEngine.InitializeOutput(target));
                            mappings.Append(mapping);

                            // Also note that this target has already been mapped
                            mappedTargets.push_back(target);
                            break;
                        }
                    }
                }

                serialNum++;
            }


            // Similar to how a basic test works - use basic tools to set up output and prediction and then just go through until we
            // find a match. If we don't have access to the basic tools from the test pass (resolution, refresh rate, basic pattern,
            // etc. - then just dump the source/sink IDs to the logs so that the user can construct an appropriate config file.

            // TODO: print out whatever EDID-based mappings we have already found
            // TODO: print out all displays and all capture inputs.

            // TODO: print out that we are trying last ditch effort to locate the remaining displays

            bool printRemainingTargets = false;
            if (m_toolList.empty())
            {
                m_logger.LogWarning(L"No tools have been loaded - it may be impossible to automatically determine display output "
                                    L"- capture input mapping.");

                printRemainingTargets = true;
            }

            m_logger.LogNote(L"Using default values for all tools.");

            // For all still unassigned targets
            //    Initialize the displayengine's output and use default tool settings to generate an output/prediction.
            //    For every still unassigned inputs
            //        Pass prediction to input until one succeeds.
            auto targets = manager.GetCurrentTargets();
            for (auto&& target : targets) 
            {
                // For this type of capture card - targets should appear to Windows as regular displays and would be composed normally.
                // This means we can filter to only 'connected' targets.
                if (!target.IsConnected())
                {
                    continue;
                }

                // Second, make sure that we aren't bothering with already mapped targets.
                bool alreadyMapped = false;
                for (auto&& mappedTarget : mappedTargets)
                {
                    if (target.IsSame(mappedTarget))
                    {
                        alreadyMapped = true;
                        break;
                    }
                }
                if (alreadyMapped)
                {
                    continue;
                }

                // We have a target which has not yet been mapped - take control of it and see if any capture input matches it with
                // default Tool settings.
                auto output = m_displayEngine.InitializeOutput(target);
                for (auto&& tool : m_toolList)
                {
                    tool.SetConfiguration(tool.GetDefaultConfiguration());
                    tool.Apply(output);
                }

                // Get the predicted frame
                auto prediction = output.GetPrediction();
                
                // Start outputting to the target with the current settings
                auto renderer = output.StartRender();
                std::this_thread::sleep_for(std::chrono::seconds(1));

                // Iterate through the still unassigned inputs to find any matches
                for (auto [card, input] : unassignedInputs_NoEDID)
                {
                    input.FinalizeDisplayState();
                    auto capture = input.CaptureFrame();

                    // TODO: run the comparison of frame against prediction.
                }
            }
        }
    }
    else
    {
        // TODO: delete existing mapping and regenerate them if regenerateMappings
        for (auto&& entry : m_displayMappingsFromFile)
        {
            mappings.Append(winrt::make<SourceToSinkMapping>(entry.first, entry.second));
        }
    }

    return mappings;
}

void Core::UpdateToolList()
{
    if (m_toolboxes.empty())
        return;

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

IDisplayInput SourceToSinkMapping::Sink()
{
    return m_sink;
}

IDisplayOutput SourceToSinkMapping::Source()
{
    return m_source;
}

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation