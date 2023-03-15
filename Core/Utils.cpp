#include "pch.h"
#include "Utils.h"
#include "Framework.Version.g.cpp"

#include <format>

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {

Version::Version(uint32_t major, uint32_t minor, uint32_t patch) : 
    m_version{major, minor, patch}
{
}
uint32_t Version::Major()
{
    return std::get<0>(m_version);
}
uint32_t Version::Minor()
{
    return std::get<1>(m_version);
}
uint32_t Version::Patch()
{
    return std::get<2>(m_version);
}
hstring Version::AsString()
{
    return to_hstring(Major()) + L"." + to_hstring(Minor()) + L"." + to_hstring(Patch());
}
bool Version::IsHigherVersion(IVersion other)
{
    std::tuple<uint32_t, uint32_t, uint32_t> otherVersion
    {
        other.Major(), other.Minor(), other.Patch()
    };
    return m_version > otherVersion;
}
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
