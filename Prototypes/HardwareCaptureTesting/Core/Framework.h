#pragma once
#include "Framework.g.h"

namespace winrt::HardwareCaptureTesting::Core::implementation
{
    struct Framework : FrameworkT<Framework>
    {
        Framework() = default;

        // Initializes the framework with a specific plugin and toolbox to load.
        // This constructor will load the supplied dlls, and verify that everything
        // is set up to run tests.
        Framework(hstring const& pluginPath, hstring const& toolboxPath);

        hstring SetOutputDirectory();
        void SetOutputDirectory(hstring const& value);
        hstring SetComparisonDirectory();
        void SetComparisonDirectory(hstring const& value);

        // Registers additional suppliers of 'tools'. This is intended to allow support for 
        // in-development and custom tool implementations.
        void AddToolbox(hstring const& toolboxPath);

        // Retrieve the plugin directly. If you want to build a manual test, you'll
        // need this to fetch the capture card's captured display state.
        HardwareCaptureTesting::CaptureCard::Plugin GetPlugin();

        // Retrieves a map of the OS-tracked display paths to the plugin inputs.
        void GetDisplaysUnderTest(Windows::Foundation::Collections::IMap<Windows::Devices::Display::Core::DisplayTarget, uint32_t>& displays);

        // Sets things up for a manual test. The test name indicates how this should be logged.
        void StartManualTest(hstring const& testName);

        // Sets things up for a combinatorial test, including getting 
        void StartCombinatorialTest();

        // Modify the state of a supported 'tool' - these should match the
        // supplied PICT parameters.
        void UseTool(hstring const& toolName, hstring const& toolParams);

        // Capture the current state and evaluate against the expected match
        void EvaluateCapture();
    };
}
namespace winrt::HardwareCaptureTesting::Core::factory_implementation
{
    struct Framework : FrameworkT<Framework, implementation::Framework>
    {
    };
}
