#pragma once

class CaptureFrameworkTestBase
{
public:
    virtual bool Setup();
    virtual bool Cleanup();

protected:
    winrt::MicrosoftDisplayCaptureTools::Framework::Core m_framework{nullptr};
};
