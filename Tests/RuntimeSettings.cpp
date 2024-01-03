#include "pch.h"
#include <cwctype>

namespace winrt {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
} // namespace winrt

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace RuntimeSettings {

    static winrt::IRuntimeSettings runtimeSettingsInstance = nullptr;

    winrt::IInspectable RuntimeSettings::GetSettingValue(winrt::hstring name)
    {
        String value;
        if (SUCCEEDED(RuntimeParameters::TryGetValue(name.c_str(), value)) && !String::IsNullOrEmpty(value))
        {
            return winrt::box_value(winrt::hstring(value));
        }

        return nullptr;
    }

    bool RuntimeSettings::GetSettingValueAsBool(winrt::hstring name)
    {
        auto value = GetSettingValue(name);
        if (value)
        {
            auto valueStr = std::wstring(winrt::unbox_value<winrt::hstring>(value).c_str());
            std::transform(valueStr.begin(), valueStr.end(), valueStr.begin(), std::towlower);
            
            return valueStr == L"true";
        }
        return false;
    }

    winrt::hstring RuntimeSettings::GetSettingValueAsString(winrt::hstring name)
    {
        auto value = GetSettingValue(name);
        if (value)
        {
            return winrt::unbox_value<winrt::hstring>(value);
        }
        return L"";
    }

    double RuntimeSettings::GetSettingValueAsDouble(winrt::hstring name)
    {
        auto value = GetSettingValue(name);
        if (value)
        {
            auto valueStr = std::wstring(winrt::unbox_value<winrt::hstring>(value).c_str());
            return _wtof(valueStr.c_str());
        }
        return 0;
    }

    winrt::IRuntimeSettings GetRuntimeSettings()
    {
        if (!runtimeSettingsInstance)
        {
            runtimeSettingsInstance = winrt::make<RuntimeSettings>();
        }

        return runtimeSettingsInstance;
    }
} // namespace RuntimeSettings