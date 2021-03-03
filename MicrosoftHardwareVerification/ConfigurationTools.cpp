#include "pch.h"
#include "ConfigurationTools.h"
#if __has_include("ConfigurationTools.g.cpp")
#include "ConfigurationTools.g.cpp"
#endif

namespace winrt::MicrosoftHardwareVerification::implementation
{
    int32_t ConfigurationTools::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ConfigurationTools::MyProperty(int32_t /*value*/)
    {
        throw hresult_not_implemented();
    }
}
