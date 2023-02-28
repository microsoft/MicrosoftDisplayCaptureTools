#include "pch.h"
#include "Utils.h"
#include "Framework.Version.g.cpp"

#include <format>

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {

Version::Version(uint32_t major, uint32_t minor, uint32_t patch) : 
    m_major(major), m_minor(minor), m_patch(patch)
{
    // If any of the version members is too large, throw.
    if (major > MaxVersion || minor > MaxVersion || patch > MaxVersion)
    {
        throw winrt::hresult_invalid_argument();
    }
}
uint32_t Version::Major()
{
    return m_major;
}
uint32_t Version::Minor()
{
    return m_minor;
}
uint32_t Version::Patch()
{
    return m_patch;
}
hstring Version::AsString()
{
    return to_hstring(m_major) + L"." + to_hstring(m_minor) + L"." + to_hstring(m_patch);
}
bool Version::IsHigherVersion(IVersion other)
{
    return (m_major * MaxVersion * MaxVersion + m_minor * MaxVersion + m_patch) >
           (other.Major() * MaxVersion * MaxVersion + other.Minor() * MaxVersion + other.Patch());
}
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
