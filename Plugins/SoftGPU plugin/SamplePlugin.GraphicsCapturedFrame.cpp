#include "pch.h"
#include "SamplePlugin.GraphicsCapturedFrame.h"
#include "SamplePlugin.GraphicsCapturedFrame.g.cpp"

#include <random>

namespace winrt::SamplePlugin::implementation
{
    GraphicsCapturedFrame::GraphicsCapturedFrame(SamplePlugin::FrameCharacteristics const& frameCharacteristics, array_view<uint8_t const> buffer) : 
        m_characteristics(frameCharacteristics)
    {
        m_pixels.resize(frameCharacteristics.byteCount);
        memcpy(m_pixels.data(), buffer.data(), frameCharacteristics.byteCount);
    }

    SamplePlugin::FrameCharacteristics GraphicsCapturedFrame::GetFrameCharacteristics()
    {
        return m_characteristics;
    }

    void GraphicsCapturedFrame::GetFramePixels(array_view<uint8_t> buffer)
    {
        if (buffer.size() < m_pixels.size())
        {
            throw_hresult(ERROR_INVALID_PARAMETER);
        }

        memcpy(buffer.data(), m_pixels.data(), m_pixels.size());
    }
}
