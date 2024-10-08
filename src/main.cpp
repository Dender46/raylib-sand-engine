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
#include "rl_tooltip.hpp"
#define PROFILLER 0
#include "profiler/profiller.hpp"
#include "profiler/profilerRLDisplay.hpp"

constexpr i32 screenWidth{ 1600 };
constexpr i32 screenHeight{ 800 };

constexpr u32 canvasWidth{ screenWidth };
constexpr u32 canvasHeight{ screenHeight - 45 };

constexpr u32 gridScale{ 1 };
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
void HandleMouseButtonInput(Vector2i _currMousePos, Vector2i _prevMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange);
void HandleKeyboardInput(Brush* _brush);
void ProcessParticle(Particle& particle, u16 x, u16 y);

void DrawLine(int x0, int y0, int x1, int y1, Particle::Type p);
void DrawLineH(int x0, int y0, int x1, int y1, Particle::Type p);
void DrawLineV(int x0, int y0, int x1, int y1, Particle::Type p);

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
        gridSize.y - (i32)mousePos.y / (i32)gridScale - 1
    };
}

void InitFillScreen()
{
    for (int x = 0; x < gridSize.x; x++)
        for (int y = 0; y < gridSize.y; y++)
            SetParticle(x, y, { Particle::Type::Air });

    // Fill entire canvas with screen
#if 0
    for (int x = 0; x < gridSize.x; x++)
        for (int y = 0; y < gridSize.y; y++)
            SetParticle(x, y, { Particle::Type::Sand });
#endif

// Top - emitters, bottom - deleters
#if 0
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

    InitFillScreen();
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

        Vector2 mousePos{ GetMousePosition() };
        Vector2 mousePosCanvas{ mousePos.x + canvasWorldRec.x, mousePos.y - canvasWorldRec.y };
        Vector2i mousePosGrid{ GetMousePositionGrid({ mousePosCanvas.x, mousePosCanvas.y }) };
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

                static Vector2i prevMousePosGrid{ mousePosGrid };
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    HandleMouseButtonInput(mousePosGrid, prevMousePosGrid, MOUSE_BUTTON_LEFT, brush, &canvasChanges);
                }
                else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                {
                    HandleMouseButtonInput(mousePosGrid, prevMousePosGrid, MOUSE_BUTTON_RIGHT, brush, &canvasChanges);
                }
                prevMousePosGrid = mousePosGrid;

                if (float mwDiff{ GetMouseWheelMove() }; mwDiff != 0)
                {
                    brush.Resize(Clamp(brush.mSize + mwDiff * 5.0f, 1.0f, 100.0f), gridScale);
                }

                if (simStepper.mIsPaused)
                {
                    char textBuffer[256];
                    Particle::GetDescription(textBuffer, 256, mousePosGrid, GetParticlePtr(mousePosGrid.x, mousePosGrid.y));
                    RLExt::Tooltip(mousePos, textBuffer, { screenWidth, screenHeight });
                }
            }

            {
            TIME_BANDWIDTH("Draw UI", 0);
            brush.Hotbar(hotbarWorldRec.width, hotbarWorldRec.height);
            simStepper.Hotbar(hotbarWorldRec.width, hotbarWorldRec.height);

            if (simStepper.mIsPaused)
            {
                Vector2 localPos{ simStepper.x * gridScale, (gridSize.y - simStepper.y) * gridScale - gridScale};
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

void DrawLine(int x0, int y0, int x1, int y1, Particle::Type p)
{
    if (abs(x1-x0) > abs(y1-y0))
        DrawLineH(x0, y0, x1, y1, p);
    else
        DrawLineV(x0, y0, x1, y1, p);
}

void DrawLineH(int x0, int y0, int x1, int y1, Particle::Type particle)
{
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    if (dx == 0) {
        SetParticle(x0, y0, particle);
        return;
    }
    int dy = y1 - y0;
    int dir = dy < 0 ? -1 : 1;
    dy *= 2*dir;


    int p = dy - dx;
    for (int i = 0; i < dx+1; i++)
    {
        if (GetParticlePtr(x0 + i, y0)->props & (u16)Particle::Props::NonDestruct)
            continue;
        SetParticle(x0 + i, y0, {particle});
        if (p >= 0) {
            y0 += dir;
            p -= 2*dx;
        }
        p += dy;
    }
}

void DrawLineV(int x0, int y0, int x1, int y1, Particle::Type particle)
{
    if (y0 > y1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dy = y1 - y0;
    if (dy == 0) {
        SetParticle(x0, y0, {particle});
        return;
    }
    int dx = x1 - x0;
    int dir = dx < 0 ? -1 : 1;
    dx *= 2*dir;


    int p = dx - dy;
    for (int i = 0; i < dy+1; i++)
    {
        if (GetParticlePtr(x0, y0 + i)->props & (u16)Particle::Props::NonDestruct)
            continue;
        SetParticle(x0, y0 + i, particle);
        if (p >= 0) {
            x0 += dir;
            p -= 2*dy;
        }
        p += dx;
    }
}

void SimStepperProcessing(SimStepper* simStepper)
{
    simStepper->mStepNextParticles--;

    // First process even columns, than switch to uneven
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
    // First process even columns, than switch to uneven
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
void HandleMouseButtonInput(Vector2i _currMousePos, Vector2i _prevMousePos, MouseButton _mouseBttn, const Brush& _brush, RenderTexture2D* _canvasChange)
{
    if (_currMousePos.x <= 0 || _currMousePos.x >= canvasWidth ||
        _currMousePos.y <= 0 || _currMousePos.y >= canvasHeight)
    {
        return;
    }

    Particle::Type newParticleType;
    switch (_mouseBttn)
    {
        case MOUSE_BUTTON_LEFT: newParticleType = _brush.mDrawType;
            break;
        case MOUSE_BUTTON_RIGHT: newParticleType = Particle::Type::Air;
            break;
    }


    /* How area works:
        x h         x = mouse pos
        ^           y = begin of area from bottom
        |           w = width from x
        |           h = mouse pos + 1 (+1 for some reason)
        y---->w
            Clamp in case if mouse is out of window or close to the edges
            Also considering bedrock border
    */

   Rectangle prevArea {
        Clamp(_prevMousePos.x, 1, gridSize.x - 1),
        Clamp(_prevMousePos.y - _brush.mSize + 1, 1, gridSize.y - 1),
        Clamp(_prevMousePos.x + _brush.mSize, 1, gridSize.x - 1),
        Clamp(_prevMousePos.y + 1, 1, gridSize.y - 1),
    };

    Rectangle area {
        Clamp(_currMousePos.x, 1, gridSize.x - 1),
        Clamp(_currMousePos.y - _brush.mSize + 1, 1, gridSize.y - 1),
        Clamp(_currMousePos.x + _brush.mSize, 1, gridSize.x - 1),
        Clamp(_currMousePos.y + 1, 1, gridSize.y - 1),
    };

    // Update particle buffer and draw to "buffer of changes"
    int px = prevArea.x;
    int py = prevArea.y;
    int x = area.x;
    int y = area.y;
    while (x < area.width)
    {
        py = prevArea.y;
        y = area.y;
        while (y < area.height)
        {
            DrawLine(px, py, x, y, newParticleType);

            ++y;
            if (py < prevArea.height-1)
                ++py;
        }
        ++x;
        if (px < prevArea.width-1)
            ++px;
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