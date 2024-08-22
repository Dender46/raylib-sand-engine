#include <cstdio>

#include "raylib.h"
#include "profiller.hpp"

namespace Profiller{
inline void DrawProfilerResults(int screenWidth, int screenHeight, u64 _totalCyclesPassed)
{
    #if 0
    InitWindow(screenWidth, screenHeight, "Profiler results");
    SetTargetFPS(60);

    const auto anchorTimings{ GetAnchorsTimings(_totalCyclesPassed) };
    Font font{ GetFontDefault() };
    u32 fontSize{ 16 };
    u32 columnsCount{ 8 };
    u32 columnsWidth[columnsCount];
    u32 columndsPosX[columnsCount];
    u32 rowsHeight{ 20 };

    const auto findMaxWidth = [&](u32 colIndex, auto number) {
        std::sprintf(strBuffer, "%d", number);
        f32 strWidth{ MeasureTextEx(font, strBuffer, fontSize, 2).x };
        if (columnsWidth[colIndex] < strWidth)
        {
            columnsWidth[colIndex] = strWidth;
        }
    };

    for (const auto& a : anchorTimings)
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

        for (const auto& a : anchorTimings)
        {
            u32 rowY{ 0 };
            u32 colX{ 0 };
            u32 colIndex{ 0 };
            DrawText(a.mAnchor.label, rowY, 0, 12, BLACK);
            DrawText(a.mAnchor.hitCount, rowY, 0, 12, BLACK);
            DrawText(a.mElapsedTSC, rowY, 0, 12, BLACK);
            DrawText(a.mElapsedTimeInS, rowY, 0, 12, BLACK);
            DrawText(a.mPercentage, rowY, 0, 12, BLACK);
            DrawText(a.mPercentageWithChildren, rowY, 0, 12, BLACK);
            DrawText(a.mProcessedMegabytes, rowY, 0, 12, BLACK);
            DrawText(a.mGigabytesPerSecond, rowY, 0, 12, BLACK);
            rowY += rowsHeight;
        }
        EndDrawing();
    }

    CloseWindow();
    #endif
}

}
