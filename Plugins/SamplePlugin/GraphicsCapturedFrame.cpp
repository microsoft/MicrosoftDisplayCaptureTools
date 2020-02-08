#include "pch.h"
#include "GraphicsCapturedFrame.h"
#include "GraphicsCapturedFrame.g.cpp"

namespace winrt::SamplePlugin::implementation
{
    GraphicsCapturedFrame::GraphicsCapturedFrame(SamplePlugin::FrameCharacteristics const& frameCharacteristics) :
        m_characteristics(frameCharacteristics)
    {
    }
    SamplePlugin::FrameCharacteristics GraphicsCapturedFrame::GetFrameCharacteristics()
    {
        return m_characteristics;
    }
    void GraphicsCapturedFrame::GetFramePixels(array_view<uint8_t> buffer)
    {
        struct FrameRGB
        {
            uint8_t R, G, B;
            FrameRGB(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b) {}

            FrameRGB(float x, float y, FrameRGB a, FrameRGB b, FrameRGB c, FrameRGB d)
            {
                R = (float)((a.R * x + (1 - x) * b.R) * y + (c.R * x + (1 - x) * d.R) * (1 - y));
                G = (float)((a.G * x + (1 - x) * b.G) * y + (c.G * x + (1 - x) * d.G) * (1 - y));
                B = (float)((a.B * x + (1 - x) * b.B) * y + (c.B * x + (1 - x) * d.B) * (1 - y));
            }
        };

        FrameRGB topLeft{ 0xFF, 0xFF, 0xFF }, topRight{ 0xFF, 0x00, 0xFF },
            botLeft{ 0xFF, 0xFF, 0x00 }, botRight{ 0x00, 0xFF, 0xFF };

        FrameRGB* currPosition = reinterpret_cast<FrameRGB*>(buffer.begin());

        for (int x = 0; x < m_characteristics.width; x++)
        {
            for (int y = 0; y < m_characteristics.height; y++)
            {
                float posX = (float)x / (m_characteristics.width  - 1);
                float posY = (float)y / (m_characteristics.height - 1);

                *currPosition = FrameRGB{posX, posY, topLeft, topRight, botLeft, botRight};

                currPosition += 1;
            }
        }
    }
}
