#include "pch.h"
#include "DescriptorTests.h"

using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
using namespace MicrosoftDisplayCaptureTools::Tests;

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
    if (Helpers::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Blocked, "Cannot do descriptor tests in prediction-only mode.");
        return;
    }
}

void DescriptorTests::DisplayId2InEdid()
{
    if (Helpers::RuntimeSettings().GetSettingValueAsBool(RunPredictionOnlyRuntimeParameter))
    {
        WEX::Logging::Log::Result(WEX::Logging::TestResults::Blocked, "Cannot do descriptor tests in prediction-only mode.");
        return;
    }
}

