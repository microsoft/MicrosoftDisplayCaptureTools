#pragma once
#include "pch.h"
#include "BinaryLoader.h"
#include <filesystem>

extern "C"
{
    HRESULT __stdcall OS_RoGetActivationFactory(HSTRING classId, GUID const& iid, void** factory) noexcept;
}

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace winrt::MicrosoftDisplayCaptureTools::Libraries
{
    int32_t __stdcall GetWinRTActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept;

    BinaryLoader::~BinaryLoader() {}
    BinaryLoaderCleanup::~BinaryLoaderCleanup() {}

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

    class CBinaryLoader : BinaryLoader
    {
    private:
        std::wstring m_path;

        // Store a thread-local singleton to prevent interference between threads
        thread_local static CBinaryLoader s_Loader;

    public:
        class CBinaryLoaderCleanup : public BinaryLoaderCleanup
        {
        private:
            std::wstring m_path;

        public:
            CBinaryLoaderCleanup(std::wstring_view path) : m_path(path)
            {
                winrt_activation_handler = GetWinRTActivationFactory;
            }
            CBinaryLoaderCleanup(CBinaryLoaderCleanup&& move) noexcept : m_path(std::move(move.m_path))
            {
                winrt_activation_handler = GetWinRTActivationFactory;
            }
            ~CBinaryLoaderCleanup()
            {
                CBinaryLoader::s_Loader.SetPath(m_path);
                winrt_activation_handler = nullptr;
            }
        };

        CBinaryLoader() : m_path(L"") {}
        CBinaryLoader(const CBinaryLoader&) = delete;

        static CBinaryLoader& GetOrCreate()
        {
            return s_Loader;
        }

        static std::unique_ptr<BinaryLoaderCleanup> SetPathInScope(const std::wstring_view& path)
        {
            auto& loader = s_Loader;
            auto origPath = loader.GetPath();
            loader.SetPath(path);

            return std::make_unique<CBinaryLoaderCleanup>(origPath);

        }

        void SetPath(const std::wstring_view& path)
        {
            m_path = path;
        }

        const std::wstring_view GetPath()
        {
            return m_path;
        }
    };

    thread_local CBinaryLoader CBinaryLoader::s_Loader;

    bool starts_with(std::wstring_view value, std::wstring_view match) noexcept
    {
        return 0 == value.compare(0, match.size(), match);
    }

    HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId_hstring, GUID const& iid, void** factory) noexcept
    {
        *factory = nullptr;
        winrt::hstring nameHString;
        winrt::copy_from_abi(nameHString, classId_hstring);
        std::wstring_view name{ nameHString.c_str() };
        HMODULE library{ nullptr };
        const winrt::guid classGuid = reinterpret_cast<const winrt::guid&>(iid);

        // Get the current path override
        auto& loaderPath = CBinaryLoader::GetOrCreate();
        const wchar_t* loadPath = loaderPath.GetPath().data();
        library = LoadLibraryExW(loadPath, nullptr, 0);

        if (!library)
        {
            // Failed to load the binary using the provided path as fully qualified. Attempt
            // again using the provided path as a relative path.
            auto cwd = std::filesystem::current_path();
            winrt::hstring cwdInclusivePath = winrt::hstring(cwd.c_str()) + L"\\" + loaderPath.GetPath();
            loadPath = cwdInclusivePath.data();
            library = LoadLibraryExW(loadPath, nullptr, 0);

            if (!library)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }
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

        if (classGuid != winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
        {
            return activation_factory->QueryInterface(classGuid, factory);
        }

        *factory = activation_factory.detach();
        return S_OK;
    }

    int32_t __stdcall GetWinRTActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept
    {
        return WINRT_RoGetActivationFactory((HSTRING)classId, reinterpret_cast<const GUID&>(iid), factory);
    }

    std::unique_ptr<BinaryLoaderCleanup> BinaryLoader::SetPathInScope(const std::wstring_view& path)
    {
        return CBinaryLoader::SetPathInScope(path);
    }
}