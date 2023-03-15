#pragma once
#include "Framework.Version.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
struct Version : VersionT<Version>
{
    Version() = delete;

    Version(uint32_t major, uint32_t minor, uint32_t patch);

    uint32_t Major();
    uint32_t Minor();
    uint32_t Patch();
    hstring AsString();

    bool IsHigherVersion(IVersion other);

private:
    std::tuple<uint32_t, uint32_t, uint32_t> m_version;
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation {
struct Version : VersionT<Version, implementation::Version>
{
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
