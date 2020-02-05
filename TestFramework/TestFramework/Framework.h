#pragma once
#include "Framework.g.h"

namespace winrt::TestFramework::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        HRESULT Initialize(hstring const& configPath);
    };
}
namespace winrt::TestFramework::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
