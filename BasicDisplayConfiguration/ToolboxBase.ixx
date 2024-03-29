export module ToolboxBase;

import "pch.h";

using namespace winrt::Windows::Graphics;
using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
using namespace winrt::MicrosoftDisplayCaptureTools::Display;
using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;

// Contains methods which convert between configuration strings and the tool's internal data type.
export namespace ToolConfigConversions {

    template <typename T>
    winrt::hstring ToConfigString(T value);

    template <typename T>
    T FromConfigString(const std::wstring_view&& value);

    template <>
    winrt::hstring ToConfigString<int>(int value)
    {
        return winrt::hstring{std::to_wstring(value)};
    }

    template <>
    int FromConfigString<int>(const std::wstring_view&& value)
    {
        return std::stoi(std::wstring(value));
    }

    template <>
    auto ToConfigString<SizeInt32>(SizeInt32 value) -> winrt::hstring
    {
        return winrt::hstring{std::format(L"{0}x{1}", value.Width, value.Height)};
    }

    template <>
    auto FromConfigString<SizeInt32>(const std::wstring_view&& value) -> SizeInt32
    {
        SizeInt32 result{};
        swscanf_s(std::wstring(value).c_str(), L"%dx%d", &result.Width, &result.Height);
        return result;
    }

} // namespace ToolConfigConversions


export namespace ToolBase {
    /// <summary>
    /// Provides the basic implementation of a tool that affects rendering by placing its value in the IDisplayEngineProperties.Properties bag or another property.
    /// </summary>
    /// <typeparam name="TDerived"></typeparam>
    /// <typeparam name="TDataType"></typeparam>
    template <typename TDerived, typename TDataType, ConfigurationToolCategory CategoryValue>
    struct BasicToolBase : public winrt::implements<TDerived, IConfigurationTool>
    {
        BasicToolBase(
            winrt::hstring name,
            winrt::hstring defaultConfig,
            std::initializer_list<winrt::hstring> supportedConfigs)
        {
            m_name = name;
            m_defaultConfig = ToolConfigConversions::FromConfigString<TDataType>(defaultConfig);
            m_configuration = m_defaultConfig;
            m_supportedConfigs = supportedConfigs;
        }

        winrt::hstring Name()
        {
            return m_name;
        }
        ConfigurationToolCategory Category()
        {
            return CategoryValue;
        }
        IConfigurationToolRequirements Requirements()
        {
            return nullptr;
        }
        winrt::com_array<winrt::hstring> GetSupportedConfigurations()
        {
            return winrt::com_array<winrt::hstring>{m_supportedConfigs};
        }
        winrt::hstring GetDefaultConfiguration()
        {
            return ToolConfigConversions::ToConfigString<TDataType>(m_defaultConfig);
        }
        winrt::hstring GetCurrentConfigurationString()
        {
            return ToolConfigConversions::ToConfigString(m_configuration);
        }
        winrt::hstring GetConfiguration()
        {
            return ToolConfigConversions::ToConfigString<TDataType>(m_configuration);
        }
        void SetConfiguration(winrt::hstring configuration)
        {
            // Validate that the configuration is within the available settings
            if (std::end(m_supportedConfigs) == std::find(m_supportedConfigs.begin(), m_supportedConfigs.end(), configuration))
            {
                // An invalid configuration was asked for
                Logger().LogError(L"An invalid configuration was requested: " + configuration);

                throw winrt::hresult_invalid_argument();
            }

            m_configuration = ToolConfigConversions::FromConfigString<TDataType>(configuration);
        }
        void ApplyToOutput(IDisplayOutput displayOutput)
        {
        }
        void ApplyToPrediction(IPrediction displayPrediction)
        {
        }

    protected:
        winrt::hstring m_name;
        TDataType m_defaultConfig;
        TDataType m_configuration;
        std::vector<winrt::hstring> m_supportedConfigs;
        std::map<winrt::hstring, winrt::event_token> m_eventTokens;
    };

    /// <summary>
    /// Provides a tool implementation that stores an integer value.
    /// </summary>
    template <typename TDerived>
    struct IntTool : BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>
    {
        // using BaseType::BasicToolBase;
        using BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>::BasicToolBase;
    };

    /// <summary>
    /// Provides a tool implementation that stores a SizeInt32 value.
    /// </summary>
    template <typename TDerived>
    struct SizeTool : BasicToolBase<TDerived, SizeInt32, ConfigurationToolCategory::DisplaySetup>
    {
        using BasicToolBase<TDerived, SizeInt32, ConfigurationToolCategory::DisplaySetup>::BasicToolBase;
    };

    template <typename TToolType, typename TDerived>
    struct ToolFactory
    {
    };

    template <typename TDerived>
    struct ToolFactory<int, TDerived>
    {
        IConfigurationTool CreateTool(
            PCWSTR name,
            winrt::hstring defaultConfig,
            std::initializer_list<winrt::hstring> supportedConfigs)
        {
            return winrt::make<IntTool<TDerived>>(name, defaultConfig, supportedConfigs);
        }
    };

    template <typename TDerived>
    struct ToolFactory<SizeInt32, TDerived>
    {
        IConfigurationTool CreateTool(
            PCWSTR name,
            winrt::hstring defaultConfig,
            std::initializer_list<winrt::hstring> supportedConfigs)
        {
            return winrt::make<SizeTool<TDerived>>(name, defaultConfig, supportedConfigs);
        }
    };
} // namespace ToolBase