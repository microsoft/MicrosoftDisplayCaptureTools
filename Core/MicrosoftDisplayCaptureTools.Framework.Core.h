#pragma once
#include "MicrosoftDisplayCaptureTools.Framework.Core.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::MicrosoftDisplayCaptureTools::Framework::implementation
{
    struct Core : CoreT<Core>
    {
        Core() = default;

        void LoadPlugin(hstring const& toolboxPath);
        void OpenToolbox(hstring const& toolboxPath);
        void RunPictTest();
    };
}
namespace winrt::MicrosoftDisplayCaptureTools::Framework::factory_implementation
{
    struct Core : CoreT<Core, implementation::Core>
    {
    };
}
