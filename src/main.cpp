#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdint>

#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include "util.hpp"
#include "particle.hpp"
#include "textWithPivot.hpp"
#include "brush.hpp"
#include "simStepper.hpp"
#define PROFILLER 0
#include "profiler/profiller.hpp"
#include "profiler/profilerRLDisplay.hpp"

constexpr i32 screenWidth{ 1200 };
constexpr i32 screenHeight{ 645 };

constexpr u32 canvasWidth{ 1200 };
constexpr u32 canvasHeight{ 600 };

constexpr u32 gridScale{ 50 };
constexpr Vector2i gridSize{ canvasWidth / gridScale, canvasHeight / gridScale };

constexpr Rectangle hotbarWorldRec{
    0, 0,
    screenWidth, screenHeight - canvasHeight
};
constexpr Rectangle canvasWorldRec{
    0, screenHeight - canvasHeight,
    canvasWidth, canvasHeight
};

constexpr u32 particlesSize{ gridSize.x * gridSize.y };
Particle* particles{ new Particle[particlesSize]{} };
Color* pixelChanges{ new Color[particlesSize]{} };

Particle particleBedrock{ Particle::Type::Bedrock };
Particle particleSand{ Particle::Type::Sand };
Particle particleAir{ Particle::Type::Air };

RenderTexture2D canvas;
RenderTexture2D canvasChanges;

void OneFrameProcessing();
void SimStepperProcessing(SimStepper* simStepper);
Vector2i GetMousePositionGrid();
void HandleMouseButtonInput(Vector2 _currMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange);
void HandleKeyboardInput(Brush* _brush);
void ProcessParticle(Particle& particle, u16 x, u16 y);

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

Vector2i GetMousePositionGrid(Vector2 mousePos)
{
    return {
        (i32)mousePos.x / (i32)gridScale,
        gridSize.y - (i32)mousePos.y / (i32)gridScale
    };
}

void InitSimulation()
{
    canvas = LoadRenderTexture(gridSize.x, gridSize.y);
    canvasChanges = LoadRenderTexture(gridSize.x, gridSize.y);

    BeginTextureMode(canvas);
        ClearBackground(PURPLE);
    EndTextureMode();
    BeginTextureMode(canvasChanges);
        ClearBackground(PURPLE);
    EndTextureMode();

// Fill entire canvas with screen
#if 0
    for (int x = 0; x < gridSize.x; x++)
        for (int y = 0; y < gridSize.y; y++)
            SetParticle(x, y, { Particle::Type::Sand });
#endif

// Top - emitters, bottom - deleters
#if 1
    for (int x = 0; x < gridSize.x; x++)
    {
        for (int y = 0; y < gridSize.y; y++)
        {
            using enum Particle::Type;
            Particle::Type p{ x % 2 ? Sand : Water };
            SetParticle(x, y, { p });
        }
    }
    for (int x = 0; x < gridSize.x; x++)
    {
        using enum Particle::Type;
        Particle::Type emittingType{ x % 2 ? Sand : Water };
        SetParticle(x, gridSize.y-2, { Emitter, (u32)emittingType });
    }
    for (int x = 0; x < gridSize.x; x++)
        SetParticle(x, 1, { Particle::Type::Deleter });
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
    InitSimulation();

    //Font defaultFont{ GetFontDefault() };
    //TextWithPivot XAxisLabel(defaultFont, "X Axis", {0.5f, 1.0f}, 28);
    //TextWithPivot YAxisLabel(defaultFont, "Y Axis", {0.5f, 0.0f}, 28);

    Brush brush{ 1, gridScale };
    SimStepper simStepper;

#if 0
    {
        SetParticle(gridSize.x / 2 + 20, gridSize.y / 2  + 30, { Particle::Type::Sand });
        Particle* target{ GetParticlePtr(gridSize.x / 2, gridSize.y / 2) };
        target->velX = 1;
        target->velY = 1;
    }
#endif

    Profiller::globalProfiller.BeginProfilling(" ::::: PROFILLER ::::: ");
    u32 profillingFramesLimit{ 100 };
    u32 profillingFramesCount{ 1 };

    while (!WindowShouldClose())
    {
#if PROFILLER
        if (profillingFramesCount++ > profillingFramesLimit)
        {
            break;
        }
#endif // PROFILLER
        TIME_FUNCTION;

        auto mousePos{ GetMousePosition() };
        HandleKeyboardInput(&brush);

        if (!simStepper.mIsPaused)
        {
            TIME_BANDWIDTH("Processing", particlesSize * sizeof(Particle));
            OneFrameProcessing();
        }
        else if (simStepper.mStepNextParticles != 0)
        {
            while (simStepper.mStepNextParticles != 0)
                SimStepperProcessing(&simStepper);
        }
        else if (simStepper.mStepNextFrame != 0)
        {
            do {
                OneFrameProcessing();
            } while (--simStepper.mStepNextFrame != 0);
        }

        UpdateTexture(canvasChanges.texture, pixelChanges);

        {
        TIME_BANDWIDTH("Rendering", 0);
        {
        TIME_BANDWIDTH("Draw changes", 0);
        BeginTextureMode(canvas);
            DrawTexture(canvasChanges.texture, 0, 0, WHITE);
        EndTextureMode();
        }

        BeginDrawing();
        {
            {
            TIME_BANDWIDTH("Draw canvas", 0);
            ClearBackground(BLACK);
            DrawTextureEx(canvas.texture, {0, canvasWorldRec.y}, 0, gridScale, WHITE);
            }

            if (CheckCollisionPointRec(mousePos, canvasWorldRec))
            {
                brush.DrawMousePosOutline(gridScale);
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    HandleMouseButtonInput(mousePos, MOUSE_BUTTON_LEFT, brush, &canvasChanges);
                }
                else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                {
                    HandleMouseButtonInput(mousePos, MOUSE_BUTTON_RIGHT, brush, &canvasChanges);
                }

                if (float mwDiff{ GetMouseWheelMove() }; mwDiff != 0)
                {
                    brush.Resize(Clamp(brush.mSize + mwDiff * 5.0f, 1.0f, 100.0f), gridScale);
                }
            }

            {
            TIME_BANDWIDTH("Draw UI", 0);
            brush.Hotbar(hotbarWorldRec.width, hotbarWorldRec.height);
            simStepper.Hotbar(hotbarWorldRec.width, hotbarWorldRec.height);

            if (simStepper.mIsPaused)
            {
                Vector2 localPos{ simStepper.x * gridScale, simStepper.y * gridScale };
                localPos.y = Lerp(canvasHeight, 0, (f32)simStepper.y / gridSize.y) - gridScale;
                Vector2 globalPos{ canvasWorldRec.x + localPos.x, canvasWorldRec.y + localPos.y };
                DrawRectangleLines(globalPos.x, globalPos.y, gridScale, gridScale, RED);
            }
            DrawFPS(15, 15);
            }

            //static Color testingColor{ WHITE };
            //static float testingColorBrig{ 0.0f };
            //auto colNormalized{ ColorNormalize(testingColor) };
            //GuiSlider({10.0f, 10.0f, 100.0f, 30.0f}, "r", nullptr, &colNormalized.x, 0.0f, 1.0f);
            //GuiSlider({10.0f, 40.0f, 100.0f, 30.0f}, "g", nullptr, &colNormalized.y, 0.0f, 1.0f);
            //GuiSlider({10.0f, 70.0f, 100.0f, 30.0f}, "b", nullptr, &colNormalized.z, 0.0f, 1.0f);
            //GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "a", nullptr, &colNormalized.w, 0.0f, 1.0f);
            //GuiSlider({10.0f, 130.0f, 100.0f, 30.0f}, "brig", nullptr, &testingColorBrig, -1.0f, 1.0f);
            //testingColor = ColorFromNormalized(colNormalized);
            ////GuiSlider({10.0f, 100.0f, 100.0f, 30.0f}, "tint", nullptr, &colNormalized.a, 0.0f, 1.0f);
            //DrawRectangle(10.0f, 160.0f, 100.0f, 100.0f, ColorBrightness(testingColor, testingColorBrig));
        }
        EndDrawing();
        }
    }

    u64 totalCyclesPassed{ Profiller::globalProfiller.EndProfilling() };
    CloseWindow();

    Profiller::DrawProfilerResults(screenWidth, screenHeight, totalCyclesPassed);

    return 0;
}

void SimStepperProcessing(SimStepper* simStepper)
{
    simStepper->mStepNextParticles--;

    // First process even columns, than swith to uneven
    auto& Px = simStepper->x;
    auto& Py = simStepper->y;
    auto& particle{ *GetParticlePtr(Px, Py) };
    ProcessParticle(particle, Px, Py);
    particle.props ^= Particle::Props::IsProcessed;

    Py++;
    if (Py == gridSize.y)
    {
        Py = 0;
        Px += 2;
        if (Px == gridSize.x)
            Px = 1;
        else if (Px == gridSize.x + 1)
            Px = 0;
    }
}

void OneFrameProcessing()
{
    // First process even columns, than swith to uneven
    Particle* column{ particles };
    for (u16 x{0}; x < gridSize.x; x += 2)
    {
        for (u16 y{0}; y < gridSize.y; y++)
        {
            auto& particle{ column[y] };
            ProcessParticle(particle, x, y);
        }
        column += gridSize.y * 2;
        if (x == gridSize.x - 2)
        {
            column = particles + gridSize.y;
            x = -1;
        }
    }
    column = particles;
    for (u16 x{0}; x < gridSize.x; x++)
    {
        for (u16 y{0}; y < gridSize.y; y++)
        {
            column[y].props &= ~Particle::Props::IsProcessed;
        }
        column += gridSize.y;
    }
}

// Fills in a particle buffer and also draws to "buffer of changes"
void HandleMouseButtonInput(Vector2 _currMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange)
{
    _currMousePos.x -= canvasWorldRec.x;
    _currMousePos.y -= canvasWorldRec.y;
    if (_currMousePos.x <= 0 || _currMousePos.x >= canvasWidth ||
        _currMousePos.y <= 0 || _currMousePos.y >= canvasHeight)
    {
        return;
    }
    static Vector2 sPrevMousePosition{ GetMousePosition() };
    sPrevMousePosition = _currMousePos;

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
        float mouseCenterX{ Lerp(sPrevMousePosition.x, _currMousePos.x, lerpT) };
        float mouseCenterY{ Lerp(sPrevMousePosition.y, _currMousePos.y, lerpT) };

        Vector2i center{ GetMousePositionGrid({ mouseCenterX, mouseCenterY }) };

        // Clamp in case if mouse is out of window or close to the edges
        // Also considering bedrock border
        Rectangle area {
            Clamp(center.x, 1, gridSize.x - 1),
            Clamp(center.y - _brush.mSize, 1, gridSize.y - 1),
            Clamp(center.x + _brush.mSize, 1, gridSize.x - 1),
            Clamp(center.y, 1, gridSize.y - 1),
        };

        // Update particle buffer and draw to "buffer of changes"
        for (int y = area.y; y < area.height; y++)
        {
            for (int x = area.x; x < area.width; x++)
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
    else if (IsKeyPressed(KEY_FOUR))
    {
        _brush->mDrawType = Particle::Type::Emitter;
    }
    else if (IsKeyPressed(KEY_FIVE))
    {
        _brush->mDrawType = Particle::Type::Deleter;
    }
}

void ProcessParticle(Particle& particle, u16 x, u16 y)
{
    // TIME_FUNCTION;
    if (particle.props & Particle::Props::IsProcessed)
    {
        return;
    }

    particle.props |= Particle::Props::IsProcessed;

    switch (particle.type)
    {
    case Particle::Type::Sand: {
        if (y != 0)
        {
            TIME_BANDWIDTH("Sand", 0);
            auto p{ GetParticlePtr(x, y-1)};
            if (; (p->props & Particle::Props::Liquid) || !(p->props & (~1)))
            {
                SwapParticles(x, y, x, y-1);
            }
            else if (p = GetParticlePtr(x-1, y-1); (p->props & Particle::Props::Liquid) || !(p->props & (~1)))
            {
                SwapParticles(x, y, x-1, y-1);
            }
            else if (p = GetParticlePtr(x+1, y-1); (p->props & Particle::Props::Liquid) || !(p->props & (~1)))
            {
                SwapParticles(x, y, x+1, y-1);
            }
        }
    } break;
    case Particle::Type::Water: {
        if (y != 0)
        {
            TIME_BANDWIDTH("Water", 0);
            if (!(GetParticlePtr(x, y-1)->props & (~1)))
            {
                SwapParticles(x, y, x, y-1);
            }
            else if (!(GetParticlePtr(x+1, y-1)->props & (~1)))
            {
                SwapParticles(x, y, x+1, y-1);
            }
            else if (!(GetParticlePtr(x-1, y-1)->props & (~1)))
            {
                SwapParticles(x, y, x-1, y-1);
            }
            else if (!(GetParticlePtr(x-1, y)->props & (~1)) && !(GetParticlePtr(x+1, y)->props & (~1)))
            {
                static i8 dir[2]{ -1, 1 };
                SwapParticles(x, y, x+dir[GetRandomValue(0, 1)], y);
            }
            else if (!(GetParticlePtr(x-1, y)->props & (~1)))
            {
                SwapParticles(x, y, x-1, y);
            }
            else if (!(GetParticlePtr(x+1, y)->props & (~1)))
            {
                SwapParticles(x, y, x+1, y);
            }
        }
    } break;
    case Particle::Type::Emitter: {
        TIME_BANDWIDTH("Emitter", 0);
        if (!(GetParticlePtr(x, y-1)->props & ~1))
        {
            SetParticle(x, y-1, { (Particle::Type)(particle.reg & 255) });
            // SetParticle(x, y-1, { (Particle::Type::Sand) });
        }
    } break;
    case Particle::Type::Deleter: {
        TIME_BANDWIDTH("Deleter", 0);
        if (!(GetParticlePtr(x, y+1)->props & Particle::Props::NonDestruct))
        {
            SetParticle(x, y+1, {Particle::Type::Air});
        }
    } break;
    case Particle::Type::Air:
    case Particle::Type::Rock:
    case Particle::Type::Bedrock:
        break;
    default:
        assert(false); // Unknown particle type
        break;
    }
}

ANCHOR_LAST_INDEX_DEFINITION;
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"