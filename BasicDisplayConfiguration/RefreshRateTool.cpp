module RefreshRateTool;

import "pch.h";

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	static const winrt::hstring DefaultConfiguration = L"60";

	RefreshRateTool::RefreshRateTool(winrt::ILogger const& logger) :
        ToolBase::IntTool<RefreshRateTool>(L"RefreshRate", DefaultConfiguration, {L"30", L"60", L"100", L"120"}, logger)
	{}

	IConfigurationToolRequirements RefreshRateTool::Requirements()
	{
		return nullptr;
	}

	void RefreshRateTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        constexpr double sc_refreshRateEpsilon = 0.005;

        m_eventTokens[L"DisplaySetup"] = displayOutput.DisplaySetupCallback([&](const auto&, IDisplaySetupToolArgs args) 
		{
            double presentationRate = static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Numerator) /
                                      static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Denominator);

            args.IsModeCompatible(fabs(presentationRate - m_configuration) < sc_refreshRateEpsilon);
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + GetCurrentConfigurationString() + L" to be applied.");
	}

	void RefreshRateTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        // This tool doesn't currently matter to the output
	}
}