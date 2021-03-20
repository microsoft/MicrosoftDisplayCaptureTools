#include "pch.h"
#include "winrt\SamplePlugin.h"

#include <wincodec.h>
#include "Visualizer.h"
#include "FormatHelper.h"

using namespace winrt;
using namespace SamplePlugin;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
};

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
    errno_t err = _wsplitpath_s(modulePath, drive, _MAX_DRIVE, dir, _MAX_DIR, filename, _MAX_FNAME, ext, _MAX_EXT);

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
    std::wstring_view name{ WindowsGetStringRawBuffer(classId_hstring, nullptr), WindowsGetStringLen(classId_hstring) };
    HMODULE library{ nullptr };

    std::wstring dllPath = L"C:\\Users\\daspr\\source\\repos\\DisplayHardwareHLK\\Plugins\\SoftGPU plugin\\x64\\Release\\SoftGPU plugin.dll";// GetModulePath() + L"SoftGPU.dll";

    if (starts_with(name, L"SamplePlugin."))
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

int32_t __stdcall WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept
{
    return WINRT_RoGetActivationFactory((HSTRING)classId, (GUID)iid, factory);
}


template<typename T>
struct ProcessHeapArray
{
    ProcessHeapArray(UINT32 count) : m_count(count)
    {
        m_arr = (T*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(T) * count);
    }

    ~ProcessHeapArray()
    {
        HeapFree(GetProcessHeap(), NULL, m_arr);
    }

    T& operator[] (UINT index)
    {
        if (m_count <= index) winrt::throw_hresult(hresult_out_of_bounds().code());
        return m_arr[index];
    }

    winrt::array_view<T>  GetArrayView()
    {
        return winrt::array_view<T>(&m_arr[0], &m_arr[m_count - 1] + 1);
    }

private:
    T* m_arr;
    const UINT32 m_count;
};


int main()
{
    init_apartment();

    std::shared_ptr<PeekWindow> window = PeekWindow::GetOrCreateVisualizer();

    window->InitializeWindow();

    winrt_activation_handler = WINRT_RoGetActivationFactory;

    SamplePlugin::GraphicsCapturePlugin plugin;

    {
        auto factory = winrt::get_activation_factory<SamplePlugin::GraphicsCapturePlugin>();
        plugin = factory.ActivateInstance<SamplePlugin::GraphicsCapturePlugin>();
    }

    UINT32 inputDeviceCount = plugin.GetCaptureInputCount();

    auto inputDeviceIds = ProcessHeapArray<UINT32>(inputDeviceCount);
    plugin.GetCaptureInputDisplayIds(inputDeviceIds.GetArrayView());

    winrt::com_ptr<IWICImagingFactory> wicFactory = nullptr;
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)wicFactory.put());
    
    std::vector<SamplePlugin::GraphicsCaptureDeviceInput> inputs;
    {
        auto factory = winrt::get_activation_factory<SamplePlugin::GraphicsCaptureDeviceInput>();

        for (auto inputId : inputDeviceIds.GetArrayView())
        {
            plugin.InitializeCaptureInput(inputId);

            auto input = factory.ActivateInstance<SamplePlugin::GraphicsCaptureDeviceInput>();
            inputs.push_back(input);

            input.InitializeFromId(inputId, plugin);
        }
    }

    while (!window->CanExit())
    {
        for (auto input : inputs)
        {
            auto caps = input.GetCaptureCaps();

            auto frame = input.CaptureFrame();
            auto characteristics = frame.GetFrameCharacteristics();

            if (characteristics.byteCount == 0) continue;
            
            auto pixels = ProcessHeapArray<uint8_t>(characteristics.byteCount);
            frame.GetFramePixels(pixels.GetArrayView());

            // Create a standard format bitmap for display
            winrt::com_ptr<IWICBitmap> wicBitmap = nullptr;
            winrt::com_ptr<IWICFormatConverter> wicFormatConverter = nullptr;

            PixelDataFormat format(characteristics.format);

            wicFactory->CreateBitmapFromMemory(
                characteristics.width,
                characteristics.height,
                format.GetWICType(),
                characteristics.stride,
                characteristics.byteCount,
                pixels.GetArrayView().data(),
                wicBitmap.put());

            wicFactory->CreateFormatConverter(
                wicFormatConverter.put());

            wicFormatConverter->Initialize(
                wicBitmap.as<IWICBitmapSource>().get(),
                GUID_WICPixelFormat32bppPRGBA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeCustom);


            BOOL test = FALSE;
            wicFormatConverter->CanConvert(format.GetWICType(), GUID_WICPixelFormat32bppRGBA, &test);

            window->UpdateDisplay(wicFormatConverter.as<IWICBitmapSource>().get());

            Sleep(16);
        }
    }

    window.reset();
}