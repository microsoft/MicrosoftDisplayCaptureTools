#include "pch.h"
#include "Logger.h"

#include <Wex.Common.h>
#include <WexLogTrace.h>
#include <Wex.Logger.h>
#include <LogController.h>
#include <Log.h>
#include <iostream>

using namespace WEX::Logging;
using namespace WEX::Common;

namespace winrt::MicrosoftDisplayCaptureTools::Tests::Logging {
// callback function called if there is an issue with the logging backend.
static void __stdcall LoggingErrorCallback(const unsigned short* pszMessage, HRESULT hr)
{
    std::cout << "The logging system ran into an error: " << pszMessage << std::endl;
    std::cout << "Error Code: " << hr << std::endl;
}

WEXLogger::WEXLogger() : m_selfInitialized(!LogController::IsInitialized())
{
    // Do we need to initialize the process log controller
    if (m_selfInitialized)
    {
        // Attempt to create a log file
        BOOL success = 0;
        DWORD error = 0;
        wchar_t procName[MAX_PATH] = L"";
        DWORD procNameLength = MAX_PATH;
        auto proc = GetCurrentProcess();
        success = QueryFullProcessImageName(proc, 0, procName, &procNameLength);

        if (success != 0)
        {
            auto path = std::filesystem::path(procName);
            auto envVar = path.stem() + winrt::hstring(L"_CMD");
            success = SetEnvironmentVariable(envVar.c_str(), L"/enablewttlogging");
        }

        if (success != 0)
        {
            error = GetLastError();
        }

        LogController::InitializeLogging(LoggingErrorCallback);

        // If we are not able to create a log file for the test run, log a warning before we start.
        if (success == 0)
        {
            Log::Warning(String().Format(L"Unable to create log file. Error code: %d", error));
        }
    }
}

WEXLogger::~WEXLogger()
{
    // Do we need to uninitialize the process log controller
    if (m_selfInitialized)
    {
        LogController::FinalizeLogging();
    }
}

void WEXLogger::LogNote(hstring const& note)
{
    Log::Comment(note.c_str());
}

void WEXLogger::LogWarning(hstring const& warning)
{
    Log::Warning(warning.c_str());
}

void WEXLogger::LogError(hstring const& error)
{
    Log::Error(error.c_str());
}

void WEXLogger::LogAssert(hstring const& assert)
{
    Log::Assert(assert.c_str());
}

void WEXLogger::LogConfig(hstring const& config)
{
    Log::Comment(config.c_str());
}
} // namespace winrt::MicrosoftDisplayCaptureTools::Tests::Logging