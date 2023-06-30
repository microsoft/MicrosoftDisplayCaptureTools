export module Bitmap;

import "pch.h";

//
// Gets the value of a pixel in a 1bpp bitmap
//
//   bitmap - A pointer to the bitmap data
//
//   x - The X coordinate of the pixel
//
//   y - The Y coordinate of the pixel
//
//   pitch - The width of a single scanline, in bytes
//
export inline BOOL GetBitValue(_In_reads_bytes_(pitch*(y + 1)) const BYTE* bitmap, _In_range_(0, pitch * 8 - 1) DWORD x, DWORD y, LONG pitch)
{
    return bitmap[y * pitch + (x / 8)] & ((BYTE)0x80 >> (x % 8));
}

//
// Sets the value of a pixel in a 1bpp bitmap
//
//   bitmap - A pointer to the bitmap data
//
//   x - The X coordinate of the pixel
//
//   y - The Y coordinate of the pixel
//
//   pitch - The width of a single scanline, in bytes
//
export inline void SetBitValue(_Inout_bytecap_(pitch*(y + 1)) PBYTE bitmap, _In_range_(0, pitch * 8 - 1) LONG x, LONG y, LONG pitch, BOOL value)
{
    if (value)
        bitmap[y * pitch + (x / 8)] |= ((BYTE)0x80 >> (x % 8));
    else
        bitmap[y * pitch + (x / 8)] &= ~((BYTE)0x80 >> (x % 8));
}

//
// Encapsulates a 32-bit ARGB or ABGR bitmap.
//
export class Bitmap
{
public:
    DWORD Width;
    DWORD Height;
    PBYTE Bits;
    DWORD Pitch;

    // If this bitmap allocated the buffer, it will release it in the destructor.
    bool OwnsBuffer;

    Bitmap();

    // Constructs a bitmap by allocating a buffer
    Bitmap(DWORD width, DWORD height);

    // Constructs a bitmap by wrapping an existing buffer. Ownership of the
    // buffer remains with the caller.
    Bitmap(DWORD width, DWORD height, DWORD pitch, PBYTE buffer);

    ~Bitmap();

    Bitmap(const Bitmap& copy);

    Bitmap(Bitmap&& move);

    // Support C++11 move semantics to prevent spurious buffer copies
    Bitmap& operator=(Bitmap&& move);
    Bitmap& operator=(const Bitmap& copy);
    bool operator==(const Bitmap& other) const;
    bool operator!=(const Bitmap& other) const;

    // Gets just the alpha byte of a specified pixel
    BYTE GetPixelAlpha(DWORD x, DWORD y) const;

    // Sets just the alpha byte of a specified pixel
    void SetPixelAlpha(DWORD x, DWORD y, BYTE alpha);

    // Gets the color of a pixel, not including the alpha
    DWORD GetPixelColor(DWORD x, DWORD y) const;

    // Sets the color of the pixel, not including the alpha
    void SetPixelColor(DWORD x, DWORD y, DWORD rgb);

    // Sets the alpha and color values for all pixels
    void Clear(BYTE alpha, DWORD color);

    // Returns true if any pixel contains a non-zero alpha, false otherwise
    bool ContainsNonZeroAlpha();

    void Destroy();
};

module : private;

Bitmap::Bitmap()
{
    Width = 0;
    Height = 0;
    Bits = nullptr;
    Pitch = 0;
    OwnsBuffer = false;
}

// Constructs a bitmap by allocating a buffer

Bitmap::Bitmap(DWORD width, DWORD height)
{
    Width = width;
    Height = height;
    Pitch = width * sizeof(DWORD);
    Bits = new BYTE[Pitch * height];
    OwnsBuffer = true;
}

// Constructs a bitmap by wrapping an existing buffer. Ownership of the
// buffer remains with the caller.

Bitmap::Bitmap(DWORD width, DWORD height, DWORD pitch, PBYTE buffer)
{
    OwnsBuffer = false;
    Width = width;
    Height = height;
    Pitch = pitch;
    Bits = buffer;
}

Bitmap::~Bitmap()
{
    Destroy();
}

Bitmap::Bitmap(const Bitmap& copy)
{
    Width = copy.Width;
    Height = copy.Height;
    Pitch = copy.Pitch;
    OwnsBuffer = true;

    if (copy.Bits)
    {
        // Always copy to a new buffer
        Bits = new BYTE[copy.Pitch * copy.Height];
        memcpy(Bits, copy.Bits, copy.Pitch * copy.Height);
    }
}

Bitmap::Bitmap(Bitmap&& move)
{
    *this = std::move(move);
}

// Support C++11 move semantics to prevent spurious buffer copies
Bitmap& Bitmap::operator=(Bitmap&& move)
{
    if (this != &move)
    {
        if (Bits && OwnsBuffer)
        {
            delete[] Bits;
            Bits = nullptr;
        }

        Width = move.Width;
        Height = move.Height;
        Bits = move.Bits;
        Pitch = move.Pitch;
        OwnsBuffer = move.OwnsBuffer;

        move.Bits = nullptr;
        move.Width = 0;
        move.Height = 0;
        move.Pitch = 0;
        move.OwnsBuffer = false;
    }
    return *this;
}

Bitmap& Bitmap::operator=(const Bitmap& copy)
{
    Destroy();
    Width = copy.Width;
    Height = copy.Height;
    Pitch = copy.Pitch;

    if (copy.Bits)
    {
        OwnsBuffer = true;
        Bits = new BYTE[Height * Pitch];
        memcpy(Bits, copy.Bits, Height * Pitch);
    }

    return *this;
}

bool Bitmap::operator==(const Bitmap& other) const
{
    if (Width != other.Width || Height != other.Height)
    {
        return false;
    }

    for (DWORD y = 0; y < Height; y++)
    {
        for (DWORD x = 0; x < Width; x++)
        {
            if (other.GetPixelColor(x, y) != GetPixelColor(x, y) || other.GetPixelAlpha(x, y) != GetPixelAlpha(x, y))
            {
                return false;
            }
        }
    }

    return true;
}

bool Bitmap::operator!=(const Bitmap& other) const
{
    return !(*this == other);
}

// Gets just the alpha byte of a specified pixel

BYTE Bitmap::GetPixelAlpha(DWORD x, DWORD y) const
{
    assert(x < Width && y < Height);

    return Bits[y * Pitch + x * sizeof(DWORD) + 3];
}

// Sets just the alpha byte of a specified pixel

void Bitmap::SetPixelAlpha(DWORD x, DWORD y, BYTE alpha)
{
    assert(x < Width && y < Height);

    Bits[y * Pitch + x * sizeof(DWORD) + 3] = alpha;
}

// Gets the color of a pixel, not including the alpha

DWORD Bitmap::GetPixelColor(DWORD x, DWORD y) const
{
    assert(x < Width && y < Height);

    DWORD color = *(PDWORD)&Bits[y * Pitch + x * sizeof(DWORD)];
    return color & 0xFFFFFF;
}

// Sets the color of the pixel, not including the alpha

void Bitmap::SetPixelColor(DWORD x, DWORD y, DWORD rgb)
{
    assert(x < Width && y < Height);

    PDWORD color = (PDWORD)&Bits[y * Pitch + x * sizeof(DWORD)];
    *color = (*color & 0xFF000000) | (rgb & 0xFFFFFF);
}

// Sets the alpha and color values for all pixels

void Bitmap::Clear(BYTE alpha, DWORD color)
{
    for (DWORD y = 0; y < Height; y++)
    {
        for (DWORD x = 0; x < Width; x++)
        {
            SetPixelAlpha(x, y, alpha);
            SetPixelColor(x, y, color);
        }
    }
}

// Returns true if any pixel contains a non-zero alpha, false otherwise

bool Bitmap::ContainsNonZeroAlpha()
{
    for (DWORD y = 0; y < Height; y++)
    {
        for (DWORD x = 0; x < Width; x++)
        {
            if (GetPixelAlpha(x, y) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

void Bitmap::Destroy()
{
    if (Bits && OwnsBuffer)
    {
        delete[] Bits;
        Bits = nullptr;
        Width = 0;
        Height = 0;
        Pitch = 0;
    }
}
