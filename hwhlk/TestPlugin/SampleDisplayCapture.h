#pragma once

namespace winrt::CaptureCard::implementation
{
    struct SampleDisplayCapture : implements<SampleDisplayCapture, IDisplayCapture>
    {
        SampleDisplayCapture() = default;

        void CompareCaptureToReference(DisplayStateReference::IStaticReference reference);
        void SaveCaptureToDisk(hstring path);
    };
}