#pragma once

#include <cstdint>
#include <limits>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

constexpr u8  u8_MAX { std::numeric_limits<uint8_t>::max()  };
constexpr u16 u16_MAX{ std::numeric_limits<uint16_t>::max() };
constexpr u32 u32_MAX{ std::numeric_limits<uint32_t>::max() };
constexpr u64 u64_MAX{ std::numeric_limits<uint64_t>::max() };

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

struct Vector2i
{
    i32 x;
    i32 y;
};
