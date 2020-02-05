#include "pch.h"
#include "GraphicsCapturedFrame.h"
#include "GraphicsCapturedFrame.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    SamplePlugin::FrameCharacteristics GraphicsCapturedFrame::GetFrameCharacteristics()
    {
        throw hresult_not_implemented();
    }
    void GraphicsCapturedFrame::GetFramePixels(array_view<uint8_t> buffer)
    {
        throw hresult_not_implemented();
    }
}
