export module ToolboxBase;

import "pch.h";

using namespace winrt::Windows::Graphics;
using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;

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
        std::initializer_list<winrt::hstring> supportedConfigs,
        winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger)
    {
        m_name = name;
        m_defaultConfig = ToolConfigConversions::FromConfigString<TDataType>(defaultConfig);
        m_configuration = m_defaultConfig;
        m_supportedConfigs = supportedConfigs;
        m_logger = logger;
    }

    winrt::hstring Name()
    {
        return m_name;
    }
    winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category()
    {
        return CategoryValue;
    }
    winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements()
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
    winrt::hstring GetConfiguration()
    {
        return ToolConfigConversions::ToConfigString<TDataType>(m_configuration);
    }
    void SetConfiguration(winrt::hstring configuration)
    {
        m_configuration = ToolConfigConversions::FromConfigString<TDataType>(configuration);
    }
    void ApplyToOutput(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput displayOutput)
    {
    }
    void ApplyToPrediction(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayPrediction displayPrediction)
    {
    }

protected:
    winrt::hstring m_name;
    TDataType m_defaultConfig;
    TDataType m_configuration;
    std::vector<winrt::hstring> m_supportedConfigs;
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;
};

/// <summary>
/// Provides a tool implementation that stores an integer value.
/// </summary>
export template <typename TDerived>
struct IntTool : BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>
{
    using BaseType = BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>;

    using BaseType::BasicToolBase;
};

template <>
struct IntTool<int> : BasicToolBase<IntTool, int, ConfigurationToolCategory::DisplaySetup>
{
    using BaseType = BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>;

    using BaseType::BasicToolBase;
};

/// <summary>
/// Provides a tool implementation that stores a SizeInt32 value.
/// </summary>
export template <typename TDerived>
struct SizeTool
    : BasicToolBase<
          TDerived,
          SizeInt32,
          ConfigurationToolCategory::DisplaySetup>
{
    using BaseType = BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>;

    using BaseType::BasicToolBase;
};

template <>
struct SizeTool<SizeInt32>
    : BasicToolBase<
          SizeTool,
          SizeInt32,
          ConfigurationToolCategory::DisplaySetup>
{
    using BaseType = BasicToolBase<TDerived, int, ConfigurationToolCategory::DisplaySetup>;

    using BaseType::BasicToolBase;
};

export template <typename TDataType>
std::function<IConfigurationTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const&)> CreateToolFactory(
    PCWSTR name, winrt::hstring defaultConfig, std::initializer_list<winrt::hstring> supportedConfigs);

export template <>
std::function<IConfigurationTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const&)> CreateToolFactory<int>(
    PCWSTR name, winrt::hstring defaultConfig, std::initializer_list<winrt::hstring> supportedConfigs)
{
    return [=](winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) -> IConfigurationTool {
        return winrt::make<IntTool>(name, defaultConfig, supportedConfigs, logger);
    };
}

export template <>
std::function<IConfigurationTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const&)> CreateToolFactory<SizeInt32>(
    PCWSTR name, winrt::hstring defaultConfig, std::initializer_list<winrt::hstring> supportedConfigs)
{
    return [=](winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) -> IConfigurationTool {
        return winrt::make<SizeTool>(name, defaultConfig, supportedConfigs, logger);
    };
}
