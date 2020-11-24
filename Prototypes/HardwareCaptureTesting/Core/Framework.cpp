#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

namespace winrt::HardwareCaptureTesting::Core::implementation
{
    Framework::Framework(hstring const& pluginPath, hstring const& toolboxPath)
    {
        throw hresult_not_implemented();
    }
    hstring Framework::SetOutputDirectory()
    {
        throw hresult_not_implemented();
    }
    void Framework::SetOutputDirectory(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    hstring Framework::SetComparisonDirectory()
    {
        throw hresult_not_implemented();
    }
    void Framework::SetComparisonDirectory(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    HardwareCaptureTesting::CaptureCard::Plugin Framework::GetPlugin()
    {
        throw hresult_not_implemented();
    }
    void Framework::GetDisplaysUnderTest(Windows::Foundation::Collections::IMap<Windows::Devices::Display::Core::DisplayTarget, uint32_t>& displays)
    {
        
        throw hresult_not_implemented();
    }
    void Framework::StartManualTest(hstring const& testName)
    {
        throw hresult_not_implemented();
    }
    void Framework::StartCombinatorialTest()
    {
        throw hresult_not_implemented();
    }
    void Framework::AddToolbox(hstring const& toolboxPath)
    {
        throw hresult_not_implemented();
    }
    void Framework::UseTool(hstring const& toolName, hstring const& toolParams)
    {
        throw hresult_not_implemented();
    }
    void Framework::EvaluateCapture()
    {
        throw hresult_not_implemented();
    }
}
