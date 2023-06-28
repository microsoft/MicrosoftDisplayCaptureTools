module;

#include "pch.h"
#include "winrt/MicrosoftDisplayCaptureTools.Framework.h"

export module MonitorControl;

namespace winrt {
    // Standard WinRT inclusions
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;

    // Hardware HLK project
    using namespace winrt::MicrosoftDisplayCaptureTools::Display;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::Libraries;
} // namespace winrt

namespace MonitorUtilities {

export LUID LuidFromAdapterId(winrt::Windows::Graphics::DisplayAdapterId id)
{
    return {id.LowPart, id.HighPart};
}

export class MonitorControl
{
public:
    MonitorControl(LUID adapterId, UINT targetId, winrt::ILogger const& logger) :
        m_luid(adapterId), m_targetId(targetId), m_removeSpecializationOnExit(true), m_logger(logger)
    {
        DISPLAYCONFIG_GET_MONITOR_SPECIALIZATION getSpecialization{};
        getSpecialization.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_MONITOR_SPECIALIZATION;
        getSpecialization.header.id = m_targetId;
        getSpecialization.header.adapterId = m_luid;
        getSpecialization.header.size = sizeof(getSpecialization);

        winrt::check_win32(DisplayConfigGetDeviceInfo(&getSpecialization.header));

        if (0 == getSpecialization.isSpecializationAvailableForSystem)
        {
            m_logger.LogError(L"Monitor specialization is not available - have you enabled test signing?");
            throw winrt::hresult_error();
        }

        if (1 == getSpecialization.isSpecializationEnabled)
        {
            m_removeSpecializationOnExit = false;
        }
        else
        {
            Toggle();
        }
    }

    ~MonitorControl()
    {
        if (m_removeSpecializationOnExit)
        {
            Toggle(true);
        }
    }

private:
    void Toggle(bool reset = false)
    {
        DISPLAYCONFIG_SET_MONITOR_SPECIALIZATION setSpecialization{};
        setSpecialization.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_MONITOR_SPECIALIZATION;
        setSpecialization.header.id = m_targetId;
        setSpecialization.header.adapterId = m_luid;
        setSpecialization.header.size = sizeof(setSpecialization);

        setSpecialization.isSpecializationEnabled = reset ? 0 : 1;
        wsprintf(setSpecialization.specializationApplicationName, L"%s", L"HardwareHLK - BasicDisplayControl");
        setSpecialization.specializationType = GUID_MONITOR_OVERRIDE_PSEUDO_SPECIALIZED;
        setSpecialization.specializationSubType = GUID_NULL;

        winrt::check_win32(DisplayConfigSetDeviceInfo(&setSpecialization.header));
    }

    LUID m_luid;
    UINT m_targetId;
    bool m_removeSpecializationOnExit;
    const winrt::ILogger m_logger;
};

} // namespace MonitorUtilities
