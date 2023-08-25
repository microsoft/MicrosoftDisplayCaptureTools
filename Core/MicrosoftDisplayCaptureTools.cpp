#include "pch.h"
#include "MicrosoftDisplayCaptureTools.h"
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
    m_logger.LogNote(L"Initializing MicrosoftDisplayCaptureTools v" + this->Version().ToString());
}

// Constructor taking a caller-defined logging class
Core::Core(ILogger const& logger) : m_logger(logger)
{
    m_logger.LogNote(L"Initializing MicrosoftDisplayCaptureTools v" + this->Version().ToString());
}

IController Core::LoadCapturePlugin(hstring const& pluginPath, hstring const& className)
{
    // Ensure that a component can't be changed if a test has locked the framework
    if (IsFrameworkLocked())
    {
        m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
        throw winrt::hresult_illegal_method_call();
    }

    // Load the capture card from the provided path.
    auto captureCardFactory = LoadInterfaceFromPath<IControllerFactory>(pluginPath, className);

    auto captureCardController = captureCardFactory.CreateController(m_logger);

    m_logger.LogNote(L"Loaded Capture Plugin: " + captureCardController.Name() + L", Version " + captureCardController.Version().ToString());

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
    // Ensure that a component can't be changed if anything has locked the framework
    if (IsFrameworkLocked())
    {
        m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
        throw winrt::hresult_illegal_method_call();
    }

    // Load the toolbox from the provided path.
    auto toolboxFactory = LoadInterfaceFromPath<IConfigurationToolboxFactory>(toolboxPath, className);

    auto toolbox = toolboxFactory.CreateConfigurationToolbox(m_logger);

    m_logger.LogNote(L"Loaded Toolbox: " + toolbox.Name() + L", Version " + toolbox.Version().ToString());

    return toolbox;
}

IConfigurationToolbox Core::LoadToolbox(hstring const& toolboxPath)
{
    // Create the className string from
    winrt::hstring className = std::path(toolboxPath.c_str()).stem().c_str();
    return LoadToolbox(toolboxPath, className + c_ConfigurationToolboxDefaultName);
}

IDisplayEngine Core::LoadDisplayEngine(hstring const& displayEnginePath, hstring const& className)
{
    // Ensure that a component can't be changed if a test has locked the framework
    if (IsFrameworkLocked())
    {
        m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
        throw winrt::hresult_illegal_method_call();
    }

    // Load the toolbox from the provided path.
    auto displayEngineFactory = LoadInterfaceFromPath<IDisplayEngineFactory>(displayEnginePath, className);

    IDisplayEngine displayEngine = nullptr;
    
    try
    {
        displayEngine = displayEngineFactory.CreateDisplayEngine(m_logger);
    }
    catch (...)
    {
        m_logger.LogWarning(L"Failed to load DisplayEngine: " + className + L" from " + displayEnginePath);
    }

    if (displayEngine)
    {
        m_logger.LogNote(L"Loaded DisplayEngine: " + displayEngine.Name() + L", Version " + displayEngine.Version().ToString());
    }

    return displayEngine;
}

IDisplayEngine Core::LoadDisplayEngine(hstring const& displayEnginePath)
{
    // Create the className string from
    winrt::hstring className = std::path(displayEnginePath.c_str()).stem().c_str();
    return LoadDisplayEngine(displayEnginePath, className + c_DisplayEngineDefaultName);
}

void Core::LoadConfigFile(hstring const& configFile)
{
    m_logger.LogNote(L"Loading configuration from file...");

    // Ensure that a component can't be changed if a test has locked the framework
    if (IsFrameworkLocked())
    {
        m_logger.LogAssert(L"Attemped to modify framework configuration while test locked!");
        return;
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
        return;
    }

    m_configFile = jsonObject;

    // Dump the config file to the log - this is to make debugging easier
    m_logger.LogConfig(L"Using Config File: " + m_configFile.ToString());

    // Parse component information out of the config file
    {
        auto configComponents = m_configFile.TryLookup(L"Components");

        if (!configComponents || configComponents.ValueType() == winrt::JsonValueType::Null)
        {
            // No data was found.
            m_logger.LogWarning(L"Configuration doesn't specify components to load.");
        }
        else
        {
            auto componentConfigData = configComponents.GetObjectW();

            // Attempt to load the DisplayEngine
            auto configDisplayEngineValue = componentConfigData.TryLookup(L"DisplayEngine");
            if (configDisplayEngineValue && configDisplayEngineValue.ValueType() == winrt::JsonValueType::Object)
            {
                auto configDisplayEngine = configDisplayEngineValue.GetObjectW();

                auto displayEngine = LoadDisplayEngine(configDisplayEngine.GetNamedString(L"Path"), configDisplayEngine.GetNamedString(L"Class"));
                
                if (displayEngine)
                {
                    m_displayEngines.push_back(displayEngine);

                    if (configDisplayEngine.HasKey(L"Settings"))
                    {
                        auto configDisplayEngineSettings = configDisplayEngine.TryLookup(L"Settings");
                        displayEngine.SetConfigData(configDisplayEngineSettings);
                    }
                }
            }

            // Attempt to load the CapturePlugin
            auto configCapturePluginValue = componentConfigData.TryLookup(L"CapturePlugin");
            if (configCapturePluginValue && configCapturePluginValue.ValueType() == winrt::JsonValueType::Object)
            {
                auto configCapturePlugin = configCapturePluginValue.GetObjectW();

                auto captureCard = LoadCapturePlugin(configCapturePlugin.GetNamedString(L"Path"), configCapturePlugin.GetNamedString(L"Class"));

                if (captureCard)
                {
                    m_captureCards.push_back(captureCard);

                    if (configCapturePlugin.HasKey(L"Settings"))
                    {
                        auto configCapturePluginSettings = configCapturePlugin.TryLookup(L"Settings");
                        captureCard.SetConfigData(configCapturePluginSettings);
                    }
                }
            }

            // Attempt to load any ConfigurationToolboxes, if applicable
            auto configToolboxesValue = componentConfigData.TryLookup(L"ConfigurationToolboxes");
            if (configToolboxesValue && configToolboxesValue.ValueType() == winrt::JsonValueType::Array)
            {
                auto configToolboxes = configToolboxesValue.GetArray();

                for (auto configToolboxValue : configToolboxes)
                {
                    auto configToolbox = configToolboxValue.GetObjectW();

                    auto toolbox = LoadToolbox(configToolbox.GetNamedString(L"Path"), configToolbox.GetNamedString(L"Class"));
                    
                    if (toolbox)
                    {
                        m_toolboxes.push_back(toolbox);

                        if (configToolbox.HasKey(L"Settings"))
                        {
                            auto configToolboxSettings = configToolbox.TryLookup(L"Settings");
                            toolbox.SetConfigData(configToolboxSettings);
                        }
                    }
                }
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
            if (m_captureCards.empty() || m_displayEngines.empty())
            {
                m_logger.LogAssert(L"Display mappings specified in config file while missing display or capture card plugins.");
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
                                    foundInput.FinalizeDisplayState();
                                }
                            }
                        }
                    }

                    if (!foundInput)
                    {
                        m_logger.LogError(L"Display input mapped from config file not found.");
                    }

                    // From the stable monitor ID parsed from the config file, find the corresponding DisplayTarget. Note that any complete 
                    // DisplayEngine implementation needs to be able to do this, so it's safe to just pick one from the list of those loaded,
                    // this function would have already errored out if none were available.
                    auto displaySource = m_displayEngines[0].InitializeOutput(displayId).Target();

                    m_displayMappingsFromFile.push_back(winrt::make<SourceToSinkMapping>(displaySource, foundInput));
                }
            }
        }
    }
}

com_array<IConfigurationToolbox> Core::GetConfigurationToolboxes()
{
    return com_array<IConfigurationToolbox>(m_toolboxes);
}

com_array<IController> Core::GetCaptureCards()
{
    return com_array<IController>(m_captureCards);
}

com_array<IDisplayEngine> Core::GetDisplayEngines()
{
    return com_array<IDisplayEngine>(m_displayEngines);
}

IVector<ISourceToSinkMapping> Core::GetSourceToSinkMappings(bool regenerateMappings, IDisplayEngine displayEngine, IConfigurationToolbox toolbox)
{
    // Prevent component changes while we are attempting configuration
    auto lock = LockFramework();
    
    auto mappings = winrt::single_threaded_vector<ISourceToSinkMapping>();

    if (regenerateMappings)
    {
        // If the framework is requested to regenerate display-to-capture mappings, a display capture plugin and a display engine
        // are both required.
        if (m_captureCards.empty() || !displayEngine)
        {
            m_logger.LogAssert(L"Cannot generate display to capture mappings without a display engine and a capture card.");
            return mappings;
        }

        {
            // Create a display manager
            auto manager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);

            // Get all display inputs, add to 2 unassigned lists (supports edid, doesn't support edid)
            std::list<std::tuple<winrt::IController, winrt::IDisplayInput>> unassignedInputs_EDID, unassignedInputs_NoEDID;
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
                // For mappings we don't want to fail the setup if we fail to map.
                auto suppressErrors = m_logger.LogErrorsAsWarnings();

                // Create a standard EDID, and give it a specific serial number
                auto standardEDID = EDIDDescriptor::CreateStandardEDID();
                standardEDID.SerialNumber(++serialNum);

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
                            auto pluginName = card.Name();
                            auto pluginInputName = input.Name();
                            auto displayId = target.StableMonitorId();

                            m_logger.LogConfig(hstring(L"Display: ") + displayId + L" connected to " + pluginName + L"." + pluginInputName);

                            // We found the match, add it to the mappings with this input
                            auto mapping = winrt::make<SourceToSinkMapping>(target, input);
                            mappings.Append(mapping);

                            // Also note that this target has already been mapped
                            mappedTargets.push_back(target);
                            break;
                        }
                    }
                }

                serialNum++;
            }

            // If a capture card input cannot hotplug in a display with a custom descriptor, the next mechanism is to take control of each 
            // display and setup a scanout and prediction - similar to how a basic test works, just with all default options. 
            // 
            // NOTE: We will not remove any displays from composition for this - to prevent a user accidentally making their system unusable.
            //       Instead for this type of capture card we will only consider displays already marked as 'specialized'. The user may have to
            //       manually mark the applicable display as specialized in display settings, or specify the target in the config file for this
            //       device. For the latter case, this auto-config method will print out the possible display IDs.
            auto toolList = GetAllTools(toolbox);
            if (toolList.empty())
            {
                m_logger.LogWarning(L"No tools have been loaded - it is impossible to automatically determine display output "
                                    L"- capture input mapping.");
            }

            m_logger.LogNote(L"Using default values for all tools.");

            // For all still unassigned targets
            //    Initialize the displayengine's output and use default tool settings to generate an output/prediction.
            //    For every still unassigned inputs
            //        Pass prediction to input until one succeeds.
            if (unassignedInputs_NoEDID.size() > 0)
            {
                auto targets = manager.GetCurrentTargets();
                for (auto&& target : targets)
                {
                    // For this type of capture card - targets should appear to Windows as regular displays and would be composed
                    // normally. This means we can filter to only 'connected' targets.

                    if (!target.IsConnected())
                    {
                        continue;
                    }

                    auto monitor = target.TryGetMonitor();

                    if (!monitor)
                    {
                        continue;
                    }

                    // We don't take control of displays for this config - there is too high a risk of accidental black screens
                    if (monitor.UsageKind() == DisplayMonitorUsageKind::Standard)
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

                    try
                    {
                        // We have a target which has not yet been mapped - take control of it and see if any capture input
                        // matches it, using default settings for the tools.
                        auto output = displayEngine.InitializeOutput(target);
                        auto prediction = toolbox.CreatePrediction();
                        for (auto&& tool : toolList)
                        {
                            tool.SetConfiguration(tool.GetDefaultConfiguration());
                            tool.ApplyToOutput(output);
                            tool.ApplyToPrediction(prediction);
                        }

                        // Start generating the prediction in the background
                        auto predictionAsync = prediction.FinalizePredictionAsync();

                        // Start outputting to the target with the current settings
                        auto renderer = output.StartRender();
                        std::this_thread::sleep_for(std::chrono::seconds(5));

                        // Iterate through the still unassigned inputs to find any matches
                        for (auto&& unassignedInput : unassignedInputs_NoEDID)
                        {
                            auto [card, input] = unassignedInput;

                            auto suppressErrors = m_logger.LogErrorsAsWarnings();

                            input.FinalizeDisplayState();
                            auto capture = input.CaptureFrame();
                            if (capture)
                            {
                                m_logger.LogNote(
                                    winrt::hstring(L"Comparing output of ") + monitor.DisplayName() + L" to input " +
                                    card.Name() + L"." + input.Name());

                                // Make sure that we finished generating the prediction
                                auto prediction = predictionAsync.get();
                                auto captureResult = capture.CompareCaptureToPrediction(L"ConfigurationPass", prediction.FrameSet());

                                if (captureResult && !suppressErrors.HasErrored())
                                {
                                    // We found the match, add it to the mappings with this input
                                    auto mapping = winrt::make<SourceToSinkMapping>(target, input);
                                    mappings.Append(mapping);

                                    m_logger.LogNote(
                                        L"Successfully matched output " + monitor.DisplayName() + L" to input " + card.Name() +
                                        L"." + input.Name());

                                    unassignedInputs_NoEDID.remove(unassignedInput);

                                    break;
                                }
                            }
                        }

                        std::this_thread::sleep_for(std::chrono::seconds(5));
                    }
                    catch (...)
                    {
                        // If something fails trying to set up this output as a target - just try others... failure here 
                        // almost assuredly means that this is not a valid test target.
                        continue;
                    }
                }
            }

            if (mappings.Size() == 0)
            {
                m_logger.LogWarning(L"Unable to match any inputs to outputs, please ensure that the proper outputs are removed "
                                    L"from the desktop in advanced display settings.");
            }
        }
    }
    else
    {
        // RegenerateMappings is set to false, so just use any already set mappings (from config file)
        for (auto&& entry : m_displayMappingsFromFile)
        {
            mappings.Append(entry);
        }
    }

    return mappings;
}

// Go through and discover all plugins installed for this framework on this system
void Core::DiscoverInstalledPlugins()
{
    wchar_t filepath[MAX_PATH];
    auto coreHandle = GetModuleHandle(c_CoreFrameworkName.c_str());
    auto returnLength = GetModuleFileNameW(coreHandle, filepath, MAX_PATH);

    if (returnLength == MAX_PATH)
    {
        m_logger.LogAssert(L"Failed to locate the filepaths for the framework - please reinstall packages.");
        return;
    }

    // Get all plugin directories
    std::filesystem::path path(filepath);
    auto installedDirectory = path.parent_path();
    auto capturePluginDirectory        = installedDirectory / c_CapturePluginDirectory;
    auto configurationToolboxDirectory = installedDirectory / c_ConfigurationToolboxDirectory;
    auto displayEngineDirectory        = installedDirectory / c_DisplayEngineDirectory;

    // Search for capture cards
    if (m_captureCards.empty())
    {
        try
        {
            for (auto const& file : std::filesystem::directory_iterator{capturePluginDirectory})
            {
                if (file.path().extension() == L".dll")
                {
                    m_logger.LogNote(L"Discovered Capture Plugin: " + file.path().stem());

                    try
                    {
                        if (auto capturePlugin = LoadCapturePlugin(file.path().c_str()))
                        {
                            m_logger.LogNote(L"Loaded " + file.path().stem());
                            m_captureCards.push_back(capturePlugin);
                        }
                    }
                    catch (...)
                    {
                        m_logger.LogNote(L"Failed to load " + file.path().stem());
                    }
                }
            }
        }
        catch (...)
        {
            m_logger.LogNote(L"Unable to load capture plugin directory.");
        }
    }
    else
    {
        m_logger.LogNote(L"Using pre-loaded capture plugins, skipping auto-discovery.");
    }

    // Search for toolboxes
    if (m_toolboxes.empty())
    {
        try
        {
            for (auto const& file : std::filesystem::directory_iterator{configurationToolboxDirectory})
            {
                if (file.path().extension() == L".dll")
                {
                    m_logger.LogNote(L"Discovered Toolbox: " + file.path().stem());

                    try
                    {
                        if (auto toolbox = LoadToolbox(file.path().c_str()))
                        {
                            m_logger.LogNote(L"Loaded " + file.path().stem());
                            m_toolboxes.push_back(toolbox);
                        }
                    }
                    catch (...)
                    {
                        m_logger.LogNote(L"Failed to load " + file.path().stem());
                    }
                }
            }
        }
        catch (...)
        {
            m_logger.LogNote(L"Unable to load configuration toolbox directory.");
        }
    }
    else
    {
        m_logger.LogNote(L"Using pre-loaded toolboxes, skipping auto-discovery.");
    }

    // Search for displayEngines
    if (m_displayEngines.empty())
    {
        try
        {
            for (auto const& file : std::filesystem::directory_iterator{displayEngineDirectory})
            {
                if (file.path().extension() == L".dll")
                {
                    m_logger.LogNote(L"Discovered Display Engine: " + file.path().stem());

                    try
                    {
                        if (auto displayEngine = LoadDisplayEngine(file.path().c_str()))
                        {
                            m_logger.LogNote(L"Loaded " + file.path().stem());
                            m_displayEngines.push_back(displayEngine);
                        }
                    }
                    catch (...)
                    {
                        m_logger.LogNote(L"Failed to load " + file.path().stem());
                    }
                }
            }
        }
        catch (...)
        {
            m_logger.LogNote(L"Unable to load display engine directory.");
        }
    }
    else
    {
        m_logger.LogNote(L"Using pre-loaded DisplayEngines, skipping auto-discovery.");
    }
}

std::vector<ConfigurationTools::IConfigurationTool> Core::GetAllTools(IConfigurationToolbox specificToolbox = nullptr)
{
    std::vector<IConfigurationTool> tools;

    if (specificToolbox)
    {
        for (auto&& toolName : specificToolbox.GetSupportedTools())
        {
            tools.push_back(specificToolbox.GetTool(toolName));
        }

        return tools;
    }

    for (auto&& toolbox : m_toolboxes)
    {
        for (auto&& toolName : toolbox.GetSupportedTools())
        {
            tools.push_back(toolbox.GetTool(toolName));
        }
    }

    return tools;
}

winrt::Windows::Foundation::IClosable Core::LockFramework()
{
    return winrt::make<TestLock>(&m_lockCount);
}

SourceToSinkMapping::SourceToSinkMapping(winrt::DisplayTarget const& source, IDisplayInput const& sink) :
    m_source(source), m_sink(sink)
{
}

SourceToSinkMapping::~SourceToSinkMapping()
{
}

IDisplayInput SourceToSinkMapping::Sink()
{
    return m_sink;
}

winrt::DisplayTarget SourceToSinkMapping::Source()
{
    return m_source;
}

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation