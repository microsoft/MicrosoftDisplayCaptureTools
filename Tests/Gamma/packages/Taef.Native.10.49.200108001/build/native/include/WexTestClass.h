//----------------------------------------------------------------------------------------------------------------------
/// \file
/// <summary>Test Authoring and Execution Framework native macro definitions</summary>
// Copyright (c) Microsoft Corporation.  All Rights Reserved.
//----------------------------------------------------------------------------------------------------------------------
#pragma once

#if _MSC_VER < 1900
# error Unsupported version of Visual Studio. WexTestClass.h requires VS 2015 or later.
#endif

#pragma warning(push)
#pragma warning(disable:4481)

#if !defined(__clang__)
#pragma comment(lib, "TE.Common.lib")
#pragma comment(lib, "Wex.Common.lib")
#pragma comment(lib, "Wex.Logger.lib")
#endif

// Allow anyone who has defined an Assert macro to compile with this header file included
#pragma push_macro("Assert")
#undef Assert

#include "Interruption.h"
#include "RuntimeParameters.h"
#include "TestData.h"
#include "Verify.h"
#include "WexAssert.h"
#include "WexDebug.h"

// excpt.h is used for GetExceptionCode, which is used to detect C++ exceptions in a structured exception handler.
#include <excpt.h>

#if defined(_CPPUNWIND)
#include "WexException.h"

#include <new>
#endif

#pragma push_macro("_Outptr_")
#pragma push_macro("_Outptr_result_maybenull_z_")
#pragma push_macro("_Ret_maybenull_z_")
#pragma push_macro("_In_reads_z_")
#pragma push_macro("_In_")
#pragma push_macro("_Post_invalid_")

#if !defined(_Outptr_)
#define _Outptr_
#endif

#if !defined(_Outptr_result_maybenull_z_)
#define _Outptr_result_maybenull_z_
#endif

#if !defined(_Ret_maybenull_z_)
#define _Ret_maybenull_z_
#endif

#if !defined(_In_reads_z_)
#define _In_reads_z_(x)
#endif

#if !defined(_In_)
#define _In_
#endif

#if !defined(_Post_invalid_)
#define _Post_invalid_
#endif

/// \internal
/// <summary> Wide string conversion helper </summary>
#define TAEF_WIDEN_INT(x) L ## x

/// \internal
/// <summary> Wide string version of "stringizing" operator </summary>
#define TAEF_WIDEN(x) TAEF_WIDEN_INT(x)

/// \internal
/// <summary>Wide string version of __FUNCTION__ macro </summary>
#define TAEF__WFUNCTION__ TAEF_WIDEN(__FUNCTION__)

/// \internal
/// <summary>Stringize internal macro </summary>
#define TAEF_STRINGIZE_INT(x) #x

/// \internal
/// <summary>Stringize macro </summary>
#define TAEF_STRINGIZE(x) TAEF_STRINGIZE_INT(x)

/// \internal
/// <summary>TAEF version number.</summary>
#define TAEF_ABI_VERSION 10

/// \internal
/// <summary>The magic number that will lead the testdata segment in v3.3 and later. This is "TAEF" in ascii.</summary>
#define TAEF_HEADER_SIGNATURE 0x46454154

#ifdef INLINE_TEST_METHOD_MARKUP
#define TAEF_TERMINATOR
#else
#define TAEF_TERMINATOR ;
#endif

#if defined(__cpp_inline_variables) && __cpp_inline_variables && !defined(TAEF_AVOID_INLINE_STATICS) && !defined(TAEF_USE_INLINE_STATICS)
# define TAEF_USE_INLINE_STATICS
#endif

#if defined(__cpp_constexpr) && __cpp_constexpr && !defined(TAEF_AVOID_CONSTEXPR) && !defined(TAEF_USE_CONSTEXPR)
# define TAEF_USE_CONSTEXPR
#endif

#if defined(TAEF_USE_CONSTEXPR)
// We mark the testdata section as read-only memory. The structs we place inside this section need to avoid dynamic initializers for two reasons:
// * TAEF reflects over the DLL by reading directly from the testdata section without actually loading the DLL for execution. A dynamic initializer
//   will make it so the data is not written to the testdata section until the function containing the dynamic initializer is run.
// * Since the testdata section is marked as read-only, the dynamic initializer will crash when it tries to write to the testdata section.
// The test code can cause a dyanmic initializer to be used by doing something like setting metadata to a string that is the return value of
// a function. Using constexpr causes these cases to be compiler errors. When constexpr is not available, we fall back to const although that does
// not cause build breaks. Instead, you will get dynamic initializers which may result in crashes when the DLL loads.
# define TAEF_CONST_TESTDATA constexpr
#else
# define TAEF_CONST_TESTDATA const
#endif

#if !defined(__cplusplus_cli)

#if !defined(__INTRIN_H_)
// Including intrin.h from WexTestClass.h causes build breaks for some users of WexTestClass.h.
extern "C" __declspec(noreturn) void __fastfail(unsigned int);
#endif // #if !defined(__INTRIN_H_)

#define TAEF_FAST_FAIL __fastfail(7 /* FAST_FAIL_FATAL_APP_EXIT */)
#else
// Using __fastfail with C++/CLI causes warning C4793 to be raised.
#define TAEF_FAST_FAIL System::Environment::FailFast(nullptr)
#endif // #if !defined(__cplusplus_cli)

/// <summary>
/// The WEX namespace contains classes used for marking up C++ Tests.
/// </summary>
namespace WEX
{
    namespace Private
    {
        extern "C" __declspec(dllimport) int __stdcall DoNotDiscard(const void*);

#if defined(__clang__)
        template <size_t Size>
        struct CharacterArray
        {
            const wchar_t m_sz[Size];
        };

        template <size_t... Indices>
        struct IndexSequence {};

        template <typename IndexSequenceType, size_t NewIndex>
        struct AppendIndexSequence;

        template <size_t... ExistingIndices, size_t NewIndex>
        struct AppendIndexSequence<IndexSequence<ExistingIndices...>, NewIndex>
        {
            using Type = IndexSequence<ExistingIndices..., NewIndex>;
        };

        template <size_t IndexCount>
        struct IndexSequenceGenerator;

        template <>
        struct IndexSequenceGenerator<0>
        {
            using Type = IndexSequence<>;
        };

        template <size_t IndexCount>
        struct IndexSequenceGenerator
        {
            using Type = typename AppendIndexSequence<typename IndexSequenceGenerator<IndexCount - 1>::Type, IndexCount - 1>::Type;
        };

        template <size_t IndexCount>
        constexpr auto MakeIndexSequence()
        {
            return typename IndexSequenceGenerator<IndexCount>::Type{};
        }

        // Helper function to index into arrays to build up big array with concatenated characters of two c strings. 
        // Uses template function arg deduction to pull out NumChars from constant c strings and parameter pack expansion with
        // an index sequence to rewrite into a new constant "string like object" that resembles a C++ qualified member name.
        //
        // TL;DR:
        //    "ClassName", "MethodName" -> "ClassName::MethodName"
        //    all at compile time because of constexpr and template magic.
        template<unsigned NumChar1, unsigned NumChar2, size_t... Indices1, size_t... Indices2>
        constexpr CharacterArray<NumChar1 + NumChar2 + 1> BuildQualifiedName(
            wchar_t const (&str1)[NumChar1], wchar_t const (&str2)[NumChar2], IndexSequence<Indices1...>, IndexSequence<Indices2...>) {
            static_assert(sizeof...(Indices1) < NumChar1, "Range of characters to use from first string must not include null terminator.");
            return {{ str1[Indices1]..., L':', L':', str2[Indices2]...}};
        }

        // Helper to make calling the other one a little nicer using template argument type deduction.
        template<unsigned NumChar1, unsigned NumChar2>
        constexpr CharacterArray<NumChar1 + NumChar2 + 1> BuildQualifiedName(
            wchar_t const (&str1)[NumChar1], wchar_t const (&str2)[NumChar2]) {
            return BuildQualifiedName(str1, str2, MakeIndexSequence<NumChar1 - 1>(), MakeIndexSequence<NumChar2>());
        }
#endif

        /// \internal
        /// <summary>
        /// A SEH filter.
        /// </summary>
        unsigned long inline DetectMSVCExceptions(unsigned long exceptionCode, bool& isCPlusPlusException)
        {
            // If the 'exceptionCode' is 0xE06D7363 (which is the exception code that MSVC uses to implement
            // C++ exceptions using SEH), then this is a C++ exception. Set isCPlusPlusException to indicate
            // this. This is used by the catch(...) handler which does a fast-fail if it is not a C++
            // exception. The catch(...) will only be invoked for non-C++ exceptions if the /EHa switch is
            // used on the compiler while compiling the test DLL.
            isCPlusPlusException = (exceptionCode == 0xE06D7363);

#if !defined(_CPPUNWIND)
            if (isCPlusPlusException)
            {
                ::WEX::Logging::Log::Error(L"A C++ exception was throw by a test in a DLL that was compiled with C++ exceptions disabled.");
                // In this case, if we let the exception continue the search, it would be caught by TAEF code. We want to avoid
                // catching the exception so the process can crash with the right context.
                
                // To fix this in a Visual Studio project, change the Configuration Properties -> C/C++ ->
                // Code Generation -> Enable C++ Exceptions setting in the project properties from "No" to "Yes (/EHsc)".
                // To fix this in the build.exe-based build system from the old Windows Driver Kit, add
                // "USE_NATIVE_EH=1" to the sources file.
                TAEF_FAST_FAIL;
            }
#endif

            return EXCEPTION_CONTINUE_SEARCH;
        }

        template <typename TFunctor>
        bool SafeInvoke_Impl(const TFunctor& functor, bool& isCPlusPlusException)
        {
            bool result = false;
            __try
            {
                result = functor();
            }
            __except (::WEX::Private::DetectMSVCExceptions(GetExceptionCode(), isCPlusPlusException))
            {
                // This is unreachable because DetectMSVCExceptions returns EXCEPTION_CONTINUE_SEARCH.
                TAEF_FAST_FAIL;
            }
            return result;
        }

        struct StringManager
        {
            template <size_t Size>
            _Ret_maybenull_z_ static wchar_t* Allocate(const wchar_t (&szLiteral)[Size])
            {
                return Allocate(szLiteral, Size - 1);
            }

            _Ret_maybenull_z_ static wchar_t* Allocate(const ::WEX::Common::NoThrowString& str)
            {
                if (str.IsValid())
                {
                    return Allocate(str, static_cast<size_t>(str.GetLength()));
                }

                return nullptr;
            }

            _Ret_maybenull_z_ static wchar_t* Allocate(_In_reads_z_(length+1) const wchar_t* pszString, size_t length)
            {
                WEX_ASSERT(pszString[length] == L'\0', L"The string is not null-terminated.");

                wchar_t* pszRet = nullptr;
#if defined(_CPPUNWIND)
                try
                {
#endif
                    // We aren't using "new wchar_t[length+1]" as that caused some linking problems for some existing tests.
                    pszRet = static_cast<wchar_t*>(::operator new(sizeof(wchar_t)*(length+1)));
#if defined(_CPPUNWIND)
                }
                catch (const std::exception&)
                {
                    // If new fails, pszRet will be null.
                }
#endif

                if (pszRet)
                {
                    for (size_t i = 0; i < length; ++i)
                    {
                        pszRet[i] = pszString[i];
                    }
                    // Suppress false positive in code analysis. This write is within the bounds of the allocation.
#pragma warning(suppress:6386)
                    pszRet[length] = L'\0';
                }

                return pszRet;
            }

            static void __cdecl Deallocate(_In_ _Post_invalid_ wchar_t* pszString)
            {
                ::operator delete(static_cast<void*>(pszString));
            }
        };
    }/* namespace Private */

    /// \internal
    /// <summary>
    /// Identifiers for the structs that make up the native ABI.
    /// </summary>
    namespace TAEF_Identifier
    {
        enum Value : uintptr_t
        {
            TestClassInfo = 1,
            TestMethodInfo,
            ModuleSetup,
            ModuleCleanup,
            TestClassSetup,
            TestClassCleanup,
            TestMethodSetup,
            TestMethodCleanup,
            ModuleMetadata,
            TestClassMetadata,
            TestMethodMetadata,
        }; 
    }

    template <typename TFunctor>
    HRESULT SafeInvoke(const TFunctor& functor)
    {
        // This is set to false if a structured exception which is not a C++ exception is raised by the test or fixture method.
        bool exceptionIsCPlusPlusException = true;
#if defined(_CPPUNWIND)
        try
        {
#endif
            return ::WEX::Private::SafeInvoke_Impl(functor, exceptionIsCPlusPlusException)
                ? S_OK : S_FALSE;
#if defined(_CPPUNWIND)
        }
# if !defined(NO_VERIFY_EXCEPTIONS)
        catch (const ::WEX::TestExecution::VerifyFailureException& e)
        {
            return e.ErrorCode();
        }
# endif
        catch (...)
        {            
            // exceptionIsCPlusPlusException will always be true if the test is not compiled with /EHa.
            if (!exceptionIsCPlusPlusException)
            {
                // This catch (...) just caught a structured exception which is not a C++ exception.
                // This means the test DLL was compiled using the /EHa switch on the compiler.
                // See https://msdn.microsoft.com/en-us/library/1deeycx5.aspx for the /EH documentation.
                // The use of /EHa is discouraged. Please consider switching to /EHsc, which instructs
                // the compiler to only catch C++ exceptions with catch (...) and to assume that functions
                // declared as extern "C" cannot throw.
                // 
                // To fix this in a Visual Studio project, change the Configuration Properties -> C/C++ ->
                // Code Generation -> Enable C++ Exceptions setting in the project properties from
                // "Yes with SEH Exceptions (/EHa)" to "Yes (/EHsc)".
                // To fix this in the build.exe-based build system from the old Windows Driver Kit, change
                // "USE_NATIVE_EH=ASYNC" to "USE_NATIVE_EH=1" in the sources file.
                TAEF_FAST_FAIL;
            }

            ::WEX::Common::NoThrowString messageBuffer;
            HRESULT hr = S_OK;
            ::WEX::Logging::Log::Error(::WEX::TestExecution::Private::GetExceptionInfo(messageBuffer, &hr));

            return hr;
        }
#endif // if defined(_CPPUNWIND)
    }

    /// <summary>
    /// Legacy base class that all test classes used to have to inherit from.
    /// </summary>
    template <class T>
    class TestClass {};

    #pragma warning(push)
    #pragma warning(disable:6101)
    template <typename T>
    struct TestClassFactory
    {
        /// \internal
        /// <summary>
        /// Creates an instance of a test class
        /// </summary>
        static HRESULT __cdecl CreateInstance(_Outptr_ void** ppInstance, _Outptr_result_maybenull_z_ wchar_t** ppszMessage)
        {
            *ppszMessage = nullptr;
            // This is set to false if a structured exception which is not a C++ exception is raised by the class constructor.
            bool exceptionIsCPlusPlusException = true;
#if defined(_CPPUNWIND)
            try
            {
#endif
                *ppInstance = CreateInstanceWithTryExcept(exceptionIsCPlusPlusException);
                if (!*ppInstance)
                {
                    *ppszMessage = ::WEX::Private::StringManager::Allocate(L"Failed to allocate the test class");
                    return E_OUTOFMEMORY;
                }

                return S_OK;
#if defined(_CPPUNWIND)
            }
# if !defined(NO_VERIFY_EXCEPTIONS)
            catch (const ::WEX::TestExecution::VerifyFailureException&)
            {
                *ppszMessage = ::WEX::Private::StringManager::Allocate(L"Verify failure");
                return E_FAIL;
            }
# endif
            catch (const ::WEX::Common::Exception& e)
            {
                *ppszMessage = ::WEX::Private::StringManager::Allocate(
                    ::WEX::Common::NoThrowString().Format(L"Caught WEX::Common::Exception: %s", e.Message()));
                return e.ErrorCode();
            }
#if defined(__cplusplus_winrt)
            catch (Platform::Exception^ e)
            {
                *ppszMessage = ::WEX::Private::StringManager::Allocate(
                    ::WEX::Common::NoThrowString().Format(L"Caught Platform::Exception^: %s", e->Message->Data()));
                return e->HResult;
            }
#endif // if defined(__cplusplus_winrt)
# if defined(TAEF_CPPWINRT_SUPPORT)
            catch (const winrt::hresult_error& e)
            {
                *ppszMessage = ::WEX::Private::StringManager::Allocate(
                    ::WEX::Common::NoThrowString().Format(L"Caught winrt::hresult_error: %s", e.message().c_str()));
                return e.code();
            }
# endif // if defined(TAEF_CPPWINRT_SUPPORT)
# if defined(TAEF_SYSTEM_ERROR_SUPPORT)
            catch (const std::system_error& e)
            {
                ::WEX::Common::NoThrowString message;
                message.Format(L"Caught std::system_error: %S", e.what());
                // std::system_error often ends its messages with a new-line. Trim that off if it is present.
                message.TrimRight();
                *ppszMessage = ::WEX::Private::StringManager::Allocate(message);
                bool isExactConversion;
                return ::WEX::TestExecution::Private::GetHResultForStdErrorCode(e.code(), isExactConversion);
            }
# endif // if defined(TAEF_SYSTEM_ERROR_SUPPORT)
            catch (const std::exception& e)
            {
                *ppszMessage = ::WEX::Private::StringManager::Allocate(
                    ::WEX::Common::NoThrowString().Format(L"Caught std::exception: %S", e.what()));
                return E_FAIL;
            }
            catch (...)
            {
                // exceptionIsCPlusPlusException will always be true if the test is not compiled with /EHa.
                if (!exceptionIsCPlusPlusException)
                {
                    // This catch (...) just caught a structured exception which is not a C++ exception.
                    // This means the test DLL was compiled using the /EHa switch on the compiler.
                    // See https://msdn.microsoft.com/en-us/library/1deeycx5.aspx for the /EH documentation.
                    // The use of /EHa is discouraged. Please consider switching to /EHsc, which instructs
                    // the compiler to only catch C++ exceptions with catch (...) and to assume that functions
                    // declared as extern "C" cannot throw.
                    // 
                    // To fix this in a Visual Studio project, change the Configuration Properties -> C/C++ ->
                    // Code Generation -> Enable C++ Exceptions setting in the project properties from
                    // "Yes with SEH Exceptions (/EHa)" to "Yes (/EHsc)".
                    // To fix this in the build.exe-based build system from the old Windows Driver Kit, change
                    // "USE_NATIVE_EH=ASYNC" to "USE_NATIVE_EH=1" in the sources file.
                    TAEF_FAST_FAIL;
                }

                *ppszMessage = ::WEX::Private::StringManager::Allocate(L"Caught an unidentified C++ exception.");
                return E_UNEXPECTED;
            }
#endif // if defined(_CPPUNWIND)
        }

        /// \internal
        /// <summary>
        /// Creates an instance of a test class
        /// </summary>
        static T* __cdecl CreateInstanceWithTryExcept(bool& exceptionIsCPlusPlusException)
        {
            T* pInstance = nullptr;
            __try
            {
                // We have to call the constructor in a separate function to avoid error C2712.
                pInstance = CallDefaultConstructor();
            }
            __except (::WEX::Private::DetectMSVCExceptions(GetExceptionCode(), exceptionIsCPlusPlusException))
            {
                // This is unreachable because DetectMSVCExceptions returns EXCEPTION_CONTINUE_SEARCH.
                TAEF_FAST_FAIL;
            }
            return pInstance;
        }

        static T* __cdecl CallDefaultConstructor()
        {
            // What the caller links to controls the throwing behavior. We would prefer to use
            // new (std::nothrow) T(), but that doesn't link for some existing test binaries; so
            // we just use plain new.
            return new T();
        }

        /// \internal
        /// <summary>
        /// Destroys an instance of a test class
        /// </summary>
        static void __cdecl DestroyInstance(void* pTestClass)
        {
            delete static_cast<T*>(pTestClass);
        }
    };
    #pragma warning(pop)

    template <class T>
    struct TestInvokeFunctor
    {
        typedef void (__thiscall T::*TestMethod)();
    public:
        TestInvokeFunctor(T& instance, TestMethod pTestMethod)
            : m_pTestMethod(pTestMethod)
            , m_instance(instance)
        {}

        bool operator() () const
        {
#if defined(_CPPUNWIND) && !defined(NO_VERIFY_EXCEPTIONS)
            try
            {
#endif
                (m_instance.*m_pTestMethod)();
#if defined(_CPPUNWIND) && !defined(NO_VERIFY_EXCEPTIONS)
            }
            catch (const ::WEX::TestExecution::VerifyFailureException&)
            {
                // Don't re-throw here. The error is already tracked in WEX.Logger.
            }
#endif

            return true;
        }

    private:
        TestInvokeFunctor(const TestInvokeFunctor&) = delete;
        TestInvokeFunctor& operator=(const TestInvokeFunctor&) = delete;

        // Store the pointer-to-member before the reference to avoid warning C4121 for tests
        // that use #pragma pointers_to_members(full_generality, virtual_inheritance).
        TestMethod m_pTestMethod;
        T& m_instance;
    };

    template <class T>
    struct FixtureInvokeFunctor
    {
        typedef bool (__thiscall T::*FixtureMethod)();
    public:
        FixtureInvokeFunctor(T& instance, FixtureMethod pFixtureMethod)
            : m_pFixtureMethod(pFixtureMethod)
            , m_instance(instance)
        {}

        bool operator() () const
        {
            return (m_instance.*m_pFixtureMethod)();
        }

    private:
        FixtureInvokeFunctor(const FixtureInvokeFunctor&) = delete;
        FixtureInvokeFunctor& operator=(const FixtureInvokeFunctor&) = delete;

        // Store the pointer-to-member before the reference to avoid warning C4121 for tests
        // that use #pragma pointers_to_members(full_generality, virtual_inheritance).
        FixtureMethod m_pFixtureMethod;
        T& m_instance;
    };

    /// \internal
    /// <summary>
    /// Metadata storage classes
    /// Store various metadata about test classes in binary's "testdata" section
    /// </summary>

    /// \internal
    /// <summary>
    ///  Test class related metadata storage class
    ///  Resolves a class name to pointers to class creation and destruction functions
    /// </summary>
    struct TestClassInfo
    {
        /// \internal
        /// <summary> Pointer to a class 'creation' function</summary>
        typedef HRESULT (__cdecl *ClassCreator)(_Outptr_ void** ppInstance, _Outptr_result_maybenull_z_ wchar_t** ppszMessage);

        /// \internal
        /// <summary> Pointer to a class 'destroyer' function</summary>
        typedef void (__cdecl *ClassDestroyer)(void*);

        TAEF_Identifier::Value identifier;
        const wchar_t* pszClassName;

        ClassCreator pClassCreator;
        ClassDestroyer pClassDestroyer;
    };

    typedef HRESULT (__cdecl *MethodInvokerFunction)(void* pTestClass);

    /// \internal
    /// <summary>
    /// Holds test method information that is relevant to the test loader.
    /// </summary>
    struct TestMethodInfo
    {
        TAEF_Identifier::Value identifier;
        const wchar_t* pszMethodName;
        const wchar_t* pszQualifiedMethodName;
        uintptr_t index;
        MethodInvokerFunction pInvoker;
    };

    /// \internal
    /// <summary>
    /// Holds test or class setup or cleanup method information that is relevant to the test loader.
    /// </summary>
    struct FixtureMethodInfo
    {
        TAEF_Identifier::Value identifier;
        const wchar_t* pszMethodName;
        const wchar_t* pszQualifiedMethodName;
        MethodInvokerFunction pInvoker;
    };

    /// \internal
    /// <summary>
    ///  Holds various types of properties
    ///  (module, class or function level)
    ///  pszMetadataIdentifier identifies what kind of property is stored
    /// </summary>
    struct TestPropertyMetadata
    {
        TAEF_Identifier::Value identifier;
        const wchar_t* pszPropertyName;
        const wchar_t* pszPropertyValue;
    };

    /// \internal
    /// <summary>
    /// Dispatches module setup and cleanup function names to their corresponding pointers
    /// </summary>
    struct TestGlobalFunctionInfo
    {
        typedef HRESULT (__cdecl *TestModuleInvokerFunction)();

        TAEF_Identifier::Value identifier;
        const wchar_t* pszFunctionName;
        TestModuleInvokerFunction pInvoker;
    };

    /// \internal
    /// <summary>
    /// Holds framework version info
    /// </summary>
    struct TestVersionInfo
    {
        typedef void (__cdecl *Deallocator)(_In_ _Post_invalid_ wchar_t* pStr);

        uintptr_t headerSignature;
        uintptr_t abiVersion;
        const char* paszGenerator;
        Deallocator pDeallocator;
    };

} // end of namespace declaration

#pragma section("testdata$a_TDH", read)
#pragma section("testdata$b_TCI", read)
#pragma section("testdata$c_TMI", read)
#pragma section("testdata$d_TGFI", read)
#pragma section("testdata$e_FMI", read)
#pragma section("testdata$f_MM", read)
#pragma section("testdata$g_TCM", read)
#pragma section("testdata$h_TMM", read)

#define TAEF_ATTRIBUTE_SECTION(sectionName)                 __declspec(allocate(sectionName))

// The IntelliSense compiler reports errors when trying to parse 'TAEF_PIN_FUNCTION_SYMBOL', so it should be skipped.
#if defined(__INTELLISENSE__)
# define TAEF_PIN_FUNCTION_SYMBOL
#elif defined(__clang__)
/// \internal
/// <summary> Instruct the compiler not to remove the function from the obj file during the optimization phase </summary>
# define TAEF_PIN_FUNCTION_SYMBOL                 __attribute__((used)) 
#else
/// \internal
/// <summary> Instruct the compiler not to remove the function from the obj file during the optimization phase </summary>
#if defined(TAEF_USE_INLINE_STATICS)
# define TAEF_PIN_FUNCTION_SYMBOL
#else
# define TAEF_PIN_FUNCTION_SYMBOL                 __pragma (comment(linker, "/include:" __FUNCDNAME__))
#endif
#endif

#if defined (__clang__)

# define TAEF_DEFINE_CLASS_NAME(className) \
    static constexpr const wchar_t TAEF_szClassName[] = L# className

# define TAEF_QUALIFIED_METHOD_NAME &TAEF_szQualifiedMethodName.m_sz[0]
# define TAEF_DEFINE_QUALIFIED_METHOD_NAME(methodName) \
    static constexpr auto TAEF_szQualifiedMethodName = WEX::Private::BuildQualifiedName(TAEF_szClassName, methodName)

#else

# define TAEF_DEFINE_CLASS_NAME(className)

# define TAEF_QUALIFIED_METHOD_NAME TAEF__WFUNCTION__ 
# define TAEF_DEFINE_QUALIFIED_METHOD_NAME(methodName)

#endif

// Ignore the following warnings:
// * 28651: static initializer causes copy on write pages due to member function pointers
// * 4640: construction of local static object is not thread-safe
// * 4407: cast between different pointer to member representations, compiler may generate incorrect code
#define TAEF_PUSH_IGNORE_WARNINGS                __pragma (warning(push)) \
                                                 __pragma (warning(disable:28651)) \
                                                 __pragma (warning(disable:4640)) \
                                                 __pragma (warning(disable:4407))

#define TAEF_POP_IGNORE_WARNINGS                 __pragma (warning(pop))

#if defined(__INTELLISENSE__)
// The IntelliSense compiler reports errors when trying to parse TAEF_REGISTER_FIXTURE_METHOD and TAEF_REGISTER_TEST_METHOD.
# define TAEF_REGISTER_FIXTURE_METHOD(methodName, methodType)
# define TAEF_REGISTER_TEST_METHOD(methodName)
#else

# define TAEF_REGISTER_FIXTURE_METHOD(methodName, methodType) \
    struct TAEF_FixtureInvoker \
    { \
        static HRESULT __cdecl TAEF_Invoke(void* pTestClass) \
        { \
            /* TAEF_TestClassType was typedef'd in the TEST_CLASS macro so we can safely cast from \
               void* to TAEF_TestClassType* and invoke the correct derived type's member functions */ \
            return ::WEX::SafeInvoke(::WEX::FixtureInvokeFunctor<TAEF_TestClassType>(*reinterpret_cast<TAEF_TestClassType*>(pTestClass), &TAEF_TestClassType::methodName)); \
        } \
    }; \
    TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
    TAEF_PUSH_IGNORE_WARNINGS \
    TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$e_FMI") \
    static TAEF_CONST_TESTDATA ::WEX::FixtureMethodInfo s_testInfo = {methodType, L#methodName, TAEF_QUALIFIED_METHOD_NAME, &TAEF_FixtureInvoker::TAEF_Invoke}; \
    TAEF_POP_IGNORE_WARNINGS \
    return &s_testInfo; /* Return a pointer to s_testInfo in order to pin the struct into the dll so it does not get stripped out by the linker. */

// TAEF_REGISTER_TEST_METHOD uses the same value for methodName and methodToInvoke. The macros that start with "TAEF_" in this file are not meant
// for tests to use directly. However, some tests took a dependency on TAEF internals to enable the use of a separate wrapper function that invokes
// the actual test. This macro was added so those tests could be updated to use this macro instead of having their own copies of the TAEF macros.
# define TAEF_REGISTER_TEST_METHOD_IMPL(methodName, methodToInvoke) \
    static const TAEF_CPP_EXPORT ::WEX::TestMethodInfo* methodName##_TAEF_PinTestMethodInfo() \
    { \
        struct TAEF_TestInvoker \
        { \
            static HRESULT __cdecl TAEF_Invoke(void* pTestClass) \
            { \
                /* TAEF_TestClassType was typedef'd in the TEST_CLASS macro so we can safely cast from \
                   void* to TAEF_TestClassType* and invoke the correct type's member functions */ \
                return ::WEX::SafeInvoke(::WEX::TestInvokeFunctor<TAEF_TestClassType>(*reinterpret_cast<TAEF_TestClassType*>(pTestClass), &TAEF_TestClassType::methodToInvoke)); \
            } \
        }; \
        TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
        TAEF_PUSH_IGNORE_WARNINGS \
        TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$c_TMI") \
        static TAEF_CONST_TESTDATA ::WEX::TestMethodInfo s_testInfo = {::WEX::TAEF_Identifier::TestMethodInfo, L#methodName, TAEF_QUALIFIED_METHOD_NAME,  \
                                                                       __COUNTER__ - TAEF_TestMethodIndexOffset, &TAEF_TestInvoker::TAEF_Invoke}; \
        TAEF_POP_IGNORE_WARNINGS \
        return &s_testInfo; /* Return a pointer to s_testInfo in order to pin the struct into the dll so it does not get stripped out by the linker. */ \
    } \
    TAEF_COMPILER_PIN(methodName##_TAEF_PinTestMethodInfo)

# define TAEF_REGISTER_TEST_METHOD(methodName) \
    TAEF_REGISTER_TEST_METHOD_IMPL(methodName, methodName)
#endif

#if defined(TAEF_USE_INLINE_STATICS)
# define TAEF_COMPILER_PIN(methodName) \
    inline static int s_doNotDiscard_##methodName = ::WEX::Private::DoNotDiscard(methodName());

# define TAEF_CPP_EXPORT

# define TAEF_STATIC_IF_CALLED_BY_TAEF_COMPILER_PIN static
#else
# define TAEF_COMPILER_PIN(methodName)

# define TAEF_CPP_EXPORT __declspec(dllexport)

# define TAEF_STATIC_IF_CALLED_BY_TAEF_COMPILER_PIN
#endif

// This macro is used directly by some non-TAEF teams. Do not remove it unless those cases are fixed.
#define TAEF_TEST_METHOD(methodName) \
    TAEF_REGISTER_TEST_METHOD(methodName) \
    __pragma(warning(suppress:25007)) /* Disable warning that member method may be static */ \
    void methodName()

/// <summary>
/// Macro for declaring a test method without associating it with any metadata
/// </summary>
/// Example:
/// \code
///    class TestFeatureClass
///    {
///         TEST_METHOD(FeatureTestMethod1);
///    }
/// \endcode
#define TEST_METHOD(methodName) \
    TAEF_TEST_METHOD(methodName) TAEF_TERMINATOR

// Do not use TAEF_TEST_METHOD_METADATA_START/END directly. Instead use
// BEGIN_TEST_METHOD/END_TEST_METHOD at the class scope or
// BEGIN_TEST_METHOD_PROPERTIES/END_TEST_METHOD_PROPERTIES inside the test method.
#define TAEF_TEST_METHOD_METADATA_START \
    TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$h_TMM") \
    static TAEF_CONST_TESTDATA ::WEX::TestPropertyMetadata s_Metadata[] = { \
        { ::WEX::TAEF_Identifier::TestMethodMetadata, L"Name", TAEF_QUALIFIED_METHOD_NAME}, \

#define TAEF_TEST_METHOD_METADATA_END \
        {::WEX::TAEF_Identifier::TestMethodMetadata, nullptr, nullptr}}; \
    return s_Metadata;

// TAEF_BEGIN_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE and TAEF_END_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE
// are not intended to be used directly by tests, but some tests do use it; so we preserve it for
// compatibility. Use BEGIN_TEST_METHOD/END_TEST_METHOD instead of this.
#if defined(TAEF_USE_INLINE_STATICS)
# define TAEF_BEGIN_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE(methodName) \
    struct methodName##TAEF_MetadataStruct \
    { \
        inline static int s_doNotDiscard_##methodName##_GetTestMethodMetadata = ::WEX::Private::DoNotDiscard([]() \
        { \
            TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
            TAEF_TEST_METHOD_METADATA_START \

# define TAEF_END_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE() \
            TAEF_TEST_METHOD_METADATA_END \
        }()); \
    };
#else
# define TAEF_BEGIN_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE(methodName) \
    static TAEF_CPP_EXPORT const ::WEX::TestPropertyMetadata* methodName ## _GetTestMethodMetadata() \
    { \
        TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
        TAEF_TEST_METHOD_METADATA_START \

# define TAEF_END_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE() \
        TAEF_TEST_METHOD_METADATA_END \
    }

#endif

/// <summary>
///  Macro for declaring a test method and associating it with metadata
/// </summary>
///  Example:
/// \code
///      class TestFeatureClass
///      {
///          BEGIN_TEST_METHOD(TestFindNext)
///              TEST_METHOD_PROPERTY(L"Priority", L"2")
///          END_TEST_METHOD()
///      }
/// \endcode
# define BEGIN_TEST_METHOD(methodName) \
    TEST_METHOD(methodName); \
    TAEF_BEGIN_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE(methodName)

/// <summary>
///  Macro that ends test method declaration.
///  Must be used with BEGIN_TEST_METHOD()
/// </summary>
# define END_TEST_METHOD() \
    TAEF_END_TEST_METHOD_PROPERTIES_IN_CLASS_SCOPE()

/// <summary>
///  Macro for declaring metadata within inline test methods.
/// </summary>
///  Example:
/// \code
///      class TestFeatureClass
///      {
///          TEST_METHOD(TestFindNext)
///          {
///              BEGIN_TEST_METHOD_PROPERTIES()
///                  TEST_METHOD_PROPERTY(L"Priority", L"2")
///              END_TEST_METHOD_PROPERTIES()
///
///              // Your test code
///          }
///      }
/// \endcode
#define BEGIN_TEST_METHOD_PROPERTIES() \
    TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
    struct TAEF_TestMethodProperties \
    { \
        static const ::WEX::TestPropertyMetadata* TAEF_GetTestMethodMetadata() \
        { \
            TAEF_TEST_METHOD_METADATA_START

/// <summary>
///  Macro that ends inline test method property declaration.
///  Must be used with BEGIN_TEST_METHOD_PROPERTIES()
/// </summary>
#define END_TEST_METHOD_PROPERTIES() \
            TAEF_TEST_METHOD_METADATA_END \
        } \
    }; \
    ::WEX::Private::DoNotDiscard(TAEF_TestMethodProperties::TAEF_GetTestMethodMetadata());

/// <summary>
///  Macro for adding a piece of metadata to a test method.
///  Must be used with BEGIN_TEST_METHOD() / END_TEST_METHOD() or within BEGIN_TEST_METHOD_PROPERTIES() / END_TEST_METHOD_PROPERTIES()
/// </summary>
#define TEST_METHOD_PROPERTY(propertyName, propertyValue) \
                    {::WEX::TAEF_Identifier::TestMethodMetadata, propertyName, propertyValue},

/// <summary>
///  Macro for defining module properties
/// </summary>
///  Example:
/// \code
///      BEGIN_MODULE()
///          MODULE_PROPERTY(L"Area", L"Desktop Shell")
///          MODULE_PROPERTY(L"SubArea", L"Navigation")
///          MODULE_PROPERTY(L"Component", L"Start Menu")
///      END_MODULE()
/// \endcode
#define BEGIN_MODULE() \
    TAEF_STATIC_IF_CALLED_BY_TAEF_COMPILER_PIN TAEF_CPP_EXPORT const ::WEX::TestPropertyMetadata* TAEF_GetModuleMetadata() \
    { \
        TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$f_MM") \
        static TAEF_CONST_TESTDATA ::WEX::TestPropertyMetadata s_Metadata[] = { \
            {::WEX::TAEF_Identifier::ModuleMetadata,  L"Name", L"___##TestFile##___"},
// BEGIN_MODULE/END_MODULE is intended to be used outside of a class. However, some tests took
// advantage of the fact that it happened to work when placed within a class. To prevent build
// breaks for those tests, we now use static on the TAEF_GetModuleMetadata function when it is
// called within the TAEF_COMPILER_PIN macro. This avoids build breaks when BEGIN_MODULE/END_MODULE
// is used within a class. We don't use static when exporting though because that can cause build
// breaks for Clang since we would be exporting a function without external linkage.

/// <summary>
///  Macro for adding a piece of metadata (a property) to a test module propeties definition.
///  Must be used with BEGIN_MODULE() / END_MODULE()
/// </summary>
#define MODULE_PROPERTY(propertyName, propertyValue) \
            {::WEX::TAEF_Identifier::ModuleMetadata,  propertyName,  propertyValue},

/// <summary>
///  Macro that ends test module properties definition.
///  Must be used with BEGIN_MODULE()
/// </summary>
#define END_MODULE() \
            {::WEX::TAEF_Identifier::ModuleMetadata,  nullptr,  nullptr} \
        }; \
        return s_Metadata; \
    } \
    TAEF_COMPILER_PIN(TAEF_GetModuleMetadata)

/// <summary>
///  Macro for declaring a test class without associating it with any metadata
/// </summary>
///  Example:
/// \code
///      class TestFeatureClass
///      {
///          TEST_CLASS(TestFeatureClass)
///      }
/// \endcode
#define TEST_CLASS(className) \
    TAEF_DEFINE_CLASS_NAME(className); \
    typedef className TAEF_TestClassType; \
    static const uintptr_t TAEF_TestMethodIndexOffset = __COUNTER__; \
    friend struct ::WEX::TestClassFactory<className>; \
    typedef bool (className::*TAEF_MemberMaintFunc)(); \
    __pragma(warning(suppress:25007)) /* Disable warning that member method may be static */ \
    bool TAEF_DummyMaintFunc() { return true; } \
    static TAEF_CPP_EXPORT const ::WEX::TestClassInfo* TAEF_GetTestClassInfo() \
    { \
        /* this works similarly to WEX::Common::Conversion<T, U> checking if an expression (&_DummyMaintFunc) \
         * is convertible to the MemberMaintFunc type \
         */ \
        struct TaefClassNameTester { \
            /* these functions are only implemented because they're in an local class, so the compiler warns about missing definitions */ \
            static char TestType(TAEF_MemberMaintFunc) \
            { \
                return 'c'; \
            } \
            static TaefClassNameTester TestType(...) \
            { \
                return TaefClassNameTester(); \
            } \
            char member[2]; /* a two element array to ensure a different size from char returned by TestType(MemberMaintFunc) */ \
        }; \
        static_assert(sizeof(TaefClassNameTester::TestType(&TAEF_TestClassType::TAEF_DummyMaintFunc)) == 1, \
                TAEF_STRINGIZE(className) " is not the name of the current class"); \
 \
        TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
        TAEF_PUSH_IGNORE_WARNINGS \
        TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$b_TCI") \
        static TAEF_CONST_TESTDATA ::WEX::TestClassInfo s_ClassInfo =  \
            {::WEX::TAEF_Identifier::TestClassInfo, TAEF_QUALIFIED_METHOD_NAME, &::WEX::TestClassFactory<className>::CreateInstance, \
             &::WEX::TestClassFactory<className>::DestroyInstance }; \
        TAEF_POP_IGNORE_WARNINGS \
        return &s_ClassInfo; \
    } \
    TAEF_COMPILER_PIN(TAEF_GetTestClassInfo)

/// <summary>
///  Macro for declaring a test class and associating it with metadata
/// </summary>
///  Example:
/// \code
///  class FeatureTestClass
///  {
///      BEGIN_TEST_CLASS(FeatureTestClass)
///          TEST_CLASS_PROPERTY(L"BVT", L"TRUE")
///          TEST_CLASS_PROPERTY(L"STRESS", L"TRUE")
///      END_TEST_CLASS()
///  }
/// \endcode
#define BEGIN_TEST_CLASS(className) \
    TEST_CLASS(className) \
 \
    static TAEF_CPP_EXPORT const ::WEX::TestPropertyMetadata* TAEF_GetClassMetadata() \
    { \
        TAEF_DEFINE_QUALIFIED_METHOD_NAME(TAEF__WFUNCTION__); \
        TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$g_TCM") \
        static TAEF_CONST_TESTDATA ::WEX::TestPropertyMetadata s_Metadata[] =  { \
            { ::WEX::TAEF_Identifier::TestClassMetadata, L"Name", TAEF_QUALIFIED_METHOD_NAME}, \

/// <summary>
///  Macro for adding a piece of metadata to a test class declaration.
///  Must be used with BEGIN_TEST_CLASS() / END_TEST_CLASS()
/// </summary>
#define TEST_CLASS_PROPERTY(propertyName, propertyValue) \
            {::WEX::TAEF_Identifier::TestClassMetadata, propertyName, propertyValue },

/// <summary>
///  Macro that ends test class declaration.
///  Must be used with BEGIN_TEST_CLASS()
/// </summary>
#define END_TEST_CLASS() \
            {::WEX::TAEF_Identifier::TestClassMetadata, nullptr, nullptr} \
        }; \
        return s_Metadata; \
    } \
    TAEF_COMPILER_PIN(TAEF_GetClassMetadata)

#define TAEF_MODULE_FIXTURE_INFO_IMPL(methodName, moduleFixtureType) \
    struct TAEF_ModuleFixtureInvoker \
    { \
        static HRESULT __cdecl TAEF_Invoke() \
        { \
            return ::WEX::SafeInvoke(methodName); \
        } \
    }; \
 \
    TAEF_PUSH_IGNORE_WARNINGS \
    TAEF_PIN_FUNCTION_SYMBOL TAEF_ATTRIBUTE_SECTION("testdata$d_TGFI") \
    static TAEF_CONST_TESTDATA ::WEX::TestGlobalFunctionInfo s_moduleFixtureInfo = \
        {moduleFixtureType, L#methodName, &TAEF_ModuleFixtureInvoker::TAEF_Invoke }; \
    TAEF_POP_IGNORE_WARNINGS \
    return &s_moduleFixtureInfo;

#define TAEF_FIXTURE_IMPL(methodName) \
    __pragma(warning(suppress:25007)) /* Disable warning that member method may be static */ \
    bool methodName()

#ifdef INLINE_TEST_METHOD_MARKUP
#define TAEF_DEFINE_FIXTURE(methodName) \
    TAEF_FIXTURE_IMPL(methodName)
#else
#define TAEF_DEFINE_FIXTURE(methodName)
#endif

#ifdef INLINE_TEST_METHOD_MARKUP
#define TAEF_DECLARE_FIXTURE(methodName)
#else
#define TAEF_DECLARE_FIXTURE(methodName) \
    __if_not_exists(methodName) \
    { \
        TAEF_FIXTURE_IMPL(methodName); \
    }
#endif

#define TAEF_DECLARE_OR_DEFINE_FIXTURE(methodName) \
    TAEF_DECLARE_FIXTURE(methodName) \
    TAEF_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macro for defining global Setup function
/// </summary>
///  The Setup function runs before any test is executed
///  Example:
/// \code
///
///  MODULE_SETUP(AddRegistrySettings) //AddRegistrySettings function adds necessary reg settings for all tests
///
/// \endcode
#define MODULE_SETUP(methodName) \
    TAEF_FIXTURE_IMPL(methodName); \
    TAEF_CPP_EXPORT const ::WEX::TestGlobalFunctionInfo* YOU_CAN_ONLY_DESIGNATE_ONE_FUNCTION_TO_BE_A_MODULE_SETUP_FUNCTION() \
    { \
        TAEF_MODULE_FIXTURE_INFO_IMPL(methodName, ::WEX::TAEF_Identifier::ModuleSetup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_FUNCTION_TO_BE_A_MODULE_SETUP_FUNCTION) \
    TAEF_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macro for defining global Teardown function
/// </summary>
///  The Teardown function runs after all the test are executed
///  Example:
/// \code
///
///  MODULE_CLEANUP(CleanupRegistrySettings) //CleanupRegistrySettings restores the registry after all tests
///
/// \endcode
#define MODULE_CLEANUP(methodName) \
    TAEF_FIXTURE_IMPL(methodName); \
    TAEF_CPP_EXPORT const ::WEX::TestGlobalFunctionInfo* YOU_CAN_ONLY_DESIGNATE_ONE_FUNCTION_TO_BE_A_MODULE_CLEANUP_FUNCTION() \
    { \
        TAEF_MODULE_FIXTURE_INFO_IMPL(methodName, ::WEX::TAEF_Identifier::ModuleCleanup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_FUNCTION_TO_BE_A_MODULE_CLEANUP_FUNCTION) \
    TAEF_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macro for test setup method declaration
/// </summary>
///  Test setup method gets called before every test method is called
///  Example:
/// \code
///      class NotepadTestClass
///      {
///          // Declare CopyDownTestFiles class method and designate this method to be a test setup method
///          TEST_METHOD_SETUP(CopyDownTestFiles);
///      }
/// \endcode
#define TEST_METHOD_SETUP(methodName) \
    static TAEF_CPP_EXPORT const ::WEX::FixtureMethodInfo* YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_METHOD_SETUP_METHOD() \
    { \
        TAEF_REGISTER_FIXTURE_METHOD(methodName, ::WEX::TAEF_Identifier::TestMethodSetup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_METHOD_SETUP_METHOD) \
    TAEF_DECLARE_OR_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macros for test cleanup method declaration
/// </summary>
///  Test cleanup method gets called after every test method is called
///  Example:
/// \code
///      class NotepadTestClass
///      {
///          // Declare DeleteCopiedTestFiles class method and designate this method to be a test cleanup method
///          TEST_METHOD_CLEANUP(DeleteCopiedTestFiles);
///      }
/// \endcode
#define TEST_METHOD_CLEANUP(methodName) \
    static TAEF_CPP_EXPORT const ::WEX::FixtureMethodInfo* YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_METHOD_CLEANUP_METHOD() \
    { \
        TAEF_REGISTER_FIXTURE_METHOD(methodName, ::WEX::TAEF_Identifier::TestMethodCleanup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_METHOD_CLEANUP_METHOD) \
    TAEF_DECLARE_OR_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macro for test class setup method declaration
/// </summary>
///  Class setup method gets called before the first method in the class is called (after the class constructor)
///  Example:
/// \code
///      class NotepadTestClass
///      {
///          // Declare InstallNotepad class method and designate this method to be a class setup method
///          TEST_CLASS_CLEANUP(InstallNotepad);
///      }
/// \endcode
#define TEST_CLASS_SETUP(methodName) \
    static TAEF_CPP_EXPORT const ::WEX::FixtureMethodInfo* YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_CLASS_SETUP_METHOD() \
    { \
        TAEF_REGISTER_FIXTURE_METHOD(methodName, ::WEX::TAEF_Identifier::TestClassSetup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_CLASS_SETUP_METHOD) \
    TAEF_DECLARE_OR_DEFINE_FIXTURE(methodName)

/// <summary>
///  Macro for test class cleanup method declaration
/// </summary>
///  Class cleanup method gets called after the last method in the class is called (before the destructor)
///  Example:
/// \code
///      class NotepadTestClass
///      {
///          // Declare UninstallNotepad class method and designate this method to be a class cleanup method
///          TEST_CLASS_CLEANUP(UninstallNotepad);
///      }
/// \endcode
#define TEST_CLASS_CLEANUP(methodName) \
    static TAEF_CPP_EXPORT const ::WEX::FixtureMethodInfo* YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_CLASS_CLEANUP_METHOD() \
    { \
        TAEF_REGISTER_FIXTURE_METHOD(methodName, ::WEX::TAEF_Identifier::TestClassCleanup) \
    } \
    TAEF_COMPILER_PIN(YOU_CAN_ONLY_DESIGNATE_ONE_CLASS_METHOD_TO_BE_A_TEST_CLASS_CLEANUP_METHOD) \
    TAEF_DECLARE_OR_DEFINE_FIXTURE(methodName)

/// \internal
/// <summary>
///  Current framework version information
/// </summary>
extern "C" TAEF_ATTRIBUTE_SECTION("testdata$a_TDH") __declspec(selectany) ::WEX::TestVersionInfo s_versionInfo =
    {TAEF_HEADER_SIGNATURE, TAEF_ABI_VERSION, "WexTestClass.h", ::WEX::Private::StringManager::Deallocate};

#if defined(_M_IX86)
#pragma comment(linker, "/include:_s_versionInfo")
#else
#pragma comment(linker, "/include:s_versionInfo")
#endif

#pragma pop_macro("Assert")
#pragma pop_macro("_Outptr_")
#pragma pop_macro("_Outptr_result_maybenull_z_")
#pragma pop_macro("_Ret_maybenull_z_")
#pragma pop_macro("_In_reads_z_")
#pragma pop_macro("_In_")
#pragma pop_macro("_Post_invalid_")
#pragma warning(pop)
