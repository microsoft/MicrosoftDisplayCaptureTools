#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

namespace winrt::TestFramework::implementation
{
    Framework::Framework(hstring const& hardwareConfigPath)
    {
        throw hresult_not_implemented();
    }
    TestFramework::TestEnvironment Framework::CreateTestEnvironment(TestFramework::TestRequirements const& testRequirements)
    {
        throw hresult_not_implemented();
    }
}
