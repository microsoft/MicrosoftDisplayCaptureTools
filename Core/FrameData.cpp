#include "pch.h"
#include "FrameData.h"
#include "Framework.FrameData.g.cpp"

#include <string>
#include <format>

namespace winrt
{
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Graphics::Imaging;
}

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    winrt::Windows::Storage::Streams::IBuffer FrameData::Data()
    {
        return m_data;
    }
    void FrameData::Data(winrt::Windows::Storage::Streams::IBuffer const& data)
    {
        m_data = data;
    }
    winrt::Windows::Graphics::SizeInt32 FrameData::Resolution()
    {
        return m_resolution;
    }
    void FrameData::Resolution(winrt::Windows::Graphics::SizeInt32 const& resolution)
    {
        m_resolution = resolution;
    }
    winrt::MicrosoftDisplayCaptureTools::Framework::FrameDataDescription FrameData::FormatDescription()
    {
        return m_description;
    }
    void FrameData::FormatDescription(winrt::MicrosoftDisplayCaptureTools::Framework::FrameDataDescription const& description)
    {
        m_description = description;
    }


    winrt::SoftwareBitmap FrameData::GetRenderableApproximation()
    {
        SoftwareBitmap bitmap =
            SoftwareBitmap((BitmapPixelFormat)m_description.PixelEncoding, m_resolution.Width, m_resolution.Height, BitmapAlphaMode::Ignore);

        bitmap.CopyFromBuffer(m_data);

        return bitmap;
    }
    winrt::MicrosoftDisplayCaptureTools::Framework::IFrameDataComparisons FrameData::GetImageDelta(
        winrt::MicrosoftDisplayCaptureTools::Framework::IFrameData const& other)
    {
        throw hresult_not_implemented();
    }
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
