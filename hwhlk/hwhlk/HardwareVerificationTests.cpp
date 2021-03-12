#include "HardwareVerificationTests.h"

#include <Wex.Common.h>
#include <Wex.Logger.h>
#include <WexString.h>

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

bool BaseTest::BaseTestSetup()
{
    printf("Test Setup\n");
    return true;
}

bool BaseTest::BaseTestCleanup()
{
    printf("Test Cleanup\n");
    return true;
}

void BaseTest::Test()
{
    String param;
    if (SUCCEEDED(TestData::TryGetValue(L"Background", param)))
    {
        Log::Comment(L"Current test run: " + param);
    }
}