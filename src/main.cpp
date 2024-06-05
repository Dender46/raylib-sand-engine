#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdint>

#include "raylib.h"
#include "raymath.h"
// #define RAYGUI_IMPLEMENTATION
// #include "raygui.h"

#include "types.hpp"
#include "particle.hpp"
#include "textWithPivot.hpp"
#include "brush.hpp"
#include "profiller.hpp"

struct Vector2i
{
    int x;
    int y;
};


constexpr int screenWidth{ 1200 };
constexpr int screenHeight{ 600 };

constexpr int gridScale{ 1 };
constexpr Vector2i gridSize{ screenWidth / gridScale, screenHeight / gridScale };

constexpr uint32_t particlesSize{ gridSize.x * gridSize.y };
Particle* particles{ new Particle[particlesSize]{} };
Color* pixelChanges{ new Color[particlesSize]{} };

Particle particleBedrock{ Particle::Type::Bedrock };
Particle particleSand{ Particle::Type::Sand };
Particle particleAir{ Particle::Type::Air };

RenderTexture2D canvas;
RenderTexture2D canvasChanges;

void HandleMouseButtonInput(Vector2 _prevMousePos, Vector2 _currMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange);
void HandleKeyboardInput(Brush* _brush);
void ProcessParticle(const Particle& particle, u16 x, u16 y);

Particle* GetParticlePtr(int x, int y)
{
    return &particles[y + gridSize.y * x];
};

void SetPixelChange(int x, int y, Color color)
{
    pixelChanges[x + gridSize.x * y] = color;
}

void SetParticle(int x, int y, Particle p)
{
    *GetParticlePtr(x, y) = p;
    SetPixelChange(x, y, p.color);
}

void SwapParticles(int x0, int y0, int x1, int y1)
{
    auto& p0{ particles[y0 + gridSize.y * x0] };
    auto& p1{ particles[y1 + gridSize.y * x1] };
    std::swap(p0, p1);
    SetPixelChange(x0, y0, p0.color);
    SetPixelChange(x1, y1, p1.color);
};

void Init()
{
    canvas = LoadRenderTexture(gridSize.x, gridSize.y);
    canvasChanges = LoadRenderTexture(gridSize.x, gridSize.y);

    BeginTextureMode(canvas);
        ClearBackground(PURPLE);
    EndTextureMode();
    BeginTextureMode(canvasChanges);
        ClearBackground(PURPLE);
    EndTextureMode();

#if 0
    for (int x = 0; x < gridSize.x; x++)
        for (int y = 0; y < gridSize.y; y++)
            SetParticle(x, y, { Particle::Type::Sand });
#endif

    for (int y = 0; y < gridSize.y; y++)
    {
        SetParticle(0, y, particleBedrock);
        SetParticle(gridSize.x-1, y, particleBedrock);
    }
    for (int x = 0; x < gridSize.x; x++)
    {
        SetParticle(x, 0, particleBedrock);
        SetParticle(x, gridSize.y-1, particleBedrock);
    }
}



int main(void)
{
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);
    Init();

    //Font defaultFont{ GetFontDefault() };
    //TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    //TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    Brush brush{ 1, gridScale };

    constexpr u8 frameLimit{ 0 };
    u8 frameCounter{ frameLimit };
    Vector2 prevMousePosition{ GetMousePosition() };

    SetParticle(gridSize.x / 2 + 20, gridSize.y / 2  + 30, { Particle::Type::Sand });
    Particle* target{ GetParticlePtr(gridSize.x / 2, gridSize.y / 2) };
    target->velX = 1;
    target->velY = 1;

    Profiller::globalProfiller.BeginProfilling(" ::::: PROFILLER ::::: ");

    u64 frameCount{ 0 };
    u8 stepCount{ 1 };
    while (!WindowShouldClose())
    {
        TIME_BANDWIDTH("Main loop", 0);

        if (IsKeyPressed(KEY_S))
            stepCount++;

        HandleKeyboardInput(&brush);
        // if (stepCount > 0)
        {
            stepCount = 0;
            frameCounter = 0;

            auto mousePos{ GetMousePosition() };
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                HandleMouseButtonInput(prevMousePosition, mousePos, MOUSE_BUTTON_LEFT, brush, &canvasChanges);
            }
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                HandleMouseButtonInput(prevMousePosition, mousePos, MOUSE_BUTTON_RIGHT, brush, &canvasChanges);
            }
            prevMousePosition = mousePos;

            if (float mwDiff{ GetMouseWheelMove() }; mwDiff != 0)
            {
                brush.Resize(Clamp(brush.mSize + mwDiff * 5.0f, 1.0f, 100.0f), gridScale);
            }

            Particle* column{ particles };
            for (u16 x{0}; x < gridSize.x; x++)
            {
                for (u16 y{0}; y < gridSize.y; y++)
                {
                    auto& particle{ column[y] };
                    ProcessParticle(particle, x, y);
                }
                column += gridSize.y;
            }

            UpdateTexture(canvasChanges.texture, pixelChanges);

            {
            TIME_BANDWIDTH("Update canvas", 0);
            BeginTextureMode(canvas);
                DrawTexture(canvasChanges.texture, 0, 0, WHITE);
            EndTextureMode();
            }
        }

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            DrawTextureEx(canvas.texture, {0, 0}, 0, gridScale, WHITE);

            brush.DrawOutline(GetMousePosition(), gridScale);

            static Color testingColor{ WHITE };
            static float testingColorBrig{ 0.0f };
            auto colNormalized{ ColorNormalize(testingColor) };
            //GuiSlider({10.0f, 10.0f, 100.0f, 30.0f}, "r", nullptr, &colNormalized.x, 0.0f, 1.0f);
            //GuiSlider({10.0f, 40.0f, 100.0f, 30.0f}, "g", nullptr, &colNormalized.y, 0.0f, 1.0f);
            //GuiSlider({10.0f, 70.0f, 100.0f, 30.0f}, "b", nullptr, &colNormalized.z, 0.0f, 1.0f);
            //GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "a", nullptr, &colNormalized.w, 0.0f, 1.0f);
            //GuiSlider({10.0f, 130.0f, 100.0f, 30.0f}, "brig", nullptr, &testingColorBrig, -1.0f, 1.0f);
            //testingColor = ColorFromNormalized(colNormalized);
            ////GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "tint", nullptr, &colNormalized.a, 0.0f, 1.0f);
            //DrawRectangle(10.0f, 160.0f, 100.0f, 100.0f, ColorBrightness(testingColor, testingColorBrig));

            DrawFPS(20, 20);
        }
        EndDrawing();
    }

    Profiller::globalProfiller.EndProfilling();

    CloseWindow();

    return 0;
}

// Fills in a particle buffer and also draws to "buffer of changes"
void HandleMouseButtonInput(Vector2 _prevMousePos, Vector2 _currMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange)
{
    if (_currMousePos.x <= 0 || _currMousePos.x >= screenWidth ||
        _currMousePos.y <= 0 || _currMousePos.y >= screenHeight)
    {
        return;
    }

    TIME_FUNCTION;

    Particle::Type newParticleType;
    switch (_mouseBttn)
    {
        case MOUSE_BUTTON_LEFT: newParticleType = _brush.mDrawType;
            break;
        case MOUSE_BUTTON_RIGHT: newParticleType = Particle::Type::Air;
            break;
    }

    int pathSize{ 32 };
    for (int i = 0; i < pathSize; i++)
    {
        float lerpT{ (float)i / pathSize };
        float mouseCenterX{ Lerp(_prevMousePos.x, _currMousePos.x, lerpT) };
        float mouseCenterY{ Lerp(_prevMousePos.y, _currMousePos.y, lerpT) };

        Vector2i center{ (int)mouseCenterX / gridScale, gridSize.y - (int)mouseCenterY / gridScale };

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
                auto particleTarget{ GetParticlePtr(x, y) };
                if (particleTarget->props & (u16)Particle::Props::NonDestruct)
                {
                    continue;
                }
                SetParticle(x, y, { newParticleType });
            }
        }
    }
}

void HandleKeyboardInput(Brush* _brush)
{
    if (IsKeyPressed(KEY_ONE))
    {
        _brush->mDrawType = Particle::Type::Sand;
    }
    else if (IsKeyPressed(KEY_TWO))
    {
        _brush->mDrawType = Particle::Type::Water;
    }
    else if (IsKeyPressed(KEY_THREE))
    {
        _brush->mDrawType = Particle::Type::Rock;
    }
}

void ProcessParticle(const Particle& particle, u16 x, u16 y)
{
    TIME_FUNCTION;
    switch (particle.type)
    {
    case Particle::Type::Sand:
        if (y != 0)
        {
            if (auto p{ GetParticlePtr(x, y-1)->props}; (p & Particle::Props::Liquid) || !p)
            {
                SwapParticles(x, y, x, y-1);
            }
            else if (auto p{GetParticlePtr(x-1, y-1)->props}; (p & Particle::Props::Liquid) || !p)
            {
                SwapParticles(x, y, x-1, y-1);
            }
            else if (auto p{GetParticlePtr(x+1, y-1)->props}; (p & Particle::Props::Liquid) || !p)
            {
                SwapParticles(x, y, x+1, y-1);
            }
        }
        break;
    case Particle::Type::Water:
        if (y != 0)
        {
            if (GetParticlePtr(x, y-1)->props == 0)
            {
                SwapParticles(x, y, x, y-1);
            }
            else if (GetParticlePtr(x+1, y-1)->props == 0)
            {
                SwapParticles(x, y, x+1, y-1);
            }
            else if (GetParticlePtr(x-1, y-1)->props == 0)
            {
                SwapParticles(x, y, x-1, y-1);
            }
            else if (GetParticlePtr(x-1, y)->props == 0 && GetParticlePtr(x+1, y)->props == 0)
            {
                i16 dir{ ((i16[]){-1, 1})[GetRandomValue(0, 1)] };
                SwapParticles(x, y, x+dir, y);
            }
            else if (GetParticlePtr(x-1, y)->props == 0)
            {
                SwapParticles(x, y, x-1, y);
            }
            else if (GetParticlePtr(x+1, y)->props == 0)
            {
                SwapParticles(x, y, x+1, y);
            }
        }
        break;
    case Particle::Type::Air:
    case Particle::Type::Rock:
    case Particle::Type::Bedrock:
        break;
    default:
        assert(false); // Unknown particle type
        break;
    }
}
