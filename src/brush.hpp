#pragma once
#include <cstdint>

#include "raylib.h"
#include "particle.hpp"

struct Brush
{
    Brush(uint16_t _size, int _gridScale);
    void Hotbar(float width, float height);
    void DrawMousePosOutline(int _gridScale);

    void Resize(uint16_t _newSize, int _gridScale);

    uint16_t mSize;
    Rectangle mArea;
    Image mImage;
    Particle::Type mDrawType{ Particle::Type::Sand };
};



inline Brush::Brush(uint16_t _size, int _gridScale)
    : mSize{ _size },
      mArea{ 0.0f, 0.0f, _size * (float)_gridScale, _size * (float)_gridScale },
      mImage{ GenImageColor(_size, _size, BLANK) }
{}

inline void Brush::Hotbar(float width, float height)
{
    const auto drawParticleIcon =
        [selectedParticle = &mDrawType]
        (int posX, Particle::Type pType, Color pCol)
    {
        constexpr int posY{ 10 };
        constexpr int size{ 30 };

        Vector2 mousePos{ GetMousePosition() };
        Rectangle iconRec{ (float)posX, posY, size, size };
        // Icon is selected with a mouse
        if (CheckCollisionPointRec(mousePos, iconRec))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                pCol = ColorBrightness(pCol, -0.35f);
            }
            else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                *selectedParticle = pType;
            }
            else
            {
                pCol = ColorBrightness(pCol, 0.35f);
            }
            DrawRectangle(posX-3, 10-3, size+6, size+6, RAYWHITE);
        }
        // Color of particle
        DrawRectangle(posX, 10, size, size, pCol);
        // Particle is selected
        if (*selectedParticle == pType)
        {
            DrawLineEx(
                {posX + size * 0.2f, posY + size * 0.5f},
                {posX + size * 0.55f, posY + size * 0.75f}, 5.0f, BLACK);
            DrawLineEx(
                {posX + size * 0.5f, posY + size * 0.7f}, 
                {posX + size * 0.8f, posY + size * 0.2f}, 5.0f, BLACK);
        }
    };
    constexpr int padding{ 10 };
    DrawRectangle(0, 0, width, height, BLACK);
    drawParticleIcon(100,               Particle::Type::Sand,       GOLD);
    drawParticleIcon(130 + padding * 1, Particle::Type::Water,      BLUE);
    drawParticleIcon(160 + padding * 2, Particle::Type::Rock,       GRAY);
    drawParticleIcon(190 + padding * 3, Particle::Type::Emitter,    GREEN);
    drawParticleIcon(220 + padding * 4, Particle::Type::Deleter,    RED);
}

inline void Brush::DrawMousePosOutline(int _gridScale)
{
    Vector2 mousePos{ GetMousePosition() };
    mArea.x = mousePos.x - mSize * _gridScale / 2;
    mArea.y = mousePos.y - mSize * _gridScale / 2;
    DrawRectangleLinesEx(mArea, _gridScale, RED);
}

inline void Brush::Resize(uint16_t _newSize, int _gridScale)
{
    mSize = _newSize;
    mArea = { 0.0f, 0.0f, _newSize * (float)_gridScale, _newSize * (float)_gridScale };
    UnloadImage(mImage);
    mImage = GenImageColor(_newSize, _newSize, BLANK);
}
