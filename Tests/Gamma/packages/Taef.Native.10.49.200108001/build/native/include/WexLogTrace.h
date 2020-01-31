//----------------------------------------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All Rights Reserved.
//----------------------------------------------------------------------------------------------------------------------
#pragma once

#include "LogTestResults.h"

#pragma push_macro("LogTrace")
#undef LogTrace

// Just including <stdint.h> and using intptr_t/uintptr_t caused build breaks for some existing users.
// Work around this for now by defining our own pointer-sized ints for MSVC++.
#if defined(_MSC_VER)
# ifdef _WIN64
#  define WEX_INTPTR __int64
#  define WEX_UINTPTR unsigned __int64
# else
#  define WEX_INTPTR int
#  define WEX_UINTPTR unsigned int
# endif
#else
# include <stdint.h>
# define WEX_INTPTR intptr_t
# define WEX_UINTPTR uintptr_t
#endif

namespace WEX { namespace Logging
{
    namespace LogTraceAttachment
    {
        enum Attachment : WEX_UINTPTR
        {
            None = 0x0,
            ScreenCapture = 0x1,
            MiniDump = 0x2,
        };

        inline Attachment operator|(Attachment p1, Attachment p2)
        {
            return static_cast<Attachment>(static_cast<WEX_UINTPTR>(p1) | static_cast<WEX_UINTPTR>(p2));
        }
        inline Attachment& operator|=(Attachment& p1, Attachment p2)
        {
            return p1 = p1 | p2;
        }
    }/* namespace LogTraceAttachment */

    // When using this, it is recommended that you use it all in one statement. Here is an example:
    //
    //   Log::Trace(ErrorTrace{L"Message"}.WithContext(L"MyTestLib").WithXmlAnnotation(L"<CustomDataForLogFile />"));
    //
    // The strings are not copied by the trace classes; so if you use temporary strings and don't use the trace class right away,
    // you may end up reading freed memory. Using the trace class on the same line as the Log::Trace call avoids the issue.
    //
    // You can also add source info to a trace like this:
    //
    //   Log::Trace(ErrorTrace{L"Message"}.WithSourceInfo(WEX_LOGGER_CURRENT_SOURCE_INFO));
    //
    // WEX_LOGGER_CURRENT_SOURCE_INFO expands to the current file, function, and line number, but you can also pass the parameters manually.
    class LogTrace
    {
        friend class LogTraceFriend;
    public:
        LogTrace&& WithContext(const wchar_t* pszContext) &&
        {
            m_pszContext = pszContext;
            return static_cast<LogTrace&&>(*this);
        }

#define WEX_LOGGER_CURRENT_SOURCE_INFO __FILEW__, __FUNCTIONW__, __LINE__
        LogTrace&& WithSourceInfo(const wchar_t* pszFile, const wchar_t* pszFunction, int line) &&
        {
            m_pszFile = pszFile;
            m_pszFunction = pszFunction;
            m_line = static_cast<WEX_INTPTR>(line);
            return static_cast<LogTrace&&>(*this);
        }

        // The XML annotation is added to the WTT log file for this trace. It can contain any contents, but it should be valid
        // XML or the log file will be corrupted.
        LogTrace&& WithXmlAnnotation(const wchar_t* pszXmlAnnotation) &&
        {
            m_pszXmlAnnotation = pszXmlAnnotation;
            return static_cast<LogTrace&&>(*this);
        }

        // You can attach a screenshot and/or minidump to any trace using this method.
        LogTrace&& WithAttachment(::WEX::Logging::LogTraceAttachment::Attachment attachment) &&
        {
            m_traceTypeAndAttachments |= (static_cast<WEX_UINTPTR>(attachment) & 0xff) << 8;
            return static_cast<LogTrace&&>(*this);
        }

        // Prevent copying of trace classes.
        LogTrace(const LogTrace&) = delete;
        LogTrace& operator=(const LogTrace&) = delete;
        // Prevent allocation of trace classes.
        static void* operator new(size_t) = delete;
        static void operator delete(void*) = delete;

    protected:
        enum TraceType : WEX_UINTPTR
        {
            Assert,
            Error,
            Warning,
            Comment,
            StartGroup,
            EndGroup,
            KnownBug,
            FileName,
            FileBuffer,
            Property,
            Xml,
            Result,
            LightweightStartGroup,
            LightweightEndGroup,

            TraceTypeCount
        };

        LogTrace(TraceType traceType, const wchar_t* pszMessage, WEX_UINTPTR traceTypeSpecificIntegralValue = 0,
                const void* pTraceTypeSpecificPointerValue = nullptr)
            : m_traceTypeAndAttachments(static_cast<WEX_UINTPTR>(traceType) & 0xff)
            , m_pszMessage(pszMessage)
            , m_traceTypeSpecificIntegralValue(traceTypeSpecificIntegralValue)
            , m_pTraceTypeSpecificPointerValue(pTraceTypeSpecificPointerValue)
            , m_pszContext(nullptr)
            , m_pszFile(nullptr)
            , m_pszFunction(nullptr)
            , m_line(-1)
            , m_pszXmlAnnotation(nullptr)
        {}

    private:
        // All members are the same size to avoid having inconsistent padding across DLLs.

        // In m_traceTypeAndAttachments, the low 8 bits are the TraceType enum value.
        // The next 8 bits are the LogTraceAttachment::Attachment enum value.
        // The remaining bits are reserved and set to zero.
        WEX_UINTPTR m_traceTypeAndAttachments;
        const wchar_t* const m_pszMessage;
        const WEX_UINTPTR m_traceTypeSpecificIntegralValue;
        const void* const m_pTraceTypeSpecificPointerValue;

        const wchar_t* m_pszContext;
        const wchar_t* m_pszFile;
        const wchar_t* m_pszFunction;
        WEX_INTPTR m_line;
        const wchar_t* m_pszXmlAnnotation;
    };
    
#pragma warning(push)
// Disable "copy constructor was implicitly defined as deleted" and "assignment operator was implicitly defined as deleted".
// It is intentional that the derived classes have implicitly deleted copy constructors and assignment operators.
#pragma warning(disable:4625 4626)

    class AssertTrace : public LogTrace
    {
    public:
        explicit AssertTrace(const wchar_t* pszAssert)
            : LogTrace(Assert, pszAssert)
        {}
    };

    class BugTrace : public LogTrace
    {
    public:
        BugTrace(const wchar_t* pszBugDatabase, int bugId)
            : LogTrace(KnownBug, pszBugDatabase, static_cast<WEX_UINTPTR>(bugId))
        {}
    };

    class CommentTrace : public LogTrace
    {
    public:
        explicit CommentTrace(const wchar_t* pszComment)
            : LogTrace(Comment, pszComment)
        {}
    };

    class EndGroupTrace : public LogTrace
    {
    public:
        explicit EndGroupTrace(const wchar_t* pszGroupName)
            : LogTrace(EndGroup, pszGroupName)
        {}
    };

    class ErrorTrace : public LogTrace
    {
    public:
        explicit ErrorTrace(const wchar_t* pszError)
            : LogTrace(Error, pszError)
        {}
    };

    class FileTrace : public LogTrace
    {
    public:
        explicit FileTrace(const wchar_t* pszFilePath)
            : LogTrace(FileName, pszFilePath)
        {}

        FileTrace(const wchar_t* pszFileName, const unsigned char* pFileBuffer, size_t bufferSize)
            : LogTrace(FileBuffer, pszFileName, static_cast<WEX_UINTPTR>(bufferSize), pFileBuffer)
        {}
    };

    class PropertyTrace : public LogTrace
    {
    public:
        PropertyTrace(const wchar_t* pszName, const wchar_t* pszValue)
            : LogTrace(Property, pszName, 0, pszValue)
        {}
    };

    class ResultTrace : public LogTrace
    {
    public:
        explicit ResultTrace(TestResults::Result testResult)
            : LogTrace(Result, nullptr, static_cast<WEX_UINTPTR>(testResult))
        {}

        ResultTrace(TestResults::Result testResult, const wchar_t* pszComment)
            : LogTrace(Result, pszComment, static_cast<WEX_UINTPTR>(testResult))
        {}
    };

    class StartGroupTrace : public LogTrace
    {
    public:
        explicit StartGroupTrace(const wchar_t* pszGroupName)
            : LogTrace(StartGroup, pszGroupName)
        {}

        StartGroupTrace(const wchar_t* pszGroupName, TestResults::Result defaultTestResult)
            : LogTrace(StartGroup, pszGroupName, static_cast<WEX_UINTPTR>(defaultTestResult))
        {}
    };

    class WarningTrace : public LogTrace
    {
    public:
        explicit WarningTrace(const wchar_t* pszWarning)
            : LogTrace(Warning, pszWarning)
        {}
    };

    class XmlTrace : public LogTrace
    {
    public:
        explicit XmlTrace(const wchar_t* pszXml)
            // XML traces don't use the message field.
            : LogTrace(Xml, nullptr, 0, pszXml)
        {}
    };
#pragma warning(pop)
}/* namespace Logging */}/* namespace WEX */

#pragma pop_macro("LogTrace")
