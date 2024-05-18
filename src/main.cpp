#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdint>

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
    enum class Type { Air, Sand, Bedrock };

    Color color{ PURPLE };
    Type type{ Type::Air };
};

constexpr int screenWidth{ 1200 };
constexpr int screenHeight{ 800 };

constexpr int gridScale{ 1 };
constexpr int brushSize{ 31 };
constexpr Vector2i gridSize{ screenWidth / gridScale, screenHeight / gridScale };

constexpr uint32_t particlesSize{ gridSize.x * gridSize.y };
Particle* particles{ new Particle[particlesSize]{} };

Particle particleBedrock{ BLACK, Particle::Type::Bedrock };
Particle particleSand{ GOLD, Particle::Type::Sand };
Particle particleAir{ PURPLE, Particle::Type::Air };


void HandleMouseButtonInput(MouseButton _mouseBttn, RenderTexture2D* _canvasChange);

Particle* GetParticlePtr(int x, int y)
{
    return &particles[y + gridSize.y * x];
};

void SwapParticles(int x0, int y0, int x1, int y1)
{
    std::swap(particles[y0 + gridSize.y * x0], particles[y1 + gridSize.y * x1]);
};

int main(void)
{
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    Font defaultFont{ GetFontDefault() };
    TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    RenderTexture2D canvas{ LoadRenderTexture(gridSize.x, gridSize.y) };
    RenderTexture2D canvasChanges{ LoadRenderTexture(gridSize.x, gridSize.y) };

    BeginTextureMode(canvas);
        ClearBackground(PURPLE);
    EndTextureMode();
    BeginTextureMode(canvasChanges);
        ClearBackground(PURPLE);
    EndTextureMode();

#if 0
    for (int i = 0; i < particlesSize; i++)
    {
        particles[i] = particleSand;
    }
#endif

    for (int y = 0; y < gridSize.y; y++)
    {
        *GetParticlePtr(0, y) = particleBedrock;
        *GetParticlePtr(gridSize.x-1, y) = particleBedrock;
    }
    for (int x = 0; x < gridSize.x; x++)
    {
        *GetParticlePtr(x, 0) = particleBedrock;
        *GetParticlePtr(x, gridSize.y-1) = particleBedrock;
    }

    constexpr uint8_t frameLimit{ 0 };
    uint8_t frameCounter{ frameLimit };
    while (!WindowShouldClose())
    {
        // if (++frameCounter > frameLimit)
        {
            frameCounter = 0;
            BeginTextureMode(canvasChanges);
                ClearBackground(BLANK);

                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    HandleMouseButtonInput(MOUSE_BUTTON_LEFT, &canvasChanges);
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                {
                    HandleMouseButtonInput(MOUSE_BUTTON_RIGHT, &canvasChanges);
                }

                Particle* column{ particles };
                for (int x = 0; x < gridSize.x; x++)
                {
                    for (int y = 0; y < gridSize.y; y++)
                    {
                        if (column[y].type == Particle::Type::Air)
                        {
                            continue;
                        }

                        struct PosAndColor{ Color col; int x{ -1 }; int y; } prevAndCurr[2];
                        switch (column[y].type)
                        {
                        case Particle::Type::Sand:
                            if (y != 0)
                            {
                                if (GetParticlePtr(x, y-1)->type == Particle::Type::Air)
                                {
                                    SwapParticles(x, y, x, y-1);
                                    prevAndCurr[0] = { GetParticlePtr(x, y)->color, x, y };
                                    prevAndCurr[1] = { GetParticlePtr(x, y-1)->color, x, y-1 };
                                }
                                else if (GetParticlePtr(x-1, y-1)->type == Particle::Type::Air && GetParticlePtr(x+1, y-1)->type == Particle::Type::Air)
                                {
                                    int dirs[]{ -1, 1 };
                                    int dir{ dirs[GetRandomValue(0, 1)] };
                                    GetParticlePtr(x, y)->color = dir == -1 ? RED : GREEN;
                                    SwapParticles(x, y, x+dir, y-1);
                                    prevAndCurr[0] = { GetParticlePtr(x, y)->color, x, y };
                                    prevAndCurr[1] = { GetParticlePtr(x+dir, y-1)->color, x+dir, y-1 };
                                }
                                else if (GetParticlePtr(x+1, y-1)->type == Particle::Type::Air)
                                {
                                    GetParticlePtr(x, y)->color = VIOLET;
                                    SwapParticles(x, y, x+1, y-1);
                                    prevAndCurr[0] = { GetParticlePtr(x, y)->color, x, y };
                                    prevAndCurr[1] = { GetParticlePtr(x+1, y-1)->color, x+1, y-1 };
                                }
                                else if (GetParticlePtr(x-1, y-1)->type == Particle::Type::Air)
                                {
                                    GetParticlePtr(x, y)->color = ORANGE;
                                    SwapParticles(x, y, x-1, y-1);
                                    prevAndCurr[0] = { GetParticlePtr(x, y)->color, x, y };
                                    prevAndCurr[1] = { GetParticlePtr(x-1, y-1)->color, x-1, y-1 };
                                }
                            }
                            break;
                        
                        default:
                            break;
                        }

                        if (prevAndCurr[0].x != -1)
                        {
                            DrawPixel(prevAndCurr[0].x, prevAndCurr[0].y, prevAndCurr[0].col);
                            DrawPixel(prevAndCurr[1].x, prevAndCurr[1].y, prevAndCurr[1].col);
                        }
                    }
                    column += gridSize.y;
                }
            EndTextureMode();

            BeginTextureMode(canvas);
                // "Pro" version is used to prevent from drawing image flipped on Y axis
                DrawTexturePro(canvasChanges.texture,
                    {0, (float)gridSize.y, (float)gridSize.x, (float)-gridSize.y},
                    {0, 0, (float)gridSize.x, (float)gridSize.y},
                    {0, 0},
                    0, WHITE
                );
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

// Fills in a particle buffer and also draws to "buffer of changes"
void HandleMouseButtonInput(MouseButton _mouseBttn, RenderTexture2D* _canvasChange)
{
    auto mousePos{ GetMousePosition() };
    if (mousePos.x <= 0 || mousePos.x >= screenWidth ||
        mousePos.y <= 0 || mousePos.y >= screenHeight)
    {
        return;
    }

    Vector2i center{ (int)mousePos.x / gridScale, gridSize.y - (int)mousePos.y / gridScale };
    Particle newParticle{};
    switch (_mouseBttn)
    {
        case MOUSE_BUTTON_LEFT: newParticle = particleSand;
            break;
        case MOUSE_BUTTON_RIGHT: newParticle = particleAir;
            break;
    }

    // Clamp in case if mouse is out of window or close to the edges
    // Also considering bedrock border
    Rectangle area {
        Clamp(center.x - brushSize / 2, 1, gridSize.x - 2),
        Clamp(center.y - brushSize / 2, 1, gridSize.y - 2),
        Clamp(center.x + brushSize / 2, 1, gridSize.x - 2),
        Clamp(center.y + brushSize / 2, 1, gridSize.y - 2),
    };

    // Update particle buffer and draw to "buffer of changes"
    for (int y = area.y; y <= area.height; y++)
    {
        for (int x = area.x; x <= area.width; x++)
        {
            auto particle{ GetParticlePtr(x, y) };
            if (particle->type != Particle::Type::Bedrock)
            {
                *particle = newParticle;
                DrawPixel(x, y, newParticle.color);
            }
        }
    }
}