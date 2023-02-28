#pragma once
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Display 
{
	struct __declspec(uuid("08715D73-F1CC-4391-AEEF-BF4DEA64FC1C")) IDisplayEngineInterop : ::IUnknown
	{
        virtual HRESULT __stdcall GetPlaneTexture(ID3D11Texture2D** texture) = 0;
	};
} // namespace winrt::MicrosoftDisplayCaptureTools::Libraries