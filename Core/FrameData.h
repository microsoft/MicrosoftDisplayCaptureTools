#pragma once
#include "Framework.FrameData.g.h"
#include "Framework.FrameMetadata.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
struct FrameData : FrameDataT<FrameData>
{
    FrameData(MicrosoftDisplayCaptureTools::Framework::ILogger logger) : m_logger(logger){};

    winrt::Windows::Storage::Streams::IBuffer Data();
    void Data(winrt::Windows::Storage::Streams::IBuffer const& data);
    winrt::Windows::Graphics::SizeInt32 Resolution();
    void Resolution(winrt::Windows::Graphics::SizeInt32 const& resolution);
    winrt::MicrosoftDisplayCaptureTools::Framework::FrameDataDescription FormatDescription();
    void FormatDescription(winrt::MicrosoftDisplayCaptureTools::Framework::FrameDataDescription const& description);
    com_array<winrt::MicrosoftDisplayCaptureTools::Framework::FrameMetadata> GetFrameMetadata();
    void AddMetadata(winrt::MicrosoftDisplayCaptureTools::Framework::FrameMetadata const& data);
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Graphics::Imaging::SoftwareBitmap> GetRenderableApproximationAsync();
    winrt::MicrosoftDisplayCaptureTools::Framework::IFrameDataComparisons GetImageDelta(winrt::MicrosoftDisplayCaptureTools::Framework::IFrameData const& other);

private:
    const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    Windows::Storage::Streams::IBuffer m_data{nullptr};
    Windows::Graphics::SizeInt32 m_resolution{0, 0};
    MicrosoftDisplayCaptureTools::Framework::FrameDataDescription m_description{0};
    std::vector<FrameMetadata> m_metadataList;
};
struct FrameMetadata : FrameMetadataT<FrameMetadata>
{
    FrameMetadata() = delete;

    FrameMetadata(hstring const& Format, winrt::Windows::Storage::Streams::IBuffer const& Data);
    winrt::Windows::Storage::Streams::IBuffer Data();
    hstring Name();

private:
    hstring m_format;
    winrt::Windows::Storage::Streams::IBuffer m_data;
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation {
struct FrameData : FrameDataT<FrameData, implementation::FrameData>
{
};
struct FrameMetadata : FrameMetadataT<FrameMetadata, implementation::FrameMetadata>
{
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
