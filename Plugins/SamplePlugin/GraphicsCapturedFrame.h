#pragma once
#include "GraphicsCapturedFrame.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCapturedFrame : GraphicsCapturedFrameT<GraphicsCapturedFrame>
    {
        GraphicsCapturedFrame() = default;

        SamplePlugin::FrameCharacteristics GetFrameCharacteristics();
        void GetFramePixels(array_view<uint8_t> buffer);
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCapturedFrame : GraphicsCapturedFrameT<GraphicsCapturedFrame, implementation::GraphicsCapturedFrame>
    {
    };
}
