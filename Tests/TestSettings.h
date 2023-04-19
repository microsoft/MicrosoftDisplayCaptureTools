#pragma once

namespace MicrosoftDisplayCaptureTools::Tests {
	inline static const wchar_t ConfigFileRuntimeParameter[] = L"DisplayCaptureConfig";
	inline static const wchar_t DisableFirmwareUpdateRuntimeParameter[] = L"DisableFirmwareUpdate";
} // namespace MicrosoftDisplayCaptureTools::Tests

extern winrt::MicrosoftDisplayCaptureTools::Framework::Core g_framework;
extern winrt::MicrosoftDisplayCaptureTools::Framework::ILogger g_logger;
extern winrt::Windows::Foundation::Collections::IVector<winrt::MicrosoftDisplayCaptureTools::Framework::ISourceToSinkMapping> g_displayMap;