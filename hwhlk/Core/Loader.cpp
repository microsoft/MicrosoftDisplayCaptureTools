#include "pch.h"

#include "Loader.h"

extern "C"
{
    HRESULT __stdcall OS_RoGetActivationFactory(HSTRING classId, GUID const& iid, void** factory) noexcept;
}

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif

thread_local BinaryLoader BinaryLoader::s_Loader;

bool starts_with(std::wstring_view value, std::wstring_view match) noexcept
{
    return 0 == value.compare(0, match.size(), match);
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::wstring GetModulePath()
{
    std::wstring val;
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW((HINSTANCE)&__ImageBase, modulePath, _countof(modulePath));
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t filename[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    (void)_wsplitpath_s(modulePath, drive, _MAX_DRIVE, dir, _MAX_DIR, filename, _MAX_FNAME, ext, _MAX_EXT);

    val = drive;
    val += dir;

    return val;
}

HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId_hstring, GUID const& iid, void** factory) noexcept
{
    *factory = nullptr;
#ifdef WINDOWSAI_RAZZLE_BUILD
    // For razzle build, always look for the system dll for testing
    return OS_RoGetActivationFactory(classId_hstring, iid, factory);
#else

    winrt::hstring classId;
    winrt::attach_abi(classId, classId_hstring);

    HMODULE library{ nullptr };

    auto& loaderPath = BinaryLoader::GetOrCreate();
    auto nextBinaryPath = loaderPath.GetPath();

    std::wstring dllPath = GetModulePath() + std::wstring(nextBinaryPath);

    if (!nextBinaryPath.empty())
    {
        const wchar_t* libPath = dllPath.c_str();
        library = LoadLibraryW(libPath);
    }
    else
    {
        return OS_RoGetActivationFactory(classId_hstring, iid, factory);
    }

    if (!library)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    using DllGetActivationFactory = HRESULT __stdcall(HSTRING classId, void** factory);
    auto call = reinterpret_cast<DllGetActivationFactory*>(GetProcAddress(library, "DllGetActivationFactory"));

    if (!call)
    {
        HRESULT const hr = HRESULT_FROM_WIN32(GetLastError());
        WINRT_VERIFY(FreeLibrary(library));
        return hr;
    }

    winrt::com_ptr<winrt::Windows::Foundation::IActivationFactory> activation_factory;
    HRESULT const hr = call(classId_hstring, activation_factory.put_void());

    if (FAILED(hr))
    {
        WINRT_VERIFY(FreeLibrary(library));
        return hr;
    }

    if (winrt::guid(iid) != winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
    {
        return activation_factory->QueryInterface(iid, factory);
    }

    *factory = activation_factory.detach();
    return S_OK;
#endif
}
