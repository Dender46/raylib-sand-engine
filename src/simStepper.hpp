#pragma once

#include "util.hpp"
#include "raygui.h"

struct SimStepper
{
    void Hotbar(float parentW, float parentH);

    bool mIsPaused{false};
    u32 mStepNextParticles{};
    u32 mStepNextFrame{};
    u16 x{};
    u16 y{};
};

inline void SimStepper::Hotbar(float parentW, float parentH)
{
    Vector2 containerSize{};
    float containerMargin{ 10 };
    float containerElementsPadding{ 10 };

    Vector2 bttnSize{ 35, 35 };
    Rectangle nextMultParticlesBttnRec{
        .x = parentW - bttnSize.x - containerSize.x - containerMargin,
        .y = containerMargin,
        .width  = bttnSize.x,
        .height = bttnSize.y,
    };
    containerSize.x += bttnSize.x + containerElementsPadding;
    Rectangle nextParticleBttnRec{
        .x = parentW - bttnSize.x - containerSize.x - containerMargin,
        .y = containerMargin,
        .width  = bttnSize.x,
        .height = bttnSize.y,
    };
    containerSize.x += bttnSize.x + containerElementsPadding + 10;
    Rectangle nextFrameBttnRec{
        .x = parentW - bttnSize.x - containerSize.x - containerMargin,
        .y = containerMargin,
        .width  = bttnSize.x,
        .height = bttnSize.y,
    };
    containerSize.x += bttnSize.x + containerElementsPadding;
    Rectangle pauseBttnRec{
        .x = parentW - bttnSize.x - containerSize.x - containerMargin,
        .y = containerMargin,
        .width  = bttnSize.x,
        .height = bttnSize.y,
    };


    if (GuiButton(pauseBttnRec, GuiIconText(this->mIsPaused ? ICON_PLAYER_PLAY : ICON_PLAYER_PAUSE, nullptr)))
    {
        this->x = 0;
        this->y = 0;
        this->mIsPaused = !this->mIsPaused;
    }
    if (this->mIsPaused)
    {
        if (GuiButton(nextFrameBttnRec, GuiIconText(ICON_BOX_GRID, nullptr)))
        {
            this->mStepNextFrame = 1;
        }
        if (GuiButton(nextParticleBttnRec, GuiIconText(ICON_PLAYER_NEXT, nullptr)))
        {
            this->mStepNextParticles = 1;
        }
        if (GuiButton(nextMultParticlesBttnRec, GuiIconText(ICON_PLAYER_JUMP, nullptr)))
        {
            this->mStepNextParticles = 10;
        }
    }
}
