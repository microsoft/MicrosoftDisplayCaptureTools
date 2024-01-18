module RefreshRateTool;

import "pch.h";

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	static const winrt::hstring DefaultConfiguration = L"60";

	RefreshRateTool::RefreshRateTool() :
        ToolBase::IntTool<RefreshRateTool>(L"RefreshRate", DefaultConfiguration, {L"30", L"60", L"100", L"120"})
	{}

	void RefreshRateTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_eventTokens[L"DisplaySetup"] = displayOutput.DisplaySetupCallback([&](const auto&, IDisplaySetupToolArgs args) 
		{
            constexpr double sc_refreshRateEpsilon = 0.005;

            double presentationRate = static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Numerator) /
                                      static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Denominator);

            args.IsModeCompatible(fabs(presentationRate - m_configuration) < sc_refreshRateEpsilon);
        });

        Logger().LogNote(L"Registering " + Name() + L": " + GetCurrentConfigurationString() + L" to be applied.");
	}
}