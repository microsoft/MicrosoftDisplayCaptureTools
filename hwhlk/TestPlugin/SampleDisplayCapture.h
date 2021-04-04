#pragma once

namespace winrt::CaptureCard::implementation
{
    struct SampleDisplayCapture : implements<SampleDisplayCapture, IDisplayCapture>
    {
        SampleDisplayCapture();

        void CompareCaptureToReference(hstring name, DisplayStateReference::IStaticReference reference);
        void SaveCaptureToDisk(hstring path);

    private:
        hstring ComputeHashedFileName(hstring testName);
        winrt::Windows::Graphics::Imaging::SoftwareBitmap LoadComparisonImage(hstring name);
        void SaveMismatchedImage(hstring name, winrt::Windows::Graphics::Imaging::SoftwareBitmap bitmap);
        static winrt::Windows::Storage::StorageFolder LoadFolder(hstring subFolder);

    private:
        // The folder in which to look for test data
        winrt::Windows::Storage::StorageFolder m_testDataFolder, m_mismatchFolder;
    };
}