#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

namespace winrt::TestFramework::implementation
{
    int32_t Framework::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void Framework::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
