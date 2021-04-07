#include "pch.h"
#include "ConfigurationTools.ConfigurationToolbox.h"
#include "ConfigurationTools.ConfigurationToolbox.g.cpp"

namespace winrt::ConfigurationTools::implementation
{
    enum class Tools
    {
        Pattern
    };

    std::map<std::wstring, Tools> MapNameToTool =
    {
        {L"Pattern", Tools::Pattern}
    };

    hstring ConfigurationToolbox::Name()
    {
        return L"Public Toolbox";
    }
    com_array<hstring> ConfigurationToolbox::GetSupportedTools()
    {
        auto toolNames = std::vector<hstring>();
        for (auto tool : MapNameToTool)
        {
            toolNames.push_back(hstring(tool.first));
        }

        return com_array<hstring>(toolNames);
    }
    ConfigurationTools::IConfigurationTool ConfigurationToolbox::GetTool(hstring const& toolName)
    {
        switch (MapNameToTool[std::wstring(toolName)])
        {
        case Tools::Pattern:
            return winrt::make<PatternTool>();
            break;
        }

        // This toolbox does not implement the tool asked for.
        throw winrt::hresult_not_implemented();
    }
}
