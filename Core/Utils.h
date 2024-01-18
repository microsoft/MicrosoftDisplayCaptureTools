#pragma once
#include "Framework.Version.g.h"
#include "Framework.Runtime.g.h"

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation {
struct Version : VersionT<Version>
{
    Version() = delete;

    Version(uint32_t major, uint32_t minor, uint32_t patch);

    uint32_t Major();
    uint32_t Minor();
    uint32_t Patch();
    hstring ToString();

    bool IsHigherVersion(IVersion other);

private:
    std::tuple<uint32_t, uint32_t, uint32_t> m_version;
};

struct Runtime : RuntimeT<Runtime>
{
    Runtime() = delete;
    Runtime(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger logger, winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings settings);

    static void CreateRuntime(
        winrt::MicrosoftDisplayCaptureTools::Framework::ILogger logger, winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings settings);
    static winrt::MicrosoftDisplayCaptureTools::Framework::Runtime GetRuntime();
    winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings RuntimeSettings();
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger Logger();

 private:

    winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings m_runtimeSettings;
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;
};

static winrt::com_ptr<Runtime> g_runtime{nullptr};

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation {
struct Version : VersionT<Version, implementation::Version>
{
};
struct Runtime : RuntimeT<Runtime, implementation::Runtime>
{
};
} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
