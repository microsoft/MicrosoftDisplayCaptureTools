#pragma once
#include "Framework.g.h"

namespace winrt::TestFramework::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        Framework(hstring const& hardwareConfigPath);
        TestFramework::TestEnvironment CreateTestEnvironment(TestFramework::TestRequirements const& testRequirements);
    };
}
namespace winrt::TestFramework::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
