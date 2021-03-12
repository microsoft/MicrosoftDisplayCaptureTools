#pragma once
#include "CaptureCard.DisplayCapture.g.h"

namespace winrt::CaptureCard::implementation
{
    struct DisplayCapture : DisplayCaptureT<DisplayCapture>
    {
        DisplayCapture() = default;

        void CompareCaptureToReference(DisplayStateReference::StaticReference const& reference);
        void SaveCaptureToDisk(hstring const& path);
    };
}
