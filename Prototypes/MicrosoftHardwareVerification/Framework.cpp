#include "pch.h"
#include "Framework.h"
#if __has_include("Framework.g.cpp")
#include "Framework.g.cpp"
#endif

namespace winrt::MicrosoftHardwareVerification::implementation
{
    Framework::Framework(hstring const& pluginPath)
    {
        throw hresult_not_implemented();
    }
    hstring Framework::Name()
    {
        throw hresult_not_implemented();
    }
    void Framework::Name(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    void Framework::OpenToolbox(hstring const& toolboxPath)
    {
        throw hresult_not_implemented();
    }
    void Framework::RunPictTest()
    {
        throw hresult_not_implemented();
    }
}
