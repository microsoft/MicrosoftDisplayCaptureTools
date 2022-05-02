#include "pch.h"
#include "CaptureFrameworkTestBase.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt {
using namespace Windows::Foundation;
using namespace MicrosoftDisplayCaptureTools;
using namespace Windows::Devices::Display;
using namespace Windows::Devices::Display::Core;
using namespace Windows::Graphics::Imaging;
} // namespace winrt

bool CaptureFrameworkTestBase::Setup()
{
    winrt::init_apartment();
    return true;
}

bool CaptureFrameworkTestBase::Cleanup()
{
    return true;
}