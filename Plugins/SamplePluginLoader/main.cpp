#include "pch.h"
#include <iostream>
#include "winrt\SamplePlugin.h"

extern "C"
{
    HRESULT __stdcall OS_RoGetActivationFactory(HSTRING classId, GUID const& iid, void** factory) noexcept;
}

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif

bool starts_with(std::wstring_view value, std::wstring_view match) noexcept
{
    return 0 == value.compare(0, match.size(), match);
}

HRESULT __stdcall WINRT_RoGetActivationFactory(HSTRING classId, GUID const& iid, void** factory) noexcept
{
    *factory = nullptr;
    std::wstring_view name{ WindowsGetStringRawBuffer(classId, nullptr), WindowsGetStringLen(classId) };
    HMODULE library{ nullptr };

    if (starts_with(name, L"SamplePlugin."))
    {
        library = LoadLibraryW(L"SamplePlugin.dll");
    }
    else
    {
        return OS_RoGetActivationFactory(classId, iid, factory);
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
    HRESULT const hr = call(classId, activation_factory.put_void());

    if (FAILED(hr))
    {
        WINRT_VERIFY(FreeLibrary(library));
        return hr;
    }

    //winrt::guid iid_winrt(
    //    iid.Data1, iid.Data2, iid.Data3, 
    //    { iid.Data4[0], iid.Data4[1], iid.Data4[2], iid.Data4[3], iid.Data4[4], iid.Data4[5], iid.Data4[6], iid.Data4[7] });
    //    
    //if (iid_winrt != winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
    //{
    //    return activation_factory->QueryInterface(iid_winrt, factory);
    //}

    *factory = activation_factory.detach();
    return S_OK;
}

using namespace winrt;
using namespace Windows::Foundation;
using namespace SamplePlugin;

int main()
{
    init_apartment();

    auto factory = winrt::get_activation_factory<SamplePlugin::GraphicsCapturePlugin>();
    SamplePlugin::GraphicsCapturePlugin plugin = factory.ActivateInstance<SamplePlugin::GraphicsCapturePlugin>();

    UINT32 inputDeviceCount = plugin.GetCaptureInputCount();
    std::cout << "Supported plugin count: " << inputDeviceCount << "\n";

    UINT32* inputDeviceIds = (UINT32*)HeapAlloc(GetProcessHeap(), NULL, inputDeviceCount * sizeof(UINT32));
    winrt::array_view<UINT32> inputDeviceIdArray(&inputDeviceIds[0], &inputDeviceIds[inputDeviceCount]);
    plugin.GetCaptureInputDisplayIds(inputDeviceIdArray);
    std::cout << "Input IDs: ";
    for (auto id : inputDeviceIdArray) std::cout << id << ", ";
    std::cout << "\n";
    HeapFree(GetProcessHeap(), NULL, inputDeviceIds);
}
