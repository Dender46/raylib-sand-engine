#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdint>

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "textWithPivot.hpp"
#define MY_BRUSH_IMPLEMENTATION
#include "brush.hpp"

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
constexpr Vector2i gridSize{ screenWidth / gridScale, screenHeight / gridScale };

constexpr uint32_t particlesSize{ gridSize.x * gridSize.y };
Particle* particles{ new Particle[particlesSize]{} };
Color* pixelChanges{ new Color[particlesSize]{} };

Particle particleBedrock{ BLACK, Particle::Type::Bedrock };
Particle particleSand{ GOLD, Particle::Type::Sand };
Particle particleAir{ PURPLE, Particle::Type::Air };


void HandleMouseButtonInput(MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange);

Particle* GetParticlePtr(int x, int y)
{
    return &particles[y + gridSize.y * x];
};

void SwapParticles(int x0, int y0, int x1, int y1)
{
    std::swap(particles[y0 + gridSize.y * x0], particles[y1 + gridSize.y * x1]);
};

void SetPixelChange(int x, int y, Color color)
{
    pixelChanges[x + gridSize.x * y] = color;
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    Font defaultFont{ GetFontDefault() };
    TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    RenderTexture2D canvas{ LoadRenderTexture(gridSize.x, gridSize.y) };
    RenderTexture2D canvasChanges{ LoadRenderTexture(gridSize.x, gridSize.y) };

    Brush brush{ 31, gridScale };

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

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                HandleMouseButtonInput(MOUSE_BUTTON_LEFT, brush, &canvasChanges);
            }
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                HandleMouseButtonInput(MOUSE_BUTTON_RIGHT, brush, &canvasChanges);
            }

            if (float mwDiff{ GetMouseWheelMove() }; mwDiff != 0)
            {
                brush.Resize(Clamp(brush.mSize + mwDiff * 5.0f, 1.0f, 100.0f), gridScale);
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

                    switch (column[y].type)
                    {
                    case Particle::Type::Sand:
                        if (y != 0)
                        {
                            if (GetParticlePtr(x, y-1)->type == Particle::Type::Air)
                            {
                                SwapParticles(x, y, x, y-1);
                                SetPixelChange(x, y, GetParticlePtr(x, y)->color);
                                SetPixelChange(x, y-1, GetParticlePtr(x, y-1)->color);
                            }
                            else if (GetParticlePtr(x-1, y-1)->type == Particle::Type::Air && GetParticlePtr(x+1, y-1)->type == Particle::Type::Air)
                            {
                                int dirs[]{ -1, 1 };
                                int dir{ dirs[GetRandomValue(0, 1)] };
                                SwapParticles(x, y, x+dir, y-1);
                                SetPixelChange(x, y, GetParticlePtr(x, y)->color);
                                SetPixelChange(x+dir, y-1, GetParticlePtr(x+dir, y-1)->color);
                            }
                            else if (GetParticlePtr(x+1, y-1)->type == Particle::Type::Air)
                            {
                                SwapParticles(x, y, x+1, y-1);
                                SetPixelChange(x, y, GetParticlePtr(x, y)->color);
                                SetPixelChange(x+1, y-1, GetParticlePtr(x+1, y-1)->color);
                            }
                            else if (GetParticlePtr(x-1, y-1)->type == Particle::Type::Air)
                            {
                                SwapParticles(x, y, x-1, y-1);
                                SetPixelChange(x, y, GetParticlePtr(x, y)->color);
                                SetPixelChange(x-1, y-1, GetParticlePtr(x-1, y-1)->color);
                            }
                        }
                        break;
                    
                    default:
                        break;
                    }
                }
                column += gridSize.y;
            }

            UpdateTexture(canvasChanges.texture, pixelChanges);

            BeginTextureMode(canvas);
                DrawTexture(canvasChanges.texture, 0, 0, WHITE);
            EndTextureMode();
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            XAxisLabel.Draw(screenWidth / 2, screenHeight, GRAY);
            YAxisLabel.Draw(0, screenHeight / 2, -90.0f, GRAY);

            DrawTextureEx(canvas.texture, {0, 0}, 0, gridScale, WHITE);

            brush.DrawOutline(GetMousePosition(), gridScale);

            static Color testingColor{ WHITE };
            static float testingColorBrig{ 0.0f };
            auto colNormalized{ ColorNormalize(testingColor) };
            GuiSlider({10.0f, 10.0f, 100.0f, 30.0f}, "r", nullptr, &colNormalized.x, 0.0f, 1.0f);
            GuiSlider({10.0f, 40.0f, 100.0f, 30.0f}, "g", nullptr, &colNormalized.y, 0.0f, 1.0f);
            GuiSlider({10.0f, 70.0f, 100.0f, 30.0f}, "b", nullptr, &colNormalized.z, 0.0f, 1.0f);
            GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "a", nullptr, &colNormalized.w, 0.0f, 1.0f);
            GuiSlider({10.0f, 130.0f, 100.0f, 30.0f}, "brig", nullptr, &testingColorBrig, -1.0f, 1.0f);
            testingColor = ColorFromNormalized(colNormalized);
            // GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "tint", nullptr, &colNormalized.a, 0.0f, 1.0f);
            DrawRectangle(10.0f, 160.0f, 100.0f, 100.0f, ColorBrightness(testingColor, testingColorBrig));

            DrawFPS(20, 20);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

// Fills in a particle buffer and also draws to "buffer of changes"
void HandleMouseButtonInput(MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange)
{
    auto mousePos{ GetMousePosition() };
    if (mousePos.x <= 0 || mousePos.x >= screenWidth ||
        mousePos.y <= 0 || mousePos.y >= screenHeight)
    {
        return;
    }

    Vector2i center{ (int)mousePos.x / gridScale, gridSize.y - (int)mousePos.y / gridScale };
    Particle particleTemplate;
    switch (_mouseBttn)
    {
        case MOUSE_BUTTON_LEFT: particleTemplate = particleSand;
            break;
        case MOUSE_BUTTON_RIGHT: particleTemplate = particleAir;
            break;
    }

    // Clamp in case if mouse is out of window or close to the edges
    // Also considering bedrock border
    Rectangle area {
        Clamp(center.x - _brush.mSize / 2, 1, gridSize.x - 2),
        Clamp(center.y - _brush.mSize / 2, 1, gridSize.y - 2),
        Clamp(center.x + _brush.mSize / 2, 1, gridSize.x - 2),
        Clamp(center.y + _brush.mSize / 2, 1, gridSize.y - 2),
    };

    // Update particle buffer and draw to "buffer of changes"
    for (int y = area.y; y <= area.height; y++)
    {
        for (int x = area.x; x <= area.width; x++)
        {
            auto particle{ GetParticlePtr(x, y) };
            if (particle->type != Particle::Type::Bedrock)
            {
                Particle newParticle{ particleTemplate };

                if (particleTemplate.type != Particle::Type::Air && GetRandomValue(0, 100) < 20)
                {
                    newParticle.color = ColorBrightness(newParticle.color, GetRandomValue(-10, 0) * 0.01);
                }

                *particle = newParticle;
                SetPixelChange(x, y, newParticle.color);
            }
        }
    }
}