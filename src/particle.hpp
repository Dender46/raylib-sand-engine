#pragma once

#include <cassert>
#include <array>

#include "raylib.h"
#include "util.hpp"

#define TYPES_TABLE \
    TYPE_DEF(Air), \
    TYPE_DEF(Sand), \
    TYPE_DEF(Water), \
    TYPE_DEF(Rock), \
    TYPE_DEF(Emitter), \
    TYPE_DEF(Deleter), \
    TYPE_DEF(Bedrock),

#define PROPS_TABLE \
    PROP_DEF(IsProcessed,   0) \
    PROP_DEF(Static,        1) \
    PROP_DEF(Solid,         2) \
    PROP_DEF(Liquid,        3) \
    PROP_DEF(UserDestruct,  4) \
    PROP_DEF(NonDestruct,   5) \

struct Particle
{
    #define TYPE_DEF(type) type
    enum class Type : u8 {
        TYPES_TABLE

        COUNT
    };
    #undef TYPE_DEF

    #define PROP_DEF(prop, id) prop = 1 << id,
    enum Props : u16 {
        None = 0,
        PROPS_TABLE
    };
    #undef PROP_DEF

    Particle() = default;
    Particle(Type _type, u32 _reg = 0);

    Color color{ PURPLE };
    u32 reg{ 0 }; // holds type-dependant info
    u16 props{ 0 };
    // i8 velX{ 0 };
    // i8 velY{ 0 };
    Type type{ Type::Air };

    static void GetDescription(char* buffer, u16 bufferLen, Vector2i pPos, const Particle* const particle);
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
const ParticlePropertiesMap particlePropertiesMap;
#define TYPE_DEF(type) #type
static const char* const ParticleTypeStr[] {
    TYPES_TABLE
};
#undef TYPE_DEF
#undef TYPES_TABLE

#define PROP_DEF(prop, id) #prop,
static const char* const ParticlePropStr[] {
    PROPS_TABLE
};
#undef PROP_DEF
#undef PROPS_TABLE

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
#include <iostream>
inline void Particle::GetDescription(char* buffer, u16 bufferLen, Vector2i pPos, const Particle* const particle)
{
    // Pos: 1000 x 1000\nType: Emitter\nProps: None\nIsProcessed\nStatic\nSolid\nLiquid\nUserDestruct\nNonDestruct
    // const char* str{ "Pos: %u x %u\nType: %s\nProps: %s\n" };
    const char* str{ "Pos: %u x %u\nType: %s\nProps: %s" };
    char propsStr[128]{0};

    u16 index{ 0 }, bitOffset{ 1 };
    int propsStrPos{ 0 };
    while (index < ARRAY_COUNT(ParticlePropStr))
    {
        if (particle->props & bitOffset)
        {
            TextAppend(propsStr, ParticlePropStr[index], &propsStrPos);
            TextAppend(propsStr, "; ", &propsStrPos);
        }

        bitOffset <<= 1;
        index++;
    }

    std::sprintf(buffer, str, pPos.x, pPos.y, ParticleTypeStr[(u8)particle->type], propsStr);
}
