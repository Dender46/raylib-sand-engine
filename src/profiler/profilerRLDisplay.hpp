#include <cstdio>

#include "raylib.h"
#include "raygui.h"
#include "profiller.hpp"

namespace Profiller{

inline void DrawProfilerResults(int screenWidth, int screenHeight, u64 _totalCyclesPassed)
{
    InitWindow(screenWidth, screenHeight, "Profiler results");
    SetTargetFPS(60);

    const auto profilerReports{ GetAnchorsTimings(_totalCyclesPassed) };

    struct TableColumn
    {
        const char* mName{ nullptr };
        f32 mWidth{ 0 };

        void findMaxWidth(const char* str, Font font, u32 fontSize)
        {
            f32 strWidth{ MeasureTextEx(font, str, fontSize, fontSize / 10).x };
            if (mWidth < strWidth)
                mWidth = strWidth;
        };
        void findMaxWidth(f64 number, Font font, u32 fontSize)
        {
            char strBuffer[64];
            std::sprintf(strBuffer, "%f", number);
            f32 strWidth{ MeasureTextEx(font, strBuffer, fontSize, fontSize / 10).x };
            if (mWidth < strWidth)
                mWidth = strWidth;
        };
        void findMaxWidth(u64 number, Font font, u32 fontSize)
        {
            char strBuffer[64];
            std::sprintf(strBuffer, "%llu", number);
            f32 strWidth{ MeasureTextEx(font, strBuffer, fontSize, fontSize / 10).x };
            if (mWidth < strWidth)
                mWidth = strWidth;
        };
    };
    TableColumn tableColumns[] = {
        {"anchorName", 0},
        {"elapsedTSC", 0},
        {"elapsedTimeInS", 0},
        {"%"},
        {"% w/ children", 0},
        {"processedMB", 0},
        {"GB/S", 0},
    };
    Font font{ GetFontDefault() };
    u32 fontSize{ 28 };
    u32 widthPadding{ 20 };

    constexpr u8 tableCountCol{ sizeof(tableColumns) / sizeof(tableColumns[0]) };
    u64 tableCountRow{ profilerReports.size() + 1 };

    for (u32 i = 0; i < tableCountRow - 1; i++)
    {
        i32 colIndex{ -1 };
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mAnchor.label, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mElapsedTSC, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mElapsedTimeInS, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mPercentage, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mPercentageWithChildren, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mProcessedMegabytes, font, fontSize);
        tableColumns[++colIndex].findMaxWidth(profilerReports[i].mGigabytesPerSecond, font, fontSize);
    }

    while (!WindowShouldClose())
    {
        ClearBackground(RAYWHITE);
        BeginDrawing();

        u32 widthSum{ 0 };
        char strBuffer[64];
        for (u32 i = 0; i < tableCountCol; i++)
        {
            DrawText(tableColumns[i].mName, widthSum, 0, fontSize, BLACK);
            widthSum += tableColumns[i].mWidth + widthPadding;
        }
        for (u32 i = 0; i < tableCountRow-1; i++)
        {
            widthSum = 0;
            u32 height{ (i+1) * (fontSize*1.5) };
            i32 colIndex{ -1 };
            DrawText(profilerReports[i].mAnchor.label, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%llu", profilerReports[i].mElapsedTSC);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%f", profilerReports[i].mElapsedTimeInS);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%f", profilerReports[i].mPercentage);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%f", profilerReports[i].mPercentageWithChildren);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%f", profilerReports[i].mProcessedMegabytes);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;

            std::sprintf(strBuffer, "%f", profilerReports[i].mGigabytesPerSecond);
            DrawText(strBuffer, widthSum, height, fontSize, BLACK);
            widthSum += tableColumns[++colIndex].mWidth + widthPadding;
        }
        EndDrawing();
    }

    CloseWindow();
}

}
