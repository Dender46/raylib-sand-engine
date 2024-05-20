#pragma once
#include <cstdint>

#include "raylib.h"

struct Brush
{
    Brush(uint16_t _size, int _gridScale);
    void DrawOutline(Vector2 _pos, int _gridScale);

    void Resize(uint16_t _newSize, int _gridScale);

    uint16_t mSize;
    Rectangle mArea;
    Image mImage;
};



#ifdef MY_BRUSH_IMPLEMENTATION

Brush::Brush(uint16_t _size, int _gridScale)
    : mSize{ _size },
      mArea{ 0.0f, 0.0f, _size * (float)_gridScale, _size * (float)_gridScale },
      mImage{ GenImageColor(_size, _size, BLANK) }
{}

void Brush::DrawOutline(Vector2 _pos, int _gridScale)
{
    mArea.x = _pos.x - mSize * _gridScale / 2;
    mArea.y = _pos.y - mSize * _gridScale / 2;
    DrawRectangleLinesEx(mArea, _gridScale, RED);
}

void Brush::Resize(uint16_t _newSize, int _gridScale)
{
    mSize = _newSize;
    mArea = { 0.0f, 0.0f, _newSize * (float)_gridScale, _newSize * (float)_gridScale };
    UnloadImage(mImage);
    mImage = GenImageColor(_newSize, _newSize, BLANK);
}

#endif