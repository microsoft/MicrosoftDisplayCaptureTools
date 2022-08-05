#include "pch.h"
#include "Logger.h"
#include <chrono>

#define TIMESTAMPLENGTH

using std::chrono::system_clock;

namespace winrt::MicrosoftDisplayCaptureTools::Framework
{
    // Default constructor just outputs to standard out.
    Logger::Logger()
    {
        m_output.reset(&std::wcout);
    }

    // Optionally provide an output stream to use instead of standard
    Logger::Logger(std::wostream outStream)
    {
        m_output.reset(&outStream);
    }

    Logger::~Logger()
    {
        if (m_fb.is_open())
        {
            m_fb.close();
        }
    }

    // Retrieve the current time to be printed at the start of log entries.
    // 
    // Note: this is not thread-safe, and should only be called where m_mutex
    //       is already locked.
    hstring GetTimeStamp()
    {
        auto time = system_clock::to_time_t(system_clock::now());
        tm timeBuf;
        char timeStampBuf[23] = {0};

        winrt::check_win32(localtime_s(&timeBuf, &time));
        
        std::strftime(timeStampBuf, sizeof(timeStampBuf), "[%Y:%m:%d::%H:%M:%S] ", &timeBuf);

        auto str = std::string(timeStampBuf);
        auto wstr = std::wstring(str.begin(), str.end());
        return hstring(wstr);
    }

    void Logger::LogNote(hstring const& note)
    {
        auto lock = std::scoped_lock(m_mutex);
        *m_output << note.c_str() << std::endl;
    }

    void Logger::LogWarning(hstring const& warning)
    {
        auto lock = std::scoped_lock(m_mutex);
        *m_output << warning.c_str() << std::endl;
    }

    void Logger::LogError(hstring const& error)
    {
        auto lock = std::scoped_lock(m_mutex);
        *m_output << error.c_str() << std::endl;
    }

    void Logger::LogAssert(hstring const& assert)
    {
        auto lock = std::scoped_lock(m_mutex);
        *m_output << assert.c_str() << std::endl;
    }

    void Logger::LogConfig(hstring const& config)
    {
        auto lock = std::scoped_lock(m_mutex);
        *m_output << config.c_str() << std::endl;
    }
}