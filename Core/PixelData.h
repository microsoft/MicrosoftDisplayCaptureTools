#pragma once
#include "Framework.PixelData.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
struct PixelData : PixelDataT<PixelData>
{
    PixelData() = default;

    winrt::Windows::Storage::Streams::IBuffer Pixels();
    void Pixels(winrt::Windows::Storage::Streams::IBuffer const& value);
    winrt::Windows::Graphics::SizeInt32 Resolution();
    void Resolution(winrt::Windows::Graphics::SizeInt32 const& value);
    winrt::MicrosoftDisplayCaptureTools::Framework::PixelDataDescription FormatDescription();
    void FormatDescription(winrt::MicrosoftDisplayCaptureTools::Framework::PixelDataDescription const& value);
    winrt::Windows::Graphics::Imaging::SoftwareBitmap GetRenderableApproximation();
    com_array<uint8_t> GetSpecificPixel(uint32_t x, uint32_t y);
    winrt::MicrosoftDisplayCaptureTools::Framework::IPixelDataExtension GetImageDelta(winrt::MicrosoftDisplayCaptureTools::Framework::IPixelData const& other);
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation {
struct PixelData : PixelDataT<PixelData, implementation::PixelData>
{
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
