#pragma once

#include "winrt/SamplePlugin.h"

using namespace winrt::SamplePlugin;

struct PixelDataFormat
{
    PixelDataFormat(PixelFormat pixelFormat) : m_format(pixelFormat)
    {
        if (pixelFormat > PixelFormat::BGRA16) winrt::throw_hresult(ERROR_INVALID_PARAMETER);
    }

    GUID GetWICType()
    {
        switch (m_format)
        {
        case PixelFormat::RGB8:
            return GUID_WICPixelFormat24bppRGB;
        case PixelFormat::BGR8:
            return GUID_WICPixelFormat24bppBGR;
        case PixelFormat::RGB16:
            return GUID_WICPixelFormat48bppRGB;
        case PixelFormat::BGR16:
            return GUID_WICPixelFormat48bppBGR;
        case PixelFormat::RGBA8:
            return GUID_WICPixelFormat32bppRGBA;
        case PixelFormat::BGRA8:
            return GUID_WICPixelFormat32bppBGRA;
        case PixelFormat::RGBA16:
            return GUID_WICPixelFormat64bppRGBA;
        case PixelFormat::BGRA16:
            return GUID_WICPixelFormat64bppBGRA;
        default:
            winrt::throw_hresult(ERROR_INVALID_PARAMETER);
        }
    }

    size_t GetPixelSize()
    {
        switch (m_format)
        {
        case PixelFormat::RGB8:
        case PixelFormat::BGR8:
            return 24;
        case PixelFormat::RGB16:
        case PixelFormat::BGR16:
            return 48;
        case PixelFormat::RGBA8:
        case PixelFormat::BGRA8:
            return 32;
        case PixelFormat::RGBA16:
        case PixelFormat::BGRA16:
            return 64;
        default:
            winrt::throw_hresult(ERROR_INVALID_PARAMETER);
        }
    }

    const PixelFormat m_format;
};