#include "pch.h"
#include "TestEnvironment.h"
#include "TestEnvironment.g.cpp"

namespace winrt::TestFramework::implementation
{
    TestEnvironment::TestEnvironment(array_view<TestFramework::DisplayPath const> displayPaths)
    {
        throw hresult_not_implemented();
    }
    void TestEnvironment::Run()
    {
        throw hresult_not_implemented();
    }
    TestFramework::OSOverrides TestEnvironment::GetOSOverrides()
    {
        throw hresult_not_implemented();
    }
}
