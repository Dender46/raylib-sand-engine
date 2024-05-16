#include <iostream>
#include <cmath>
#include <cstdio>

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
// #include "raygui.h"

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

struct Vector2i
{
    int x;
    int y;
};

struct Particle
{
    enum class Type { Air, Sand };

    Color color{ PURPLE };
    Type type{ Type::Air };
};

int main(void)
{
    constexpr int screenWidth{ 800 };
    constexpr int screenHeight{ 400 };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    Font defaultFont{ GetFontDefault() };
    TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    constexpr int gridScale{ 8 };
    constexpr Vector2i gridSize{ screenWidth / gridScale, screenHeight / gridScale };
    printf("grid-size: x:%d y:%d\n", gridSize.x, gridSize.y);

    RenderTexture2D canvas{ LoadRenderTexture(gridSize.x, gridSize.y) };

    constexpr uint16_t particlesSize{ gridSize.x * gridSize.y };
    Particle particles[particlesSize]{};
    auto GetParticlePtr = [&particles, &gridSize](int x, int y){ 
        return &particles[x + gridSize.x * y];
    };

    GetParticlePtr(gridSize.x >> 1, (gridSize.y >> 1))->type = Particle::Type::Sand;
    GetParticlePtr(gridSize.x >> 1, (gridSize.y >> 1))->color = GOLD;

    constexpr uint8_t frameLimit{ 1 };
    uint8_t frameCounter{ frameLimit };
    while (!WindowShouldClose())
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            auto mousePos{ GetMousePosition() };
            GetParticlePtr(mousePos.x / gridScale, gridSize.y - mousePos.y / gridScale)->type = Particle::Type::Sand;
            GetParticlePtr(mousePos.x / gridScale, gridSize.y - mousePos.y / gridScale)->color = GOLD;
        }

        if (++frameCounter > frameLimit)
        {
            frameCounter = 0;
            BeginTextureMode(canvas);

                Particle* line{ particles };
                for (int y = 0; y < gridSize.y; y++)
                {
                    for (int x = 0; x < gridSize.x; x++)
                    {
                        // Draw before particle is updated and moved to different place
                        DrawPixel(x, y, line[x].color);

                        switch (line[x].type)
                        {
                        case Particle::Type::Sand:
                            if (y == 0 || GetParticlePtr(x, y-1)->type == Particle::Type::Sand)
                                break;
                            GetParticlePtr(x, y-1)->type = Particle::Type::Sand;
                            GetParticlePtr(x, y-1)->color = GOLD;
                            line[x].type = Particle::Type::Air;
                            line[x].color = PURPLE;
                            break;
                        
                        default:
                            break;
                        }
                    }
                    line += gridSize.x;
                }
            EndTextureMode();
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            XAxisLabel.Draw(screenWidth / 2, screenHeight, GRAY);
            YAxisLabel.Draw(0, screenHeight / 2, -90.0f, GRAY);

            DrawTextureEx(canvas.texture, {0, 0}, 0, gridScale, WHITE);

            DrawFPS(20, 20);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}