#pragma once
#include <iostream>

namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities 
{
    static std::wstring GetTimeStamp();

    struct LoggerAltMode : winrt::implements<LoggerAltMode, winrt::MicrosoftDisplayCaptureTools::Framework::ILoggerMode>
    {
        LoggerAltMode(std::atomic_bool& mode, std::atomic_uint32_t& errors) : m_setting(mode), m_errors(errors)
        {
            m_setting = true;
            m_errors = 0;
        }
        ~LoggerAltMode()
        {
            m_setting = false;
        }

        boolean HasErrored()
        {
            return m_errors > 0;
        }

    private:
        std::atomic_bool& m_setting;
        std::atomic_uint32_t& m_errors;
    };

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

        winrt::MicrosoftDisplayCaptureTools::Framework::ILoggerMode LogErrorsAsWarnings();

    private:
        std::shared_ptr<std::wostream> m_output;
        std::wfilebuf m_fb;
        std::recursive_mutex m_mutex;

        std::atomic_bool m_logErrorsAsWarnings;
        std::atomic_uint32_t m_loggedErrorsAsWarnings;
    };
}