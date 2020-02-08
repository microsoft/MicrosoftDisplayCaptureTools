#pragma once
#include "TestEnvironment.g.h"

namespace winrt::TestFramework::implementation
{
    struct TestEnvironment : TestEnvironmentT<TestEnvironment>
    {
        TestEnvironment() = default;

        TestEnvironment(array_view<TestFramework::DisplayPath const> displayPaths);
        void Run();
        TestFramework::OSOverrides GetOSOverrides();
    };
}
namespace winrt::TestFramework::factory_implementation
{
    struct TestEnvironment : TestEnvironmentT<TestEnvironment, implementation::TestEnvironment>
    {
    };
}
