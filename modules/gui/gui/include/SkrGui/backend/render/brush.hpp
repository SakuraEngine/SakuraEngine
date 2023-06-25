#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/backend/render/paint_params.hpp"

namespace skr::gui
{
// TODO. better brush
// Custom Brush
// Color Brush
// Surface Brush
// Surface Nine Brush
struct CustomBrush {
    FunctionRef<void(GDIVertex&)> _custom_paint = nullptr;

    inline CustomBrush() SKR_NOEXCEPT = default;
    inline CustomBrush(FunctionRef<void(GDIVertex&)> custom_paint) SKR_NOEXCEPT : _custom_paint(custom_paint) {}
};

struct ColorBrush {
    Color _color = { 1, 1, 1, 1 };

    inline ColorBrush() SKR_NOEXCEPT = default;
    inline ColorBrush(const Color& color) SKR_NOEXCEPT : _color(color) {}
};

struct TextureBrush {
    ITexture*                     _texture = nullptr;
    Color                         _color = { 1, 1, 1, 1 };
    Rect                          _uv_rect = {};
    Rect                          _uv_rect_nine_total = {};
    float                         _rotation = 0.0f;
    Swizzle                       _swizzle = {};
    FunctionRef<void(GDIVertex&)> _custom_paint = nullptr;

    inline TextureBrush() SKR_NOEXCEPT = default;
    inline TextureBrush(ITexture* texture) SKR_NOEXCEPT
        : _texture(texture),
          _color{ 1, 1, 1, 1 },
          _uv_rect{},
          _uv_rect_nine_total{},
          _rotation(0.0f),
          _swizzle{},
          _custom_paint(nullptr)
    {
    }

    TextureBrush& color(Color color) SKR_NOEXCEPT
    {
        _color = color;
        return *this;
    }
    TextureBrush& uv_rect(Rect uv_rect) SKR_NOEXCEPT
    {
        _uv_rect = uv_rect;
        _uv_rect_nine_total = {};
        return *this;
    }
    TextureBrush& uv_rect_nine(Rect center, Rect total) SKR_NOEXCEPT
    {
        _uv_rect = center;
        _uv_rect_nine_total = total;
        return *this;
    }
    TextureBrush& rotation(float rotation) SKR_NOEXCEPT
    {
        _rotation = rotation;
        return *this;
    }
    TextureBrush& swizzle(Swizzle swizzle) SKR_NOEXCEPT
    {
        _swizzle = swizzle;
        return *this;
    }
    TextureBrush& custom_paint(FunctionRef<void(GDIVertex&)> custom_paint) SKR_NOEXCEPT
    {
        _custom_paint = custom_paint;
        return *this;
    }
};

struct MaterialBrush {
    IMaterial*                    _material = nullptr;
    Color                         _color = { 1, 1, 1, 1 };
    Rect                          _uv_rect = {};
    Rect                          _uv_rect_nine_total = {};
    float                         _rotation = 0.0f;
    FunctionRef<void(GDIVertex&)> _custom_paint = nullptr;

    inline MaterialBrush() SKR_NOEXCEPT = default;
    inline MaterialBrush(IMaterial* material) SKR_NOEXCEPT
        : _material(material),
          _color{ 1, 1, 1, 1 },
          _uv_rect{},
          _uv_rect_nine_total{},
          _rotation(0.0f),
          _custom_paint(nullptr)
    {
    }

    MaterialBrush& color(Color color) SKR_NOEXCEPT
    {
        _color = color;
        return *this;
    }
    MaterialBrush& uv_rect(Rect uv_rect) SKR_NOEXCEPT
    {
        _uv_rect = uv_rect;
        _uv_rect_nine_total = {};
        return *this;
    }
    MaterialBrush& uv_rect_nine(Rect center, Rect total) SKR_NOEXCEPT
    {
        _uv_rect = center;
        _uv_rect_nine_total = total;
        return *this;
    }
    MaterialBrush& rotation(float rotation) SKR_NOEXCEPT
    {
        _rotation = rotation;
        return *this;
    }
    MaterialBrush& custom_paint(FunctionRef<void(GDIVertex&)> custom_paint) SKR_NOEXCEPT
    {
        _custom_paint = custom_paint;
        return *this;
    }
};

struct Brush {
    EPaintType paint_type;
    union
    {
        CustomBrush   custom;
        ColorBrush    color;
        TextureBrush  texture;
        MaterialBrush material;
    };

    inline Brush() SKR_NOEXCEPT
        : paint_type(EPaintType::Color),
          color()
    {
    }

    inline Brush(const ColorBrush& color_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Color),
          color(color_brush)
    {
    }
    inline Brush(const TextureBrush& texture_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Texture),
          texture(texture_brush)
    {
    }
    inline Brush(const MaterialBrush& material_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Material),
          material(material_brush)
    {
    }
    inline Brush(const CustomBrush& custom_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Custom),
          custom(custom_brush)
    {
    }
    inline Brush(ColorBrush&& color_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Color),
          color(std::move(color_brush))
    {
    }
    inline Brush(TextureBrush&& texture_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Texture),
          texture(std::move(texture_brush))
    {
    }
    inline Brush(MaterialBrush&& material_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Material),
          material(std::move(material_brush))
    {
    }
    inline Brush(CustomBrush&& custom_brush) SKR_NOEXCEPT
        : paint_type(EPaintType::Custom),
          custom(std::move(custom_brush))
    {
    }

    inline ~Brush() SKR_NOEXCEPT
    {
        switch (paint_type)
        {
            case EPaintType::Custom:
                custom.~CustomBrush();
                break;
            case EPaintType::Color:
                color.~ColorBrush();
                break;
            case EPaintType::Texture:
                texture.~TextureBrush();
                break;
            case EPaintType::Material:
                material.~MaterialBrush();
                break;
        }
    }

    inline Brush(const Brush& other) SKR_NOEXCEPT
        : paint_type(other.paint_type)
    {
        switch (paint_type)
        {
            case EPaintType::Custom:
                custom = other.custom;
                break;
            case EPaintType::Color:
                color = other.color;
                break;
            case EPaintType::Texture:
                texture = other.texture;
                break;
            case EPaintType::Material:
                material = other.material;
                break;
        }
    }
    inline Brush(Brush&& other) SKR_NOEXCEPT
        : paint_type(other.paint_type)
    {
        switch (paint_type)
        {
            case EPaintType::Custom:
                custom = std::move(other.custom);
                break;
            case EPaintType::Color:
                color = std::move(other.color);
                break;
            case EPaintType::Texture:
                texture = std::move(other.texture);
                break;
            case EPaintType::Material:
                material = std::move(other.material);
                break;
        }
    }

    inline Brush& operator=(const Brush& other) SKR_NOEXCEPT
    {
        paint_type = other.paint_type;
        switch (paint_type)
        {
            case EPaintType::Custom:
                custom = other.custom;
                break;
            case EPaintType::Color:
                color = other.color;
                break;
            case EPaintType::Texture:
                texture = other.texture;
                break;
            case EPaintType::Material:
                material = other.material;
                break;
        }
        return *this;
    }
    inline Brush& operator=(Brush&& other) SKR_NOEXCEPT
    {
        paint_type = other.paint_type;
        switch (paint_type)
        {
            case EPaintType::Custom:
                custom = std::move(other.custom);
                break;
            case EPaintType::Color:
                color = std::move(other.color);
                break;
            case EPaintType::Texture:
                texture = std::move(other.texture);
                break;
            case EPaintType::Material:
                material = std::move(other.material);
                break;
        }
        return *this;
    }
};
} // namespace skr::gui