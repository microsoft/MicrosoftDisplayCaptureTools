#include "pch.h"
#include "MicrosoftDisplayCaptureTools.h"
#include "Framework.Core.g.cpp"
#include "Utils.h"

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
// Constructor that uses the default logger and no runtime settings
Core::Core() : Core(winrt::make<winrt::Logger>().as<ILogger>(), nullptr) {}

// Constructor taking a caller-defined logging class and a runtime settings object (which can be null)
Core::Core(ILogger const& logger, IRuntimeSettings const& settings) : m_logger(logger), m_runtimeSettings(settings)
{
    Runtime::CreateRuntime(logger, settings);

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

    auto captureCardController = captureCardFactory.CreateController();

    m_logger.LogNote(L"Loaded Capture Plugin: " + captureCardController.Name() + L", Version " + captureCardController.Version().ToString());

    return captureCardController;
}

IController Core::LoadCapturePlugin(hstring const& pluginPath)
{
    winrt::hstring namespaceName = GetNamespaceForPlugin(pluginPath);
    return LoadCapturePlugin(pluginPath, namespaceName + c_CapturePluginDefaultName);
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

    auto toolbox = toolboxFactory.CreateConfigurationToolbox();

    m_logger.LogNote(L"Loaded Toolbox: " + toolbox.Name() + L", Version " + toolbox.Version().ToString());

    return toolbox;
}

IConfigurationToolbox Core::LoadToolbox(hstring const& toolboxPath)
{
    winrt::hstring namespaceName = GetNamespaceForPlugin(toolboxPath);
    return LoadToolbox(toolboxPath, namespaceName + c_ConfigurationToolboxDefaultName);
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
        displayEngine = displayEngineFactory.CreateDisplayEngine();
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
    winrt::hstring namespaceName = GetNamespaceForPlugin(displayEnginePath);
    return LoadDisplayEngine(displayEnginePath, namespaceName + c_DisplayEngineDefaultName);
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

    // Dump the config file to the log - this is to make debugging easier
    m_logger.LogConfig(L"Using Config File: " + jsonObject.ToString());

    // Parse component information out of the config file
    {
        auto configComponents = jsonObject.TryLookup(L"Components");

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
        auto testSystemConfigDataValue = jsonObject.TryLookup(L"TestSystem");

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

IVector<ISourceToSinkMapping> Core::GetSourceToSinkMappings(
    bool regenerateMappings,
    IDisplayEngine displayEngine,
    IConfigurationToolbox toolbox,
    IController captureCard,
    IDisplayInput displayInput)
{
    // Prevent component changes while we are attempting configuration
    auto lock = LockFramework();
    
    auto mappings = winrt::single_threaded_vector<ISourceToSinkMapping>();

    if (regenerateMappings)
    {
        // If the framework is requested to regenerate display-to-capture mappings, a display capture plugin, a display engine,
        // and a toolbox must be provided.
        if ((m_captureCards.empty() && !captureCard) || !displayEngine || !toolbox)
        {
            m_logger.LogAssert(L"Cannot generate display to capture mappings without a display engine, capture card, and toolbox.");
            return mappings;
        }

        {
            // Create a display manager
            auto manager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);

            // Get all display inputs, add to 2 unassigned lists (supports edid, doesn't support edid)
            std::list<std::tuple<winrt::IController, winrt::IDisplayInput>> unassignedInputs_EDID, unassignedInputs_NoEDID;
            std::vector<IController> captureCardsToUse;

            if (captureCard)
            {
				captureCardsToUse.push_back(captureCard);
			}
            else
            {
				captureCardsToUse = m_captureCards;
			}

            if (displayInput)
            {
                if (displayInput.GetCapabilities().CanConfigureEDID() && displayInput.GetCapabilities().CanHotPlug())
                {
                    // This input can HPD with a specified EDID
                    unassignedInputs_EDID.push_back({captureCard, displayInput});
                }
                else
                {
                    // This input cannot HPD with a specified EDID
                    unassignedInputs_NoEDID.push_back({captureCard, displayInput});
                }

                // We have already found the input we want to use, so we don't need to iterate through the rest of the inputs
                captureCardsToUse.clear();
            }

            for (auto&& card : captureCardsToUse)
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

                        if (standardEDID.SerialNumber() == retrievedEDID.SerialNumber())
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

            // For all still unassigned targets
            //    Initialize the displayengine's output and use default tool settings to generate an output/prediction.
            //    For every still unassigned inputs
            //        Pass prediction to input until one succeeds.
            if (unassignedInputs_NoEDID.size() > 0)
            {
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
                        std::this_thread::sleep_for(std::chrono::seconds(2));

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
                                auto predictedFrames = predictionAsync.get();
                                auto captureResult = capture.CompareCaptureToPrediction(L"ConfigurationPass", predictedFrames);

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
                        HRESULT hr = LOG_CAUGHT_EXCEPTION();
                        m_logger.LogNote(std::format(L"Failed to load {} with error {:#x}", file.path().stem().wstring(), hr));
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
                        HRESULT hr = LOG_CAUGHT_EXCEPTION();
                        m_logger.LogNote(std::format(L"Failed to load {} with error {:#x}", file.path().stem().wstring(), hr));
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
                        HRESULT hr = LOG_CAUGHT_EXCEPTION();
                        m_logger.LogNote(std::format(L"Failed to load {} with error {:#x}", file.path().stem().wstring(), hr));
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

winrt::hstring Core::GetNamespaceForPlugin(winrt::hstring const& pluginPath)
{
    auto defaultNamespace = winrt::hstring(std::path(pluginPath.c_str()).stem().wstring());

    // We will try to get the InternalName resource from the plugin DLL, if it exists, to use as the namespace
    try
    {
        DWORD versionInfoSize = GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, pluginPath.c_str(), nullptr);
        if (versionInfoSize == 0)
        {
            THROW_WIN32(ERROR_NOT_FOUND);
        }

        std::vector<uint8_t> versionInfo(versionInfoSize);
        THROW_IF_WIN32_BOOL_FALSE(GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, pluginPath.c_str(), 0, versionInfoSize, versionInfo.data()));

        struct LANGANDCODEPAGE
        {
            WORD wLanguage;
            WORD wCodePage;
        }* lpTranslate;

        UINT cbTranslate = 0;
        if (!VerQueryValueW(versionInfo.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
        {
            THROW_WIN32(ERROR_NOT_FOUND);
        }

        // Read the file description from the version info
        for (UINT i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
        {
            wchar_t subBlock[256];
            swprintf_s(subBlock, L"\\StringFileInfo\\%04x%04x\\InternalName", lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);

            LPVOID fileDescription;
            UINT fileDescriptionSize;
            if (VerQueryValueW(versionInfo.data(), subBlock, &fileDescription, &fileDescriptionSize))
            {
                // Remove the file extension if it has it
                return winrt::hstring(std::path(std::wstring_view(reinterpret_cast<wchar_t*>(fileDescription), fileDescriptionSize)).stem().c_str());
            }
        }

        THROW_WIN32(ERROR_NOT_FOUND);
    }
    catch (...)
    {
        LOG_CAUGHT_EXCEPTION_MSG("The plugin %ws does not have an InternalName, falling back to the filename", pluginPath.c_str());
    }

    // Simply use the file name
    return defaultNamespace;
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