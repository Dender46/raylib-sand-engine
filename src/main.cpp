#include <iostream>
#include <cmath>

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

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

int main(void)
{
    const int screenWidth{ 800 };
    const int screenHeight{ 450 };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    auto defaultFont{ GetFontDefault() };
    TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    float rotation{};
    while (!WindowShouldClose())
    {
        BeginDrawing();

            ClearBackground(RAYWHITE);

            XAxisLabel.Draw(screenWidth / 2, screenHeight, GRAY);
            YAxisLabel.Draw(0, screenHeight / 2, -90.0f, GRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}