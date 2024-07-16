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
        Emitter,
        Deleter,
        Bedrock,

        COUNT
    };
    enum Props : u16 {
        None            = 0,
        Static          = 1 << 0,
        Solid           = 1 << 1,
        Liquid          = 1 << 2,
        UserDestruct    = 1 << 3,
        NonDestruct     = 1 << 4,
    };

    Particle() = default;
    Particle(Type _type, u32 _reg = 0);

    Color color{ PURPLE };
    u32 reg{ 0 }; // holds type-dependant info
    u16 props{ 0 };
    // i8 velX{ 0 };
    // i8 velY{ 0 };
    Type type{ Type::Air };
};

struct ParticlePropertiesMap
{
    ParticlePropertiesMap()
    {
        using enum Particle::Props;
        using enum Particle::Type;
        for (u32 i = 0; i < (u32)Particle::Type::COUNT; i++)
        {
            vals[i] = u16_MAX;
        }
        vals[(u8)Air]       = 0;
        vals[(u8)Sand]      = Solid;
        vals[(u8)Water]     = Liquid;
        vals[(u8)Rock]      = Static | Solid;
        vals[(u8)Emitter]   = Static | Solid | UserDestruct;
        vals[(u8)Deleter]   = Static | Solid | UserDestruct;
        vals[(u8)Bedrock]   = Static | Solid | NonDestruct;
        for (u32 i = 0; i < (u32)Particle::Type::COUNT; i++)
        {
            if (vals[i] == u16_MAX)
            {
                assert(false); // one of particle type has no properties assigned
            }
        }
    };
    u16 vals[(u32)Particle::Type::COUNT];
};
ParticlePropertiesMap particlePropertiesMap;

inline Particle::Particle(Particle::Type _type, u32 _reg)
    : props{particlePropertiesMap.vals[(u32)_type]},
      type{_type},
      reg{_reg}
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
    case Type::Emitter:
        color = GREEN;
        break;
    case Type::Deleter:
        color = RED;
        break;
    case Type::Bedrock:
        color = BLACK;
        break;
    default:
        assert(false);
        break;
    }
}