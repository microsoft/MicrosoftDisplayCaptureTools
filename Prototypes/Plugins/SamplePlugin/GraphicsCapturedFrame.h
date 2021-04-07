#pragma once
#include "GraphicsCapturedFrame.g.h"

namespace winrt::SamplePlugin::implementation
{
    struct GraphicsCapturedFrame : GraphicsCapturedFrameT<GraphicsCapturedFrame>
    {
        GraphicsCapturedFrame() = default;

        GraphicsCapturedFrame(SamplePlugin::FrameCharacteristics const& frameCharacteristics, array_view<uint8_t const> buffer);
        SamplePlugin::FrameCharacteristics GetFrameCharacteristics();
        void GetFramePixels(array_view<uint8_t> buffer);

    private:
        SamplePlugin::FrameCharacteristics m_characteristics;
    };
}
namespace winrt::SamplePlugin::factory_implementation
{
    struct GraphicsCapturedFrame : GraphicsCapturedFrameT<GraphicsCapturedFrame, implementation::GraphicsCapturedFrame>
    {
    };
}
