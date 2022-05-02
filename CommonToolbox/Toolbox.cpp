#include "pch.h"
#include "Toolbox.h"
#include "Toolbox.g.cpp"

#include "PatternTool.h"
#include "ResolutionTool.h"
#include "RefreshRateTool.h"

namespace winrt
{
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::Windows::Data::Json;
}

namespace winrt::DisplayConfiguration::implementation
{
    enum class Tools
    {
        Pattern,
        Resolution,
        RefreshRate
    };

    std::map<hstring, Tools> MapNameToTool =
    {
        {L"Pattern", Tools::Pattern},
        {L"Resolution", Tools::Resolution},
        {L"RefreshRate", Tools::RefreshRate}
    };

    hstring Toolbox::Name()
    {
        return L"CommonToolbox";
    }
    com_array<hstring> Toolbox::GetSupportedTools()
    {
        auto toolNames = std::vector<hstring>();
        for (auto tool : MapNameToTool)
        {
            toolNames.push_back(tool.first);
        }

        return com_array<hstring>(toolNames);
    }
    IConfigurationTool Toolbox::GetTool(hstring const& toolName)
    {
        switch (MapNameToTool[toolName])
        {
        case Tools::Pattern:
            return winrt::make<PatternTool>();
        case Tools::RefreshRate:
            return winrt::make<RefreshRateTool>();
        case Tools::Resolution:
            return winrt::make<ResolutionTool>();
        }

        // The caller has asked for a tool that is not exposed from this toolbox
        // TODO - log this case
        throw winrt::hresult_invalid_argument();
    }

    void Toolbox::SetConfigData(IJsonValue data)
    {
    }
}
