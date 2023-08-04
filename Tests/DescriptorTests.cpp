#include "pch.h"
#include "DescriptorTests.h"

bool DescriptorTests::Setup()
{
    return __super::Setup();
}

bool DescriptorTests::Cleanup()
{
    return __super::Cleanup();
}

void DescriptorTests::EdidManyBlocks()
{
    if (g_predictionOnly)
    {
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Blocked, "Cannot do descriptor tests in prediction-only mode.");
        return;
    }
}

void DescriptorTests::DisplayId2InEdid()
{
    if (g_predictionOnly)
    {
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Blocked, "Cannot do descriptor tests in prediction-only mode.");
        return;
    }
}

