#include "pch.h"
#include "Framework.h"
#include "Framework.g.cpp"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Foundation::Collections;
}

namespace winrt::TestFramework::implementation
{
    Framework::Framework(hstring const& hardwareConfigPath)
    {
        //throw hresult_not_implemented();
    }

    TestFramework::TestEnvironment Framework::CreateTestEnvironment(TestFramework::TestRequirements const& testRequirements)
    {
        auto displayManager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);
        winrt::IVectorView<winrt::DisplayTarget> targets = displayManager.GetCurrentTargets();

        if (targets.Size() < 1) throw_hresult(hresult_access_denied().code());

        auto myTargets = std::vector<winrt::DisplayTarget>();
        uint32_t targetIndex = 0;

        for (auto&& target : targets)
        {
            if (target.IsConnected())
            {
                myTargets[targetIndex++] = target;
                break;
            }
        }

        return TestFramework::TestEnvironment(myTargets);
    }
}
