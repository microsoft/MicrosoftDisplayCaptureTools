#pragma once
#include "Framework.FrameData.g.h"

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
    winrt::Windows::Graphics::Imaging::SoftwareBitmap GetRenderableApproximation();
    winrt::MicrosoftDisplayCaptureTools::Framework::IFrameDataComparisons GetImageDelta(winrt::MicrosoftDisplayCaptureTools::Framework::IFrameData const& other);

private:
    const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    Windows::Storage::Streams::IBuffer m_data{nullptr};
    Windows::Graphics::SizeInt32 m_resolution{0, 0};
    MicrosoftDisplayCaptureTools::Framework::FrameDataDescription m_description{0};
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation {
struct FrameData : FrameDataT<FrameData, implementation::FrameData>
{
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
