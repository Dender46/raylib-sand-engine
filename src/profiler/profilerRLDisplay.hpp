#include <cstdio>

#include "raylib.h"
#include "profiller.hpp"

namespace Profiller{

Font font{ GetFontDefault() };
u32 fontSize{ 16 };
constexpr u32 columnsCount{ 8 };
u32 columnsWidth[columnsCount];
void findMaxWidth(u32 colIndex, auto number) {
    char strBuffer[64];
    std::sprintf(strBuffer, "%d", number);
    f32 strWidth{ MeasureTextEx(font, strBuffer, fontSize, 2).x };
    if (columnsWidth[colIndex] < strWidth)
    {
        columnsWidth[colIndex] = strWidth;
    }
};

inline void DrawProfilerResults(int screenWidth, int screenHeight, u64 _totalCyclesPassed)
{
    InitWindow(screenWidth, screenHeight, "Profiler results");
    SetTargetFPS(60);

    const auto profilerReport{ GetAnchorsTimings(_totalCyclesPassed) };
    u32 columndsPosX[columnsCount];
    u32 rowsHeight{ 20 };

    

    for (const auto& a : profilerReport)
    {
        u32 colIndex{ 0 };
        findMaxWidth(colIndex  , a.mAnchor.label);
        findMaxWidth(++colIndex, a.mAnchor.hitCount);
        findMaxWidth(++colIndex, a.mElapsedTSC);
        findMaxWidth(++colIndex, a.mElapsedTimeInS);
        findMaxWidth(++colIndex, a.mPercentage);
        findMaxWidth(++colIndex, a.mPercentageWithChildren);
        findMaxWidth(++colIndex, a.mProcessedMegabytes);
        findMaxWidth(++colIndex, a.mGigabytesPerSecond);
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Hello", 10, 10, 20, BLACK);
#if 0
        for (const auto& a : profilerReport)
        {
            u32 rowY{ 0 };
            u32 colX{ 0 };
            u32 colIndex{ 0 };
            DrawText(a.mAnchor.label, rowY, 0, 12, BLACK);
            DrawText(a.mAnchor.hitCount, rowY, 0, 12, BLACK);
            DrawText(a.mElapsedTSC_str, rowY, 0, 12, BLACK);
            DrawText(a.mElapsedTimeInS_str, rowY, 0, 12, BLACK);
            DrawText(a.mPercentage_str, rowY, 0, 12, BLACK);
            DrawText(a.mPercentageWithChildren_str, rowY, 0, 12, BLACK);
            DrawText(a.mProcessedMegabytes_str, rowY, 0, 12, BLACK);
            DrawText(a.mGigabytesPerSecond_str, rowY, 0, 12, BLACK);
            rowY += rowsHeight;
        }
#endif
        EndDrawing();
    }

    CloseWindow();
}

}
