#include "pch.h"
#include "Utils.h"
#include "Framework.Version.g.cpp"
#include "Framework.Runtime.g.cpp"

#include <format>

#include <wil\com_apartment_variable.h>

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {

wil::apartment_variable<winrt::com_ptr<Runtime>> g_runtime;

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
    g_runtime.get_or_create([&logger, &settings]() -> winrt::com_ptr<Runtime>
        {
            auto runtime = winrt::make_self<Runtime>(logger, settings);
            runtime->AddRef();

            winrt::com_ptr<Runtime> rawRuntime;
            rawRuntime.attach(runtime.get());

            return rawRuntime;
        });
}

winrt::MicrosoftDisplayCaptureTools::Framework::Runtime Runtime::GetRuntime()
{
    auto runtime = g_runtime.get_if();
    if (runtime)
    {
        return runtime->as<winrt::MicrosoftDisplayCaptureTools::Framework::Runtime>();
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
