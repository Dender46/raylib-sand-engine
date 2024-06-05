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
        Water,
        Rock,
        Bedrock,
        COUNT
    };
    enum Props : u16 {
        None        = 0,
        Static      = 1 << 0,
        Solid       = 1 << 1,
        Liquid      = 1 << 2,
        NonDestruct = 1 << 3,
    };

    Particle() = default;
    Particle(Type _type);

    Color color{ PURPLE };
    u16 props{ 0 };
    i8 velX{ 0 };
    i8 velY{ 0 };
    Type type{ Type::Air };
};

namespace ParticlePropertiesMap
{
    using enum Particle::Props;
    constexpr u16 map[] = {
        /*Air*/         0,
        /*Sand*/        Solid,
        /*Sand*/        Liquid,
        /*Rock*/        Static | Solid,
        /*Bedrock*/     Static | Solid | NonDestruct,
    };

    static_assert(
        sizeof(map) / sizeof(Particle::Props)
        == // Particle types and props are mismatched
        (u32)Particle::Type::COUNT
    );
}

inline Particle::Particle(Particle::Type _type)
    : props{ParticlePropertiesMap::map[(u64)_type]}, type{_type}
{
    switch (type)
    {
    case Type::Air:
        color = PURPLE;
        break;
    case Type::Sand:
        color = ColorBrightness(GOLD, GetRandomValue(-10, 0) * 0.01);
        break;
    case Type::Water:
        color = ColorBrightness(BLUE, GetRandomValue(-10, 0) * 0.01);;
        break;
    case Type::Rock:
        color = GRAY;
        break;
    case Type::Bedrock:
        color = BLACK;
        break;
    default:
        assert(false);
        break;
    }
}