#include "pch.h"
#include "FrameData.h"
#include "Framework.FrameData.g.cpp"
#include "Framework.FrameMetadata.g.cpp"

#include <string>
#include <format>

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Devices::Display::Core;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Graphics::Imaging;

    using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    winrt::Windows::Storage::Streams::IBuffer FrameData::Data()
    {
        return m_data;
    }
    void FrameData::Data(winrt::IBuffer const& data)
    {
        m_data = data;
    }
    winrt::SizeInt32 FrameData::Resolution()
    {
        return m_resolution;
    }
    void FrameData::Resolution(winrt::SizeInt32 const& resolution)
    {
        m_resolution = resolution;
    }
    winrt::FrameDataDescription FrameData::FormatDescription()
    {
        return m_description;
    }
    void FrameData::FormatDescription(winrt::FrameDataDescription const& description)
    {
        m_description = description;
    }
    com_array<winrt::FrameMetadata> FrameData::GetFrameMetadata()
    {
        return com_array<winrt::FrameMetadata>(m_metadataList);
    }
    void FrameData::AddMetadata(winrt::FrameMetadata const& data)
    {
        m_metadataList.push_back(data);
    }

    winrt::IAsyncOperation<winrt::SoftwareBitmap> FrameData::GetRenderableApproximationAsync()
    {
        co_await winrt::resume_background();

        try
        {
            SoftwareBitmap bitmap = SoftwareBitmap(
                BitmapPixelFormat::Rgba8, m_resolution.Width, m_resolution.Height, BitmapAlphaMode::Ignore);

            // if the pixel format is conveniently exactly what SoftwareBitmap wants, cool, just copy from the buffer
            if (m_description.PixelFormat == DirectXPixelFormat::R8G8B8A8UInt)
            {
                bitmap.CopyFromBuffer(m_data);
                co_return bitmap;
            }

            throw winrt::hresult_not_implemented();
        }
        catch (...)
        {
            co_return nullptr;
        }
    }
    winrt::IFrameDataComparisons FrameData::GetImageDelta(winrt::IFrameData const& other)
    {
        // This method only functions if the 'other' provided is of the same underlying implementation
        // as this.
        auto otherFrameData = other.try_as<FrameData>();
        if (!otherFrameData)
        {
            m_logger.LogError(L"Multiple IFrameData implementations used, cannot generate a comparison with GetImageDelta.");
            throw hresult_invalid_argument();
        }

        throw hresult_not_implemented();
    }

    FrameMetadata::FrameMetadata(hstring const& Format, winrt::IBuffer const& Data) :
        m_format(Format), m_data(Data)
    {
    }
    winrt::Windows::Storage::Streams::IBuffer FrameMetadata::Data()
    {
        return m_data;
    }
    hstring FrameMetadata::Name()
    {
        return m_format;
    }
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
