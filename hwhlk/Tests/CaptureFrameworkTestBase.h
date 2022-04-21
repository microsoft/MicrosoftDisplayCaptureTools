#pragma once

class CaptureFrameworkTestBase
{
    inline static const wchar_t ConfigFileRuntimeParameter[] = L"DisplayCaptureConfig";
    inline static const wchar_t DisableFirmwareUpdateRuntimeParameter[] = L"DisableFirmwareUpdate";

public:
    virtual bool Setup();
    virtual bool Cleanup();

protected:
    winrt::MicrosoftDisplayCaptureTools::Framework::Core m_framework{nullptr};
};
