#pragma once
#include <mutex>
#include <memory>
#include <string>

template <typename TInterface>
auto CreateImplementationFromPlugin(const std::wstring_view& dllName, const std::wstring_view& className)
{
    auto binaryLoader = BinaryLoader::SetPathInScope(dllName);

    winrt::Windows::Foundation::IActivationFactory factory;
    winrt::check_hresult(WINRT_RoGetActivationFactory(winrt::get_abi(winrt::hstring(className)), winrt::guid_of<decltype(factory)>(), winrt::put_abi(factory)));

    return factory.ActivateInstance<TInterface>();
}

class BinaryLoader
{
private:
    std::wstring m_path;

    // Store a thread-local singleton to prevent interference between threads
    thread_local static BinaryLoader s_Loader;

public:
    BinaryLoader() : m_path(L"") {}
    BinaryLoader(const BinaryLoader&) = delete;

    static BinaryLoader& GetOrCreate()
    {
        return s_Loader;
    }

    class BinaryLoaderCleanup
    {
    public:
        BinaryLoaderCleanup(const std::wstring_view& path) : m_path(path)
        {
        }

        BinaryLoaderCleanup(BinaryLoaderCleanup&& move) noexcept : m_path(std::move(move.m_path))
        {
        }

        ~BinaryLoaderCleanup()
        {
            BinaryLoader::s_Loader.SetPath(m_path);
        }

    private:
        std::wstring m_path;
    };

    static BinaryLoaderCleanup SetPathInScope(const std::wstring_view& path)
    {
        auto& loader = s_Loader;
        auto origPath = loader.GetPath();
        loader.SetPath(path);

        return BinaryLoaderCleanup(origPath);
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