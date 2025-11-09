#include <gfx/primitives/text-2D.h>
#include <gfx/utils/transform.h>

namespace gfx::primitives
{

using namespace gfx::math;
using namespace gfx::core::types;
using namespace gfx::text;



Box2d Text2D::get_geometry_size() const
{
    Box2d bounds { Vec2d::zero(), Vec2d::zero() };
    double scale = font_size / font->get_units_per_em();

    double ascent { font->get_ascent() };
    double baseline_offset { ascent * scale };

    double pen_x = 0.0;

    for (size_t i = 0; i < text.length(); ++i)
    {
        uint32_t codepoint = static_cast<uint32_t>(text[i]);

        if (i > 0)
        {
            uint32_t prev_codepoint = static_cast<uint32_t>(text[i - 1]);
            pen_x += font->get_kerning(prev_codepoint, codepoint) * scale;
        }

        std::vector<ContourEdge> edges = font->get_glyph_edges(codepoint);

        for (auto edge : edges)
        {
            edge.v0 = edge.v0 * scale;
            edge.v1 = edge.v1 * scale;

            edge.v0.x += pen_x;
            edge.v1.x += pen_x;

            edge.v0.y = edge.v0.y + baseline_offset;
            edge.v1.y = edge.v1.y + baseline_offset;

            bounds.expand(edge.v0);
            bounds.expand(edge.v1);
        }

        pen_x += font->get_glyph_advance(codepoint) * scale;
    }

    return Box2d { Vec2d::zero(), bounds.size() };
}




void Text2D::rasterize_glyph(std::vector<ContourEdge> glyph, const std::function<void(const Pixel&)> emit_pixel) const
{
    if (glyph.empty())
    {
        return;
    }

    Box2i bounds {
        glyph[0].v0.round(),
        glyph[0].v0.round()
    };

    for (auto &edge : glyph)
    {
        bounds.expand(edge.v0);
        bounds.expand(edge.v1);
    }

    for (int y = bounds.min.y; y <= bounds.max.y; ++y)
    {
        for (int x = bounds.min.x; x <= bounds.max.x; ++x)
        {
            int crossings = 0;

            for (auto &edge : glyph)
            {
                Vec2d v0 { edge.v0 };
                Vec2d v1 { edge.v1 };
                if (v0.y == v1.y)
                {
                    continue;
                }

                bool intersects_y { ((v0.y > y) != (v1.y > y)) };
                if (!intersects_y)
                {
                    continue;
                }

                double intersects_x { v0.x + (v1.x - v0.x) * (y - v0.y) / (v1.y - v0.y) };
                if (intersects_x > x)
                {
                    crossings++;
                }
            }

            if (crossings % 2 == 1)
            {
                emit_pixel({ { x, y }, color });
            }
        }
    }
}

void Text2D::rasterize(const Matrix3x3d &transform, const std::function<void(const Pixel&)> emit_pixel) const
{
    double scale = font_size / font->get_units_per_em();

    double ascent { font->get_ascent() };
    double descent { font->get_descent() };

    double total_height { (ascent - descent) * scale };
    double baseline_offset { ascent * scale - total_height * 0.5 };

    double pen_x = 0.0;

    for (size_t i = 0; i < text.length(); ++i)
    {
        uint32_t codepoint = static_cast<uint32_t>(text[i]);

        if (i > 0)
        {
            uint32_t prev_codepoint = static_cast<uint32_t>(text[i - 1]);
            pen_x += font->get_kerning(prev_codepoint, codepoint) * scale;
        }

        std::vector<ContourEdge> edges = font->get_glyph_edges(codepoint);

        for (auto& edge : edges)
        {
            edge.v0 = edge.v0 * scale;
            edge.v1 = edge.v1 * scale;

            edge.v0.x += pen_x;
            edge.v1.x += pen_x;

            edge.v0.y = -edge.v0.y + baseline_offset;
            edge.v1.y = -edge.v1.y + baseline_offset;

            edge.v0 = utils::transform_point(edge.v0, transform);
            edge.v1 = utils::transform_point(edge.v1, transform);
        }

        rasterize_glyph(edges, emit_pixel);

        pen_x += font->get_glyph_advance(codepoint) * scale;
    }
}


}
