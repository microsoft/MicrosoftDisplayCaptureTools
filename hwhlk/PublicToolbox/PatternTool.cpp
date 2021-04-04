#include "pch.h"
#include "PatternTool.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt::ConfigurationTools::implementation
{
    PatternTool::PatternTool()
    {
        // Define the default configuration
        m_currentConfig = PatternToolConfigurations::Black;
    }

    hstring PatternTool::Name()
    {
        return L"PatternTool";
    }

    ConfigurationTools::ConfigurationToolCategory PatternTool::Category()
    {
        return ConfigurationTools::ConfigurationToolCategory::Render;
    }

    ConfigurationTools::ConfigurationToolRequirements PatternTool::Requirements()
    {
        auto reqs = ConfigurationTools::ConfigurationToolRequirements();
        reqs.ContributedComparisonTolerance = 0.f;
        reqs.MaxComparisonTolerance = 0.f;

        return reqs;
    }

    std::map<std::wstring, PatternToolConfigurations> MapNameToConfiguration =
    {
        {L"Black", PatternToolConfigurations::Black},
        {L"White", PatternToolConfigurations::White},
        {L"Red",   PatternToolConfigurations::Red},
        {L"Green", PatternToolConfigurations::Green},
        {L"Blue",  PatternToolConfigurations::Blue}
    };

    com_array<hstring> PatternTool::GetSupportedConfigurations()
    {
        auto configNames = std::vector<hstring>();
        for (auto tool : MapNameToConfiguration)
        {
            configNames.push_back(hstring(tool.first));
        }

        return com_array<hstring>(configNames);
    }

    void PatternTool::SetConfiguration(hstring const& configuration)
    {
        m_currentConfig = MapNameToConfiguration[std::wstring(configuration)];
    }

    void PatternTool::ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target)
    {
    }

    void PatternTool::ApplyToReference(DisplayStateReference::IStaticReference const& reference)
    {
        auto frameInfo = reference.FrameInfo();

        if (frameInfo.format != winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Rgba8)
        {
            Log::Error(String().Format(L"The Pattern Tool, does not support the chosen output format: %d. Either support needs to be added \
                       to this tool or a PICT constraint should be added to prevent this case", frameInfo.format));
        }

        // TODO: replace with with an attempt at the GPU path when avilable
        ApplyToSoftwareReferenceFallback(reference);
    }

    void PatternTool::ApplyToSoftwareReferenceFallback(DisplayStateReference::IStaticReference const& reference)
    {
        auto frameInfo = reference.FrameInfo();


        auto frame = reference.GetFrame();
        auto lock = frame.LockBuffer(winrt::Windows::Graphics::Imaging::BitmapBufferAccessMode::ReadWrite);
        auto pixelBuffer = lock.CreateReference();
        auto pixelBufferByteAccess = pixelBuffer.as<::Windows::Foundation::IMemoryBufferByteAccess>();

        BYTE* pixels;
        UINT32 pixelsSize = 0;
        winrt::check_hresult(pixelBufferByteAccess->GetBuffer(&pixels, &pixelsSize));

        struct R8G8B8A8
        {
            BYTE r, g, b, a;
        };

        R8G8B8A8 setPixelValues{};
        switch (m_currentConfig)
        {
        case PatternToolConfigurations::Black:
            break;
        case PatternToolConfigurations::White:
            setPixelValues = { 255,255,255,255 };
            break;
        case PatternToolConfigurations::Red:
            setPixelValues = { 255,0,0,255 };
            break;
        case PatternToolConfigurations::Green:
            setPixelValues = { 0,255,0,255 };
            break;
        case PatternToolConfigurations::Blue:
            setPixelValues = { 0,0,255,255 };
            break;
        }

        for (BYTE* pixel = pixels; pixel < (pixels + pixelsSize); pixel += sizeof(R8G8B8A8))
        {
            auto rgb = reinterpret_cast<R8G8B8A8*>(pixel);
            rgb->r = setPixelValues.r;
            rgb->g = setPixelValues.g;
            rgb->b = setPixelValues.b;
            rgb->a = setPixelValues.a;
        }

    }
}
