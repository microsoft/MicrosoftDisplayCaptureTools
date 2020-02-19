#include "pch.h"
#include "GraphicsCapturedFrame.h"
#include "GraphicsCapturedFrame.g.cpp"
#include <random>

namespace winrt::SamplePlugin::implementation
{
    template<typename T>
    struct FrameRGB
    {
        T R, G, B;
        FrameRGB(T r, T g, T b) : R(r), G(g), B(b) {}

        FrameRGB(float x, float y, FrameRGB<T> a, FrameRGB<T> b, FrameRGB<T> c, FrameRGB<T> d)
        {
            R = (T)((a.R * x + (1 - x) * b.R) * y + (c.R * x + (1 - x) * d.R) * (1 - y));
            G = (T)((a.G * x + (1 - x) * b.G) * y + (c.G * x + (1 - x) * d.G) * (1 - y));
            B = (T)((a.B * x + (1 - x) * b.B) * y + (c.B * x + (1 - x) * d.B) * (1 - y));
        }

        FrameRGB(uint32_t maxRandom)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dist(0, maxRandom);
            R = (T)dist(gen);
            G = (T)dist(gen);
            B = (T)dist(gen);
        }
    };

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
        switch (m_characteristics.format)
        {
        case PixelFormat::RGB8:
            {
                FrameRGB<uint8_t> topLeft(0xFF), topRight(0xFF), botLeft(0xFF), botRight(0xFF);

                FrameRGB<uint8_t>* currPosition = reinterpret_cast<FrameRGB<uint8_t>*>(buffer.begin());

                for (int y = 0; y < m_characteristics.height; y++)
                {
                    for (int x = 0; x < m_characteristics.width; x++)
                    {
                        float posX = (float)x / (m_characteristics.width - 1);
                        float posY = (float)y / (m_characteristics.height - 1);

                        *currPosition = FrameRGB<uint8_t>{ posX, posY, topLeft, topRight, botLeft, botRight };
                        currPosition++;
                    }
                }
            }
            break;
        case PixelFormat::RGB16:
            {
                FrameRGB<uint16_t> topLeft(0xFFFF), topRight(0xFFFF), botLeft(0xFFFF), botRight(0xFFFF);

                FrameRGB<uint16_t>* currPosition = reinterpret_cast<FrameRGB<uint16_t>*>(buffer.begin());

                for (int y = 0; y < m_characteristics.height; y++)
                {
                    for (int x = 0; x < m_characteristics.width; x++)
                    {
                        float posX = (float)x / (m_characteristics.width - 1);
                        float posY = (float)y / (m_characteristics.height - 1);

                        *currPosition = FrameRGB<uint16_t>{ posX, posY, topLeft, topRight, botLeft, botRight };
                        currPosition++;
                    }
                }
            }
            break;
        default:
            throw(hresult_invalid_argument());
        }
    }
}
