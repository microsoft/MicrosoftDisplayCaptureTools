#include "pch.h"
#include "Framework.h"
#include "Core.Framework.g.cpp"

#include "Loader.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;

HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId_hstring, GUID const& iid, void** factory) noexcept;

int32_t __stdcall WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept
{
    return WINRT_RoGetActivationFactory((HSTRING)classId, (GUID)iid, factory);
}

namespace winrt::MicrosoftDisplayCaptureTools::Core::implementation
{
    Framework::Framework(hstring const& pluginPath)
    {
        winrt_activation_handler = WINRT_RoGetActivationFactory;

        m_captureCard = CreateImplementationFromPlugin<winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController>(pluginPath, L"CaptureCard.Controller");

        Log::Comment(String().Format(L"Using Capture Device: %s", m_captureCard.Name().c_str()));

        auto displayInputs = m_captureCard.EnumerateDisplayInputs();

        for (auto input : displayInputs)
        {
            Log::Comment(String().Format(L"Discovered input: %s", input.Name().c_str()));
        }

        try
        {
            auto pluginToolbox = m_captureCard.GetToolbox();
            m_toolboxes.push_back(pluginToolbox);

            auto test = m_toolboxes.front().Name();
            Log::Comment(String().Format(L"Toolbox added: %s", pluginToolbox.Name().c_str()));
        }
        catch (winrt::hresult_not_implemented const&)
        {
            Log::Comment(L"Capture device does not support any tools");
        }

        m_runSoftwareOnly = false;
        {
            // Determine if we should run in software-only mode
            String softwareOnly;
            if (SUCCEEDED(RuntimeParameters::TryGetValue(L"TestSoftwareOnly", softwareOnly)) && softwareOnly == L"true")
            {
                m_runSoftwareOnly = true;
                Log::Comment(L"Running in software-only mode, this is intended only for development and not full testing.");
            }
        }
    }

    void Framework::OpenToolbox(hstring const& toolboxPath)
    {
        winrt_activation_handler = WINRT_RoGetActivationFactory;

        auto binaryLoadPath = BinaryLoader::SetPathInScope(toolboxPath);
        auto toolbox = CreateImplementationFromPlugin<winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox>(toolboxPath, L"ConfigurationToolbox");

        m_toolboxes.push_back(toolbox);

        Log::Comment(String().Format(L"Toolbox added: %s", toolbox.Name().c_str()));
    }

    void Framework::RunPictTest()
    {
        // Set up the PICT-specified tools for this test run
        auto testRun = TestRun();
        auto testName = hstring(L"");
        for (auto toolbox : m_toolboxes)
        {
            auto toolList = toolbox.GetSupportedTools();
            for (auto tool : toolList)
            {
                String toolSetting;
                if (SUCCEEDED(TestData::TryGetValue(tool.c_str(), toolSetting)))
                {
                    // There is a PICT setting selected for this tool, retrieve it.
                    auto toolToRun = toolbox.GetTool(tool);
                    toolToRun.SetConfiguration(hstring(toolSetting));
                    testRun.toolRunList.push_back(toolToRun);

                    // Build the name string for this test run
                    if (!testName.empty())
                    {
                        testName = testName + L", ";
                    }

                    testName = testName + tool + L": " + hstring(toolSetting);
                }
            }
        }

        // Re-order the tools by their metadata
        for (auto tool : testRun.toolRunList)
        {
            testRun.toolOrderedRunList.push_back(tool);
        }

        // Sort list by category
        std::sort(testRun.toolOrderedRunList.begin(), testRun.toolOrderedRunList.end(), testRun);

        // Re-iterate through the tools and ensure that requirements are met
        for (auto tool : testRun.toolOrderedRunList)
        {
            auto reqs = tool.Requirements();

            // TODO: add intra-tool requirement support to Requirements struct
        }

        // Retrieve a display matching the all test requirements, from tool metadata and TAEF parameters
        auto displayUnderTest = ChooseDisplay();

        // Run through the tool list and generate the golden image
        auto reference = winrt::make<winrt::MicrosoftDisplayCaptureTools::DisplayStateReference::implementation::StaticReferenceData>(L"Base Plane");

        for (auto tool : testRun.toolOrderedRunList)
        {
            tool.ApplyToReference(reference);
        }

        // Run through the tool list and output to a display
        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory currentCategory = testRun.toolOrderedRunList.front().Category();

        for (auto tool : testRun.toolOrderedRunList)
        {
            // We're switching over to a new category of tools, if we're finishing the setup tools, inform the plugin so that
            // it can ensure all setup data is finalized (this may require lengthly operations like an HPD).
            if (currentCategory == winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory::DisplaySetup && tool.Category() != currentCategory)
            {
                displayUnderTest.FinalizeDisplayState();
            }

            // Apply to hardware
            if (!m_runSoftwareOnly)
            {
                //auto target = displayUnderTest.MapCaptureInputToDisplayPath();
                //tool.ApplyToHardware(target);
            }
        }

        // Ask the capture card to compare the outputs
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTrigger captureTrigger{};
        captureTrigger.type = winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType::Immediate;
        auto capture = displayUnderTest.CaptureFrame(captureTrigger);

        capture.CompareCaptureToReference(testName, reference);
    }

    winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput Framework::ChooseDisplay()
    {
        auto displayList = m_captureCard.EnumerateDisplayInputs();
        for (auto display : displayList)
        {
            auto capabilities = display.GetCapabilities();
            if (m_runSoftwareOnly && capabilities.returnRawFramesToHost)
            {
                return display;
            }

            // TODO: Right now this isn't implemented for a physical card, prioritizing the virtual card we can share with Intel
        }

        Log::Error(L"No display matching requirements could be located, aborting.");

        throw winrt::hresult_error();
    }

    bool TestRun::operator()(winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool a, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool b) const
    {
        // TODO: This will become significantly more complicated as we continue to add more and more tools
        return a.Category() < b.Category();
    }
}
