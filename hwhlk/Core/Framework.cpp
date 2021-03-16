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

        Log::Comment(String().Format(L"Using Capture Device: %s", controller.Name().c_str()));

        auto displayInputs = controller.EnumerateDisplayInputs();

        for (auto input : displayInputs)
        {
            Log::Comment(String().Format(L"Discovered input: %s", input.Name().c_str()));
        }

        try
        {
            auto pluginToolbox = controller.GetToolbox();
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
        throw hresult_not_implemented();
    }
}
