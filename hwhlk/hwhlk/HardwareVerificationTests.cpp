#include "HardwareVerificationTests.h"

#include <Wex.Common.h>
#include <Wex.Logger.h>
#include <WexString.h>

#include <string>
#include <sstream>
#include <algorithm>

#include "winrt/Windows.Foundation.h"
#include "winrt/Core.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

using namespace winrt::Core;

namespace TestFramework
{
    std::shared_ptr<IFramework> framework;
}

bool BaseTest::BaseTestSetup()
{
    winrt::init_apartment();

    //
    // Get the base runtime parameters
    //
    String pluginPath;
    if (FAILED(RuntimeParameters::TryGetValue(L"OverridePluginPath", pluginPath)))
    {
        pluginPath = L"TestPlugin\\TestPlugin.dll";
    }

    auto frameworkImpl = Framework(winrt::hstring(pluginPath));
    auto framework_test = frameworkImpl.as<IFramework>();
    TestFramework::framework = std::make_shared<IFramework>(framework_test);

    String additionalToolboxPaths;
    if (FAILED(RuntimeParameters::TryGetValue(L"AdditionalToolboxPaths", additionalToolboxPaths)))
    {
        additionalToolboxPaths = L"";
        Log::Comment(L"No additional toolboxes specified");
    }
    else
    {
        std::wstring paths(additionalToolboxPaths);

        // The runtimeparameter can specify multiple paths, comma delineated.
        size_t previousIndex = 0, currentIndex = paths.find_first_of(L",");
        while (currentIndex != std::string::npos)
        {
            std::wstring currentPath = paths.substr(previousIndex, currentIndex - previousIndex);
            if (!currentPath.empty()) TestFramework::framework->OpenToolbox(currentPath);

            previousIndex = 1 + currentIndex;
            currentIndex = paths.find_first_of(L",", previousIndex);
        }

        std::wstring finalPath = paths.substr(previousIndex, currentIndex - previousIndex);
        if (!finalPath.empty()) TestFramework::framework->OpenToolbox(finalPath);
    }

    Log::Comment(L"Initialization Complete");

    return true;
}

void BaseTest::Test()
{
    TestFramework::framework->RunPictTest();
}