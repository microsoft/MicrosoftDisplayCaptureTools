#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

#include "Loader.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId_hstring, GUID const& iid, void** factory) noexcept;

int32_t __stdcall WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept
{
    return WINRT_RoGetActivationFactory((HSTRING)classId, (GUID)iid, factory);
}

namespace winrt::Core::implementation
{
    Framework::Framework(hstring const& pluginPath)
    {
        winrt_activation_handler = WINRT_RoGetActivationFactory;

        auto binaryLoadPath = BinaryLoader::GetOrCreate();
        binaryLoadPath->SetPath(pluginPath.c_str());

        auto factory = winrt::get_activation_factory<winrt::CaptureCard::Controller>();
        auto controller = factory.ActivateInstance<winrt::CaptureCard::Controller>();
        m_captureCard = std::make_shared<winrt::CaptureCard::IController>(controller);

        Log::Comment(String().Format(L"Using Capture Device: %s", m_captureCard->Name().c_str()));

        auto displayInputs = m_captureCard->EnumerateDisplayInputs();

        for (auto input : displayInputs)
        {
            Log::Comment(String().Format(L"Discovered input: %s", input.Name().c_str()));
        }

        try
        {
            auto pluginToolbox = m_captureCard->GetToolbox();
            m_toolboxes.push_back(pluginToolbox);

            auto test = m_toolboxes.front().Name();
            Log::Comment(String().Format(L"Toolbox added: %s", pluginToolbox.Name().c_str()));
        }
        catch (winrt::hresult_not_implemented const& ex)
        {
            Log::Comment(L"Capture device does not support any tools");
        }
    }

    void Framework::OpenToolbox(hstring const& toolboxPath)
    {
        winrt_activation_handler = WINRT_RoGetActivationFactory;

        auto binaryLoadPath = BinaryLoader::GetOrCreate();
        binaryLoadPath->SetPath(toolboxPath.c_str());

        auto factory = winrt::get_activation_factory<winrt::ConfigurationTools::ConfigurationToolbox>();
        auto toolbox = factory.ActivateInstance<winrt::ConfigurationTools::ConfigurationToolbox>();

        m_toolboxes.push_back(toolbox);

        Log::Comment(String().Format(L"Toolbox added: %s", toolbox.Name().c_str()));
    }

    void Framework::RunPictTest()
    {
        boolean runSoftwareOnly = false;
        {
            // Determine if we should run in software-only mode
            String softwareOnly;
            if (SUCCEEDED(RuntimeParameters::TryGetValue(L"TestSoftwareOnly", softwareOnly)))
            {
                runSoftwareOnly = true;

                Log::Comment(L"Running in software-only mode, this is intended only for development and not full testing.");
            }
        }

        // Set up the PICT-specified tools for this test run
        auto testRun = TestRun();
        for (auto toolbox : m_toolboxes)
        {
            auto toolList = toolbox.GetSupportedTools();
            for (auto tool : toolList)
            {
                String toolSetting;
                if (SUCCEEDED(TestData::TryGetValue(tool.c_str(), toolSetting)))
                {
                    // There is a PICT setting selected for this tool, retrieve it.
                    testRun.toolRunList.push_back(toolbox.GetTool(tool));
                }
            }
        }

        // Re-order the tools by their metadata

        // Retrieve a display matching the all test requirements, from tool metadata and TAEF parameters
        std::shared_ptr<winrt::CaptureCard::DisplayInput> displayUnderTest;
        auto displayList = m_captureCard->EnumerateDisplayInputs();
        for (auto display : displayList)
        {
            auto capabilities = display.GetCapabilities();
            if (runSoftwareOnly && capabilities.returnRawFramesToHost)
            {
                displayUnderTest = std::make_shared<winrt::CaptureCard::DisplayInput>(display);
                break;
            }
        }

        // Verify that we have a display selected
        if (!displayUnderTest)
        {
            Log::Error(L"No display matching requirements could be located, aborting.");
        }

        // Run through the tool list and generate the golden image
        auto reference = winrt::make<winrt::DisplayStateReference::StaticReference>();

        // Run through the tool list and output to a display
        if (!runSoftwareOnly)
        {

        }

        // Ask the capture card to compare the outputs
        auto captureTrigger = winrt::CaptureCard::CaptureTrigger();
        captureTrigger.type = winrt::CaptureCard::CaptureTriggerType::Immediate();
        auto capture = displayUnderTest->CaptureFrame(captureTrigger);

        capture.CompareCaptureToReference(reference);
    }
}
