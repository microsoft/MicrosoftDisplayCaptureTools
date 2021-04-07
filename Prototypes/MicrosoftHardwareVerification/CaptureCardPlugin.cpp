#include "pch.h"
#include "CaptureCardPlugin.h"
#if __has_include("CaptureCardPlugin.g.cpp")
#include "CaptureCardPlugin.g.cpp"
#endif

namespace winrt::MicrosoftHardwareVerification::implementation
{
    int32_t CaptureCardPlugin::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void CaptureCardPlugin::MyProperty(int32_t /*value*/)
    {
        throw hresult_not_implemented();
    }
}
