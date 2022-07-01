#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::Framework
{
    struct WEXLogger : winrt::implements<WEXLogger, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger>
    {
        WEXLogger();
        ~WEXLogger();

        void LogNote(hstring const& note);
        void LogWarning(hstring const& warning);
        void LogError(hstring const& error);
        void LogAssert(hstring const& assert);

    private:
        const bool m_selfInitialized;
    };
}