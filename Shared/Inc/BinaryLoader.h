#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::Libraries
{
    HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId_hstring, GUID const& iid, void** factory) noexcept;

    class BinaryLoaderCleanup;
    class BinaryLoader
    {
    public:
        virtual ~BinaryLoader() = 0;

        static std::unique_ptr<BinaryLoaderCleanup> SetPathInScope(const std::wstring_view& path);
    };

    class BinaryLoaderCleanup
    {
    public:
        virtual ~BinaryLoaderCleanup() = 0;
    };

    template <typename TInterface>
    auto LoadInterfaceFromPath(const std::wstring_view& dllName, const std::wstring_view& className)
    {
        auto binaryLoader = BinaryLoader::SetPathInScope(dllName);

        winrt::Windows::Foundation::IActivationFactory factory;
        winrt::check_hresult(WINRT_RoGetActivationFactory((HSTRING)winrt::get_abi(winrt::hstring(className)), winrt::guid_of<decltype(factory)>(), winrt::put_abi(factory)));

        return factory.ActivateInstance<TInterface>();
    }
}