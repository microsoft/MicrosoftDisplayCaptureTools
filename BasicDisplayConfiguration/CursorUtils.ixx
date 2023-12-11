export module CursorUtils;

import "pch.h";
import Bitmap;

export enum class CursorType
{
    Color,
    ColorMask,
    Monochrome
};

export struct CursorShapeInfo
{
    CursorType Type;
    DWORD Width;
    DWORD Height;
};

class CursorHelper
{
public:
    //
    // Performs processing on a bitmap to convert XOR/inverted pixels to use a
    // black + white border scheme. This should be used when the sink doesn't
    // support XOR/inverted pixels.
    //
    static void ConvertXor(_In_ const Bitmap &inputBitmap, _Inout_ Bitmap &outBitmap, _Inout_ POINT &hotspot);

    //
    // Converts a masked color bitmap into a straight-alpha bitmap.
    //
    static void ConvertMaskToStraight(_Inout_ Bitmap &bitmap);

    //
    // Converts monochrome image planes into a 32-bit masked color bitmap. The mask
    // is stored in the alpha bits.
    //
    static void ConvertMonochromeToColor(DWORD width, DWORD height, DWORD pitch, _In_reads_bytes_(height * pitch) const BYTE *sourceAnd,
        _In_reads_bytes_(height * pitch) const BYTE *sourceXor, _Inout_ Bitmap &outBitmap, bool preserveDest = false);

    static bool ContainsXor(const Bitmap &inputBitmap);

};

module : private;

using namespace std;
using namespace Microsoft::WRL;

constexpr DWORD BLACK = 0;
constexpr DWORD WHITE = 0xFFFFFF;
constexpr BYTE XOR_ALPHA = 0xFF;

//
// Performs processing on a bitmap to convert XOR/inverted pixels to use a
// black + white border scheme. This should be used when the sink doesn't
// support XOR/inverted pixels.
//
void CursorHelper::ConvertXor(_In_ const Bitmap &inputBitmap, _Inout_ Bitmap &outBitmap, _Inout_ POINT &hotspot)
{
    // The emboss border width is based on the size of the cursor
    const DWORD borderSize = (inputBitmap.Width + 47) / 48;

    // Allocate a bitmap that can accommodate the border
    outBitmap = Bitmap(inputBitmap.Width + borderSize * 2, inputBitmap.Height + borderSize * 2);
    outBitmap.Clear(XOR_ALPHA, BLACK);

    for (DWORD cy = 0; cy < inputBitmap.Height; ++cy)
    {
        for (DWORD cx = 0; cx < inputBitmap.Width; ++cx)
        {
            // If this is a non-transparent pixel
            if (!(inputBitmap.GetPixelAlpha(cx, cy) && inputBitmap.GetPixelColor(cx, cy) == BLACK))
            {
                // If this is an inverted (XOR) pixel
                if (inputBitmap.GetPixelAlpha(cx, cy) && inputBitmap.GetPixelColor(cx, cy) != BLACK)
                {
                    // Set the pixel to black
                    outBitmap.SetPixelAlpha(cx + borderSize, cy + borderSize, 0);
                    outBitmap.SetPixelColor(cx + borderSize, cy + borderSize, BLACK);
                }
                else
                {
                    outBitmap.SetPixelAlpha(cx + borderSize, cy + borderSize, inputBitmap.GetPixelAlpha(cx, cy));
                    outBitmap.SetPixelColor(cx + borderSize, cy + borderSize, inputBitmap.GetPixelColor(cx, cy));
                }

                // Outline the pixels in white; note outerCx and outerCy are output coordinates
                for (DWORD outerCy = cy; outerCy < cy + borderSize * 2 + 1; outerCy++)
                {
                    for (DWORD outerCx = cx; outerCx < cx + borderSize * 2 + 1; outerCx++)
                    {
                        // If the border pixel is transparent
                        if (outBitmap.GetPixelAlpha(outerCx, outerCy) && outBitmap.GetPixelColor(outerCx, outerCy) == BLACK)
                        {
                            // Set the border pixel to white
                            outBitmap.SetPixelAlpha(outerCx, outerCy, 0);
                            outBitmap.SetPixelColor(outerCx, outerCy, WHITE);
                        }
                    }
                }
            }
        }
    }

    hotspot.x += borderSize;
    hotspot.y += borderSize;
}

//
// Converts a masked color bitmap into a straight-alpha bitmap.
//
void CursorHelper::ConvertMaskToStraight(_Inout_ Bitmap &bitmap)
{
    for (DWORD cy = 0; cy < bitmap.Height; ++cy)
    {
        for (DWORD cx = 0; cx < bitmap.Width; ++cx)
        {
            BYTE alpha = bitmap.GetPixelAlpha(cx, cy);
            if (alpha != 0)
            {
                bitmap.SetPixelAlpha(cx, cy, 0);
                bitmap.SetPixelColor(cx, cy, 0);
            }
            else
            {
                bitmap.SetPixelAlpha(cx, cy, 0xFF);
            }
        }
    }
}

//
// Converts monochrome image planes into a 32-bit masked color bitmap. The mask
// is stored in the alpha bits.
//
void CursorHelper::ConvertMonochromeToColor(DWORD width, DWORD height, DWORD pitch, _In_reads_bytes_(height * pitch) const BYTE *sourceAnd,
    _In_reads_bytes_(height * pitch) const BYTE *sourceXor, _Inout_ Bitmap &outBitmap, bool preserveDest)
{
    if (!preserveDest)
    {
        outBitmap = Bitmap(width, height);
    }

    for (DWORD y = 0; y < height; y++)
    {
        for (DWORD x = 0; x < width; x++)
        {
            BOOL andValue = GetBitValue(sourceAnd, x, y, pitch);
            BOOL xorValue = GetBitValue(sourceXor, x, y, pitch);
            outBitmap.SetPixelColor(x, y, xorValue ? WHITE : BLACK);
            outBitmap.SetPixelAlpha(x, y, andValue ? XOR_ALPHA : 0);
        }
    }
}

bool CursorHelper::ContainsXor(const Bitmap &inputBitmap)
{
    for (DWORD y = 0; y < inputBitmap.Height; y++)
    {
        for (DWORD x = 0; x < inputBitmap.Width; x++)
        {
            if (inputBitmap.GetPixelColor(x, y) == WHITE &&
                inputBitmap.GetPixelAlpha(x, y))
            {
                return true;
            }
        }
    }
    return false;
}
