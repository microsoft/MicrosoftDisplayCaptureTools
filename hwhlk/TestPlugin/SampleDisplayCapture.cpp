#include "pch.h"
// add the include for IMicrosoftCaptureBoard
#include "SampleDisplayCapture.h"
#include "Singleton.h"
#include "CaptureCard.Controller.g.h"

#include <winrt/Windows.Security.Cryptography.h>
#include <winrt/Windows.Security.Cryptography.Core.h>

#include <wincodec.h>
#include <MemoryBuffer.h>


using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt::CaptureCard::implementation
{
    SampleDisplayCapture::SampleDisplayCapture(std::shared_ptr<IMicrosoftCaptureBoard> captureBoard) :
        m_testDataFolder(LoadFolder(L"TestData")),
        m_mismatchFolder(LoadFolder(L"Mismatches")),
        m_captureBoard(captureBoard)
    {
        
    }

    void SampleDisplayCapture::CompareCaptureToReference(hstring name, DisplayStateReference::IStaticReference reference)
    {
        // 
        // This software-only plugin doesn't compare against a hardware-captured image, instead it compares against 
        // an image loaded from disk.
        //

        Log::Comment(L"Comparison For: " + String(name.c_str()));

        // Hash the name string to compute the on-disk file name.
        auto hashedName = ComputeHashedFileName(name);

        // Retrieve serialized data from the current reference image
        auto referenceMetadata = reference.GetSerializedMetadata();

        auto frame = reference.GetFrame();

        boolean comparisonSucceeded = false;

        try
        {
            auto comparison = LoadComparisonImage(hashedName);

            // TODO: Actually do the comparison


            comparisonSucceeded = true;
        }
        catch (winrt::hresult_error const& ex)
        {
            if (ex.code() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                Log::Comment(L"Comparison image could not be located");
            }
            else throw;
        }

        if (!comparisonSucceeded)
        {
            SaveMismatchedImage(hashedName, frame);

            Log::Error(L"Comparison to image-on-disk failed! Generated image saved");
        }
    }

    // TODO: re-implement shader-based comparisons once I fix the hardware systems

    void SampleDisplayCapture::SaveCaptureToDisk(hstring path)
    {
    }

    hstring SampleDisplayCapture::ComputeHashedFileName(hstring testName)
    {
        auto hash = winrt::Windows::Security::Cryptography::Core::HashAlgorithmProvider::OpenAlgorithm(
            winrt::Windows::Security::Cryptography::Core::HashAlgorithmNames::Sha1());

        auto stringBuffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::ConvertStringToBinary(
            testName, winrt::Windows::Security::Cryptography::BinaryStringEncoding::Utf16LE);

        auto buffer = hash.HashData(stringBuffer);
        auto hashedName = winrt::Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(buffer);

        hashedName = hashedName + L".bmp";

        return hashedName;
    }

    winrt::Windows::Graphics::Imaging::SoftwareBitmap SampleDisplayCapture::LoadComparisonImage(hstring name)
    {
        auto file = m_testDataFolder.GetFileAsync(name).get();
        if (!file) winrt::throw_hresult(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

        auto read = file.OpenReadAsync().get();
        auto decoder = winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(read).get();
        auto bitmap = decoder.GetSoftwareBitmapAsync().get();

        return bitmap;
    }
    

    void SampleDisplayCapture::SaveMemoryToBitmap (hstring name)
    {
        auto file = m_testDataFolder.GetFileAsync(name).get();
        UINT16 dataSize = 32;	
        auto readBuffer = m_captureBoard->FpgaRead(0x20, dataSize); //reading 1080 words
        auto read = readBuffer.data(); 
        int bWidth= 644; 
        int bHeight= 300;
        int bitsPerPixel =32;

        winrt::com_ptr<IWICImagingFactory> wicFactory;

        // Create the COM imaging factory
        winrt::check_hresult(CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(wicFactory.put())
        ));

        winrt::com_ptr<IWICBitmap> embeddedBitmap;
        winrt::check_hresult(wicFactory->CreateBitmapFromMemory(  
            bWidth,
            bHeight,
            GUID_WICPixelFormat32bppRGB, 
            (bWidth*bitsPerPixel+7)/8, 
            bHeight*bWidth,
            read,
            embeddedBitmap.put()));

    }

    void SampleDisplayCapture::SaveMismatchedImage(hstring name, winrt::Windows::Graphics::Imaging::SoftwareBitmap bitmap)
    {
        auto file = m_mismatchFolder.CreateFileAsync(name, winrt::Windows::Storage::CreationCollisionOption::FailIfExists).get();
        auto stream = file.OpenAsync(winrt::Windows::Storage::FileAccessMode::ReadWrite).get();
        auto encoder = winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(winrt::Windows::Graphics::Imaging::BitmapEncoder::BmpEncoderId(), stream).get();
        encoder.SetSoftwareBitmap(bitmap);

        encoder.FlushAsync().get();
    }

    winrt::Windows::Storage::StorageFolder SampleDisplayCapture::LoadFolder(hstring subFolder)
    {
        DWORD directoryLength = MAX_PATH;
        wchar_t directoryBuffer[MAX_PATH];

        auto err = GetCurrentDirectoryW(directoryLength, directoryBuffer);

        if (err == 0 || err > MAX_PATH) winrt::check_win32(GetLastError());

        hstring directoryName(directoryBuffer);

        auto testFolder = winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(directoryName).get();
        auto folder = testFolder.CreateFolderAsync(subFolder, winrt::Windows::Storage::CreationCollisionOption::OpenIfExists).get();

        return folder;
    }
}