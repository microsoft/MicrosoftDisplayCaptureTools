#pragma once

#include "Framework.g.h"

namespace winrt::TestFramework::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::TestFramework::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
