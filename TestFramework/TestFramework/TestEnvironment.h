#pragma once
#include "TestEnvironment.g.h"

namespace winrt::TestFramework::implementation
{
    struct TestEnvironment : TestEnvironmentT<TestEnvironment>
    {
        TestEnvironment() = default;

        TestEnvironment(array_view<Windows::Devices::Display::Core::DisplayTarget const> displayTargets);
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
