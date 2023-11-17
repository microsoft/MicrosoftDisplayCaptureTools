#include "pch.h"
#include "Utils.h"
#include "Framework.Version.g.cpp"
#include "Framework.Runtime.g.cpp"

#include <format>

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>

namespace winrt 
{
    using namespace winrt::Windows::ApplicationModel::Core;
}

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
hstring Version::ToString()
{
    return to_hstring(Major()) + L"." + to_hstring(Minor()) + L"." + to_hstring(Patch());
}
bool Version::IsHigherVersion(IVersion other)
{
    return m_version > std::make_tuple(other.Major(), other.Minor(), other.Patch());
}

Runtime::Runtime(
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger logger,
    winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings settings) :
    m_logger(logger), m_runtimeSettings(settings)
{
}

void Runtime::CreateRuntime(
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger logger, winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings settings)
{
    auto runtime = winrt::make<Runtime>(logger, settings);
    auto runtimeRaw = runtime.as<IUnknown>()->AddRef();
    auto processProperties = CoreApplication::Properties();
    processProperties.Insert(L"MicrosoftDisplayCaptureToolsRuntimeStore", runtime);
}

winrt::MicrosoftDisplayCaptureTools::Framework::Runtime Runtime::GetRuntime()
{
    auto processProperties = CoreApplication::Properties();
    if (auto found = processProperties.TryLookup(L"MicrosoftDisplayCaptureToolsRuntimeStore"))
    {
        return found.as<winrt::MicrosoftDisplayCaptureTools::Framework::Runtime>();
    }
    return nullptr;
}
winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings Runtime::RuntimeSettings()
{
    return m_runtimeSettings;
}
winrt::MicrosoftDisplayCaptureTools::Framework::ILogger Runtime::Logger()
{
    return m_logger;
}

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
