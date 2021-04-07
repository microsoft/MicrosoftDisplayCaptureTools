#include "pch.h"
#include "OSOverrides.h"
#include "OSOverrides.g.cpp"

namespace winrt::TestFramework::implementation
{
    void OSOverrides::SetMatrix3x4(TestFramework::Matrix3x4 const& transform)
    {
        throw hresult_not_implemented();
    }
}
