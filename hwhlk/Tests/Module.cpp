#include "pch.h"

namespace MicrosoftDisplayCaptureTools::Tests {

// clang-format off
BEGIN_MODULE()
	MODULE_PROPERTY(L"Area", L"Graphics")
	MODULE_PROPERTY(L"SubArea", L"Display")
END_MODULE()
// clang-format on

MODULE_SETUP(ModuleSetup)
{
    winrt::init_apartment();

	return true;
}

MODULE_CLEANUP(ModuleCleanup)
{
    return true;
}

} // namespace MicrosoftDisplayCaptureTools::Tests