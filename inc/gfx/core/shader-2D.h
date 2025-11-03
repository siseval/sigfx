#ifndef GFX_SHADER_2D_H
#define GFX_SHADER_2D_H

#include <gfx/core/types/color4.h>
#include <gfx/math/vec2.h>

namespace gfx::core
{

struct ShaderInput2D
{
    gfx::math::Vec2d uv;
    double t;
};

class Shader2D
{

public:

    virtual types::Color4 frag(const ShaderInput2D &input) const = 0;
    static inline types::Color4 mix(const types::Color4 &a, const types::Color4 &b, const double factor)
    {
        double clamped_factor = std::clamp(factor, 0.0, 1.0);
        return types::Color4(
            static_cast<uint8_t>(a.r * (1.0 - clamped_factor) + b.r * clamped_factor),
            static_cast<uint8_t>(a.g * (1.0 - clamped_factor) + b.g * clamped_factor),
            static_cast<uint8_t>(a.b * (1.0 - clamped_factor) + b.b * clamped_factor),
            static_cast<uint8_t>(a.a * (1.0 - clamped_factor) + b.a * clamped_factor)
        );
    }


private:



};

}

#endif // GFX_SHADER_2D_H
