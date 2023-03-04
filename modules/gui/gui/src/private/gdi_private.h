#pragma once
#include "SkrGui/gdi.h"
#include <containers/vector.hpp>

namespace skr {
namespace gdi {
struct SKR_GUI_API SGDIElementPrivate : public SGDIElement
{
    virtual ~SGDIElementPrivate() SKR_NOEXCEPT = default;
    
    skr::vector<SGDIVertex> vertices;
    skr::vector<index_t> indices;
    skr::vector<SGDIElementDrawCommand> commands;
};

struct SKR_GUI_API SGDIPaintPrivate : public SGDIPaint
{
    
};

struct SKR_GUI_API SGDICanvasPrivate : public SGDICanvas
{
    virtual void add_element(SGDIElement* element, const skr_float4_t& transform) SKR_NOEXCEPT;
    virtual void remove_element(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<SGDIElement*> all_elements() SKR_NOEXCEPT;

    skr::vector<SGDIElement*> all_elements_;
};

struct SKR_GUI_API SGDICanvasGroupPrivate : public SGDICanvasGroup
{
    virtual void add_canvas(SGDICanvas* canvas) SKR_NOEXCEPT;
    virtual void remove_canvas(SGDICanvas* canvas) SKR_NOEXCEPT;
    virtual LiteSpan<SGDICanvas*> all_canvas() SKR_NOEXCEPT;
    skr::vector<SGDICanvas*> all_canvas_;
};

struct SKR_GUI_API SGDIDevicePrivate : public SGDIDevice
{

};

} }