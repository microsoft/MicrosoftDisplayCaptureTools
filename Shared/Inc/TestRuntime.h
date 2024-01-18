#pragma once
#include "winrt\MicrosoftDisplayCaptureTools.Framework.h"
namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers {

    inline winrt::MicrosoftDisplayCaptureTools::Framework::ILogger Logger()
    {
        return winrt::MicrosoftDisplayCaptureTools::Framework::Runtime::GetRuntime().Logger();
    }

    inline winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings RuntimeSettings()
    {
        return winrt::MicrosoftDisplayCaptureTools::Framework::Runtime::GetRuntime().RuntimeSettings();
    }
}