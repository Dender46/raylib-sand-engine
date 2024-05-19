#pragma once

#include "raylib.h"

struct TextWithPivot
{
public:
    TextWithPivot(Font _font, const char* _label, Vector2 _pivot, int _fontSize)
        : label(_label), pivot(_pivot), fontSize(_fontSize), size(MeasureTextEx(_font, _label, _fontSize, 2))
    {}

    void Draw(int posX, int posY, Color color)
    {
        DrawText(label, posX - (size.x * pivot.x), posY - (size.y * pivot.y), fontSize, color);
    }

    void Draw(int posX, int posY, float rotation, Color color)
    {
        Vector2 pos{ (float)posX, (float)posY };
        Vector2 pivotSizeBased{ 
            size.x * pivot.x,
            size.y * pivot.y,
        };
        DrawTextPro(defaultFont, label, pos, pivotSizeBased, rotation, fontSize, 1.0f, color);
    }

    const Font defaultFont{ GetFontDefault() };
    const char* label;
    const int fontSize;
    Vector2 pivot;
    const Vector2 size;
};