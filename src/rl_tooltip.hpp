#include "raylib.h"

namespace RLExt{

void Tooltip(Vector2 pos, char* text, Vector2 screenSize)
{
    auto bgColor{ Fade(BLACK, 0.6f) };
    auto fontSize{ 18 };

    auto size{ MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10) };
    size.x += 30;
    size.y += 5;

    pos.x = Clamp(pos.x, 0.0f, screenSize.x - size.x);
    pos.y = Clamp(pos.y, 0.0f, screenSize.y - size.y);

    DrawRectangle(pos.x, pos.y, size.x, size.y, bgColor);
    DrawText(text, pos.x + 15, pos.y + 15, fontSize, WHITE);
}

}