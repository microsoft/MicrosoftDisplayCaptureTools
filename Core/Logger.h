#pragma once
#include <iostream>

namespace winrt::MicrosoftDisplayCaptureTools::Framework
{
    struct Logger : winrt::implements<Logger, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger>
    {
        Logger();
        Logger(std::wostream outStream);
        ~Logger();

        void LogNote(hstring const& note);
        void LogWarning(hstring const& warning);
        void LogError(hstring const& error);
        void LogAssert(hstring const& assert);
        void LogConfig(hstring const& config);

    private:
        hstring GetTimeStamp();

    private:
        std::shared_ptr<std::wostream> m_output;
        std::wfilebuf m_fb;
        std::mutex m_mutex;
    };
}