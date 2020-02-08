#pragma once
#include "OSOverrides.g.h"

namespace winrt::TestFramework::implementation
{
    struct OSOverrides : OSOverridesT<OSOverrides>
    {
        OSOverrides() = default;

        void SetMatrix3x4(TestFramework::Matrix3x4 const& transform);
    };
}
