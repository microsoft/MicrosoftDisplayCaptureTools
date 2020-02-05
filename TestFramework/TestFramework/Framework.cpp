#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

namespace winrt::TestFramework::implementation
{
    HRESULT Framework::Initialize(hstring const& configPath)
    {
        winrt::hstring str(configPath);
        return S_OK;
    }
}
