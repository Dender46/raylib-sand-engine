#pragma once

#include <cassert>
#include <array>

#include "raylib.h"
#include "types.hpp"

struct Particle
{
    enum class Type : u8 {
        Air,
        Sand,
        Rock,
        Bedrock,
        COUNT
    };
    enum class Props : u16 {
        None        = 0,
        Static      = 1 << 0,
        Sandy       = 1 << 1,
        Liquid      = 1 << 2,
        NonDestruct = 1 << 3,
    };

    Particle() = default;
    Particle(Type _type);

    Color color{ PURPLE };
    u16 props{ 0 };
    Type type{ Type::Air };
};

constexpr u16 particlePropertiesMap[] = {
    /*Air*/         0,
    /*Sand*/        (0 | (u16)Particle::Props::Sandy),
    /*Rock*/        (0 | (u16)Particle::Props::Static),
    /*Bedrock*/     (0 | (u16)Particle::Props::Static | (u16)Particle::Props::NonDestruct),
};

static_assert(
    sizeof(particlePropertiesMap) / sizeof(Particle::Props)
    == // Particle types and props are mismatched
    (u32)Particle::Type::COUNT
);

Particle::Particle(Particle::Type _type)
    : props{particlePropertiesMap[(u64)_type]}, type{_type}
{
    switch (type)
    {
    case Particle::Type::Air:
        color = PURPLE;
        break;
    case Particle::Type::Sand:
        color = ColorBrightness(GOLD, GetRandomValue(-10, 0) * 0.01);
        break;
    case Particle::Type::Rock:
        color = GRAY;
        break;
    case Particle::Type::Bedrock:
        color = BLACK;
        break;
    default:
        assert(false);
        break;
    }
}