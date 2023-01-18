#include "pch.h"
#include "PixelData.h"
#include "Framework.PixelData.g.cpp"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
winrt::Windows::Storage::Streams::IBuffer PixelData::Pixels()
{
    throw hresult_not_implemented();
}
void PixelData::Pixels(winrt::Windows::Storage::Streams::IBuffer const& value)
{
    throw hresult_not_implemented();
}
winrt::Windows::Graphics::SizeInt32 PixelData::Resolution()
{
    throw hresult_not_implemented();
}
void PixelData::Resolution(winrt::Windows::Graphics::SizeInt32 const& value)
{
    throw hresult_not_implemented();
}
winrt::MicrosoftDisplayCaptureTools::Framework::PixelDataDescription PixelData::FormatDescription()
{
    throw hresult_not_implemented();
}
void PixelData::FormatDescription(winrt::MicrosoftDisplayCaptureTools::Framework::PixelDataDescription const& value)
{
    throw hresult_not_implemented();
}
winrt::Windows::Graphics::Imaging::SoftwareBitmap PixelData::GetRenderableApproximation()
{
    throw hresult_not_implemented();
}
com_array<uint8_t> PixelData::GetSpecificPixel(uint32_t x, uint32_t y)
{
    throw hresult_not_implemented();
}
winrt::MicrosoftDisplayCaptureTools::Framework::IPixelDataExtension PixelData::GetImageDelta(
    winrt::MicrosoftDisplayCaptureTools::Framework::IPixelData const& other)
{
    throw hresult_not_implemented();
}
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
