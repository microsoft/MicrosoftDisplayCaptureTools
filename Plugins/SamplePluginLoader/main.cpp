#include "pch.h"
#include "winrt\SamplePlugin.h"

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

winrt::guid CreateWinRTGuid(const GUID& guid)
{
    std::array<uint8_t, 8Ui64> data4 = { guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6] , guid.Data4[7] };
    return winrt::guid(guid.Data1, guid.Data2, guid.Data3, data4);
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

    winrt::guid iid_winrt = CreateWinRTGuid(iid);
    if (iid_winrt != winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
    {
        return activation_factory->QueryInterface(iid_winrt, factory);
    }

    *factory = activation_factory.detach();
    return S_OK;
}

using namespace winrt;
using namespace SamplePlugin;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

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

template<typename T>
struct pixelDataRGB
{
    T R, G, B;
};

template<typename T>
struct pixelDataRGBA
{
    T R, G, B, A;
};


int main()
{
    init_apartment();

    auto factory = winrt::get_activation_factory<SamplePlugin::GraphicsCapturePlugin>();
    SamplePlugin::GraphicsCapturePlugin plugin = factory.ActivateInstance<SamplePlugin::GraphicsCapturePlugin>();

    UINT32 inputDeviceCount = plugin.GetCaptureInputCount();
    std::cout << "Supported plugin count: " << inputDeviceCount << "\n";

    auto inputDeviceIds = ProcessHeapArray<UINT32>(inputDeviceCount);
    plugin.GetCaptureInputDisplayIds(inputDeviceIds.GetArrayView());

    for (auto id : inputDeviceIds.GetArrayView())
    {
        std::cout << "Id: " << id << "\n";
        auto input = GraphicsCaptureDeviceInput(id, plugin);
        auto caps = input.GetCaptureCaps();
        std::cout << "\t CanGetImage=" << (caps.getImage ? "true" : "false") << " tolerance=" << caps.tolerance << "\n";

        auto frame = input.CaptureFrame();
        auto characteristics = frame.GetFrameCharacteristics();
        std::cout << "\t pixel format=" << (uint32_t)characteristics.format << " dimensions=" << characteristics.width << "x" << characteristics.height << " bytecount=" << characteristics.byteCount << " stide=" << characteristics.stride << "\n";

        auto pixels = ProcessHeapArray<uint8_t>(characteristics.byteCount);
        frame.GetFramePixels(pixels.GetArrayView());

        auto bitmap = SoftwareBitmap((characteristics.format == PixelFormat::RGB8 ? BitmapPixelFormat::Rgba8 : BitmapPixelFormat::Rgba16), characteristics.width, characteristics.height);

        {
            uint8_t* bufferBytes{};
            uint32_t bufferSize{};

            auto bitmapBuffer = bitmap.LockBuffer(BitmapBufferAccessMode::Write);
            auto bitmapBufferRef = bitmapBuffer.CreateReference();
            auto bitmapBufferAccess = bitmapBufferRef.as< IMemoryBufferByteAccess >();
            bitmapBufferAccess->GetBuffer(&bufferBytes, &bufferSize);
            auto bufferLayout = bitmapBuffer.GetPlaneDescription(0);

            for (int y = 0; y < bufferLayout.Height; y++)
            {
                switch (characteristics.format)
                {
                case PixelFormat::RGB8:
                    {
                        pixelDataRGB<uint8_t>* srcPixelLine = reinterpret_cast<pixelDataRGB<uint8_t>*>(&pixels[characteristics.stride * y]);
                        pixelDataRGBA<uint8_t>* destPixelLine = reinterpret_cast<pixelDataRGBA<uint8_t>*>(&bufferBytes[bufferLayout.StartIndex + bufferLayout.Stride * y]);

                        for (int x = 0; x < bufferLayout.Width; x++)
                        {
                            destPixelLine[x] = { srcPixelLine[x].R, srcPixelLine[x].G, srcPixelLine[x].B, 0xFF };
                        }
                    }
                    break;
                case PixelFormat::RGB16:
                    {
                        pixelDataRGB<uint16_t>* srcPixelLine = reinterpret_cast<pixelDataRGB<uint16_t>*>(&pixels[characteristics.stride * y]);
                        pixelDataRGBA<uint16_t>* destPixelLine = reinterpret_cast<pixelDataRGBA<uint16_t>*>(&bufferBytes[bufferLayout.StartIndex + bufferLayout.Stride * y]);

                        for (int x = 0; x < bufferLayout.Width; x++)
                        {
                            destPixelLine[x] = { srcPixelLine[x].R, srcPixelLine[x].G, srcPixelLine[x].B, 0xFFFF };
                        }
                    }
                    break;
                default:
                    throw(hresult_not_implemented());
                }
            }
        }

        std::wstringstream filenameStream;
        filenameStream << L"OutputFile-" << id << L".jpg";
        std::wstring filename = filenameStream.str();

        auto folder = KnownFolders::GetFolderForUserAsync(nullptr /* current user */, KnownFolderId::PicturesLibrary).get();
        IStorageFile file;
        try 
        {
            file = folder.GetFileAsync(winrt::hstring(filename)).get();
        }
        catch (winrt::hresult_error const& err)
        {
            if (err.to_abi() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                file = folder.CreateFileAsync(winrt::hstring(filename)).get();
            }
            else
            {
                throw(err);
            }
        }

        //auto file = folder.CreateFileAsync(winrt::hstring(filename)).get();
        auto stream = file.OpenAsync(FileAccessMode::ReadWrite).get();
        auto encoder = BitmapEncoder::CreateAsync(BitmapEncoder::BmpEncoderId(), stream).get();
        encoder.SetSoftwareBitmap(bitmap);
        encoder.FlushAsync().get();
    }
}
