#include "gdi_nanovg.hpp"
#include "platform/memory.h"
#include "utils/make_zeroed.hpp"
#include "rtm/rtmx.h"

namespace skr {
namespace gdi {

uint32_t ToColor32ABGR(NVGcolor color)
{
    color.r = std::clamp(color.r, 0.f, 1.f);   
    color.g = std::clamp(color.g, 0.f, 1.f);   
    color.b = std::clamp(color.b, 0.f, 1.f);   
    color.a = std::clamp(color.a, 0.f, 1.f);   
    uint8_t r = static_cast<uint8_t>(color.r * 255.f);
    uint8_t g = static_cast<uint8_t>(color.g * 255.f);
    uint8_t b = static_cast<uint8_t>(color.b * 255.f);
    uint8_t a = static_cast<uint8_t>(color.a * 255.f);
    return ((uint32_t)a << 24) + ((uint32_t)b << 16) + ((uint32_t)g << 8) + (uint32_t)r;
}

static skr_float2_t nvg__remapUV(skr_float2_t is, skr_float2_t size, const NVGbox& box)
{
    skr_float2_t result;
    if(box.extend[0] == 0.f || box.extend[1] == 0.f)
    {
        return { is.x / size.x, is.y / size.y };
    }

    float marginLeft;
    float marginRight;
    float marginTop;
    float marginBottom;
    {
        if(is.y < box.margin.bottom)
            marginLeft = box.margin.left + box.radius[3];
        else if(is.y < box.margin.bottom + box.radius[3])
        {
            auto off = box.margin.bottom + box.radius[3] - is.y;
            off = off*off + box.radius[3]*box.radius[3];
            marginLeft = box.margin.left + box.radius[3] - std::sqrt(off);
        }
        else if(is.y > (size.y - box.margin.top))
            marginLeft = box.margin.left + box.radius[0];
        else if(is.y > (size.y - box.margin.top) - box.radius[0])
        {
            auto off = is.y - (size.y - box.margin.top) + box.radius[0];
            off = off*off + box.radius[0]*box.radius[0];
            marginLeft = box.margin.left + box.radius[0] - std::sqrt(off);
        }
        else 
            marginLeft = box.margin.left;
        if(is.y < box.margin.bottom)
            marginRight = box.margin.right + box.radius[2];
        else if(is.y < box.margin.bottom + box.radius[2])
        {
            auto off = box.margin.bottom + box.radius[2] - is.y;
            off = off*off + box.radius[2]*box.radius[2];
            marginRight = box.margin.right + box.radius[2] - std::sqrt(off);
        }
        else if(is.y > (size.y - box.margin.top))
            marginRight = box.margin.right + box.radius[1];
        else if(is.y > (size.y - box.margin.top) - box.radius[1])
        {
            auto off = is.y - (size.y - box.margin.top) + box.radius[1];
            off = off*off + box.radius[1]*box.radius[1];
            marginRight = box.margin.right + box.radius[1] - std::sqrt(off);
        }
        else 
            marginRight = box.margin.right;
        if(is.x < box.margin.left)
            marginTop = box.margin.top + box.radius[0];
        else if(is.x < box.margin.left + box.radius[0])
        {
            auto off = box.margin.left + box.radius[0] - is.x;
            off = off*off + box.radius[0]*box.radius[0];
            marginTop = box.margin.top + box.radius[0] - std::sqrt(off);
        }
        else if(is.x > (size.x - box.margin.right))
            marginTop = box.margin.right + box.radius[1];
        else if(is.x > (size.x - box.margin.right) - box.radius[1])
        {
            auto off = is.x - (size.x - box.margin.right) + box.radius[1];
            off = off*off + box.radius[1]*box.radius[1];
            marginTop = box.margin.top + box.radius[1] - std::sqrt(off);
        }
        else 
            marginTop = box.margin.top;
        if(is.x < box.margin.left)
            marginBottom = box.margin.bottom + box.radius[3];
        else if(is.x < box.margin.left + box.radius[3])
        {
            auto off = box.margin.left + box.radius[3] - is.x;
            off = off*off + box.radius[3]*box.radius[3];
            marginBottom = box.margin.bottom + box.radius[3] - std::sqrt(off);
        }
        else if(is.x > (size.x - box.margin.right))
            marginBottom = box.margin.right + box.radius[2];
        else if(is.x > (size.x - box.margin.right) - box.radius[2])
        {
            auto off = is.x - (size.x - box.margin.right) + box.radius[2];
            off = off*off + box.radius[2]*box.radius[2];
            marginBottom = box.margin.bottom + box.radius[2] - std::sqrt(off);
        }
        else 
            marginBottom = box.margin.bottom;
    }

    if(is.x < marginLeft)
    {
        result.x = is.x / box.extend[0];
    }
    else if((size.x - is.x) < marginRight)
    {
        result.x = 1 - (size.x - is.x) / box.extend[0];
    }
    else 
    {
        auto alpha = (is.x - marginLeft) / (size.x - marginLeft - marginRight);
        result.x = (marginLeft / box.extend[0]) * alpha + (1 - marginRight / box.extend[0]) * (1-alpha);
    }

    if(is.y < marginBottom)
    {
        result.y = is.y / box.extend[1];
    }
    else if((size.y - is.y) < marginTop)
    {
        result.y = 1 - (size.y - is.y) / box.extend[1];
    }
    else 
    {
        auto alpha = (is.y - marginBottom) / (size.y - marginBottom - marginTop);
        result.y = (marginBottom / box.extend[1]) * alpha + (1 - marginTop / box.extend[1]) * (1-alpha);
    }
    return result;
}

static void nvg__xformIdentity(float* t)
{
	t[0] = 1.f; t[1] = 0.f;
	t[2] = 0.f; t[3] = 1.f;
	t[4] = 0.f; t[5] = 0.f;
}

static void nvg__xformInverse(float* inv, float* t)
{
	double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
	if (det > -1e-6 && det < 1e-6) {
		nvg__xformIdentity(t);
		return;
	}
	invdet = 1.0 / det;
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static skr_float4x4_t nvg__getMatrix(NVGpaint* paint)
{
    float invxform[6];
    nvg__xformInverse(invxform, paint->xform);
    return 
    {{
        { invxform[0], invxform[1], 0.f, 0.f },
        { invxform[2], invxform[3], 0.f, 0.f },
        { 0.f, 0.f, 1.f, 0.f },
        { invxform[4], invxform[5], 0.f, 1.f }
    }};
}

static void nvg__renderPath(GDIElementNVG* element, const NVGpath& path, NVGpaint* paint, const skr_float4x4_t& transform, float fringe)
{
    skr_float2_t extend{paint->extent[0], paint->extent[1]};
    auto& vertices = element->vertices;
    auto& indices = element->indices;
    auto push_vertex = [&](const NVGvertex& nv, uint32_t i, uint32_t nfill)
    {
        auto gdi_paint = element->gdi_paint;
        GDIVertex v;
        v.position = { nv.x, nv.y, 0.f, 1.f };
        v.aa = {nv.u, fringe};
        const rtm::vector4f pos = rtm::vector_load((const uint8_t*)&v.position);
        const auto col0 = rtm::vector_set(transform.M[0][0], transform.M[0][1], transform.M[0][2], transform.M[0][3]);
        const auto col1 = rtm::vector_set(transform.M[1][0], transform.M[1][1], transform.M[1][2], transform.M[1][3]);
        const auto col2 = rtm::vector_set(transform.M[2][0], transform.M[2][1], transform.M[2][2], transform.M[2][3]);
        const auto col3 = rtm::vector_set(transform.M[3][0], transform.M[3][1], transform.M[3][2], transform.M[3][3]);
        const auto trans = rtm::matrix_set(col0, col1, col2, col3);
        v.color = ToColor32ABGR(paint->innerColor);

        const bool override_image_space = gdi_paint && (GDIPaintNVG::CoordinateMethod::ImageSpace & gdi_paint->coordinate_method_override);
        const bool override_nvg =  gdi_paint && (GDIPaintNVG::CoordinateMethod::NVG & gdi_paint->coordinate_method_override);
        const bool default_image_space = path.nfill;
        if (!override_nvg && (override_image_space || default_image_space))
        {
            auto imgSpace = rtm::matrix_mul_vector(pos, trans);
            const float imgSpaceX = rtm::vector_get_x(imgSpace);
            const float imgSpaceY = rtm::vector_get_y(imgSpace);
            v.texcoord = nvg__remapUV({ imgSpaceX, imgSpaceY }, extend, paint->box);
        }
        else
        {
            v.texcoord = { (float)i / (float)nfill , nv.u};
        }
        
        if (gdi_paint && gdi_paint->custom_painter)
        {
            gdi_paint->custom_painter(&v, gdi_paint->custom_painter_data);
        }
        vertices.push_back(v);
    };
    //auto& path = paths[i];
    if(path.nfill)
    {
        vertices.reserve(vertices.size() + path.nfill);
        indices.reserve(indices.size() + path.nfill * 3);
        const auto start = static_cast<index_t>(vertices.size());
        for(int j=0; j < path.nfill; ++j)
        {
            push_vertex(path.fill[j], j, path.nfill);
            if(j<path.nfill-2)
            {
                const auto id = static_cast<index_t>(vertices.size());
                indices.push_back(start);
                indices.push_back(id + 1);
                indices.push_back(id);
            }
        }
    }
    if(path.nstroke)
    {
        vertices.reserve(vertices.size() + path.nstroke);
        indices.reserve(indices.size() + path.nstroke * 3);
        for(int j=0; j<path.nstroke; ++j)
        {
            push_vertex(path.stroke[j], j, path.nstroke);
            if(j<path.nstroke-2)
            {
                const auto id = static_cast<index_t>(vertices.size() - 1);
                indices.push_back(id);
                indices.push_back(id + 1 + (j % 2));
                indices.push_back(id + 1 + !(j % 2));
            }
        }
    }
}

static void nvg__renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, float fringe,
							  const float* bounds, const NVGpath* paths, int npaths)
{
    auto element = (GDIElementNVG*)uptr;
    /*
    PrimDrawResource resource;
    resource.compositeOperation = compositeOperation;
    resource.noGamma = paint->noGamma;
    */
    auto invTransform = nvg__getMatrix(paint);
    //fast path
    if(npaths == 1 && paths[0].convex) 
    {
        auto& command = element->commands.emplace_back();
        auto begin = element->indices.size();
        for(int i=0; i<npaths; ++i)
        {
            nvg__renderPath(element, paths[i], paint, invTransform, 1.f);
        }
        command.index_count = static_cast<uint32_t>(element->indices.size() - begin);
        command.first_index = static_cast<uint32_t>(begin);
        command.material = static_cast<GDIMaterialId>(paint->material);
        command.texture = static_cast<GDITextureId>(paint->image);
        if (command.texture && ((GDIResource*)command.texture)->get_state() != EGDIResourceState::Okay)
        {
            command.texture = nullptr;
        }
    }
    //slow path
    else 
    {
        SKR_ASSERT(0);
    }
}

static void nvg__renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, float fringe,
								float strokeWidth, const NVGpath* paths, int npaths)
{
    auto element = (GDIElementNVG*)uptr;
    /*
    PrimDrawResource resource;
    resource.compositeOperation = compositeOperation;
    resource.noGamma = paint->noGamma;
    */
    auto invTransform = nvg__getMatrix(paint);
    //fast path
    auto& command = element->commands.emplace_back();
    auto begin = element->indices.size();
    float aa = (fringe * 0.5f + strokeWidth * 0.5f) / fringe;
    for(int i=0; i < npaths; ++i)
    {
        nvg__renderPath(element, paths[i], paint, invTransform, aa);
    }
    command.index_count = static_cast<uint32_t>(element->indices.size() - begin);
    command.first_index = static_cast<uint32_t>(begin);
    command.material = static_cast<GDIMaterialId>(paint->material);
    command.texture = static_cast<GDITextureId>(paint->image);
    if (command.texture && ((GDIResource*)command.texture)->get_state() != EGDIResourceState::Okay)
    {
        command.texture = nullptr;
    }
}

//
int GDIDeviceNVG::initialize() SKR_NOEXCEPT
{
    return 0;
}

int GDIDeviceNVG::finalize() SKR_NOEXCEPT
{
    return 0;
}

GDIDeviceNVG::~GDIDeviceNVG()
{
    finalize();
}

void GDIElementNVG::begin_frame(float devicePixelRatio)
{
    nvgBeginFrame(nvg, devicePixelRatio);
    // TODO: retainer box
    const bool dirty = true;
    if (dirty)
    {
        commands.clear();
        vertices.clear();
        indices.clear();
    }
}

void GDIElementNVG::begin_path()
{
    nvgBeginPath(nvg);
}

void GDIElementNVG::arc(float cx, float cy, float r, float a0, float a1, EGDIWinding dir)
{
    const uint32_t dirIndex = static_cast<uint32_t>(dir) - 1;
    const int32_t WindingLUT[] = { NVG_CW, NVG_CCW };
    nvgArc(nvg, cx, cy, r, a0, a1, WindingLUT[dirIndex]);
}

void GDIElementNVG::close_path()
{
    nvgClosePath(nvg);
}

void GDIElementNVG::rect(float x, float y, float w, float h)
{
    nvgRect(nvg, x, y, w, h);
}

void GDIElementNVG::circle(float cx, float cy, float r)
{
    nvgCircle(nvg, cx, cy, r);
}

void GDIElementNVG::rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{
    nvgRoundedRectVarying(nvg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

void GDIElementNVG::translate(float x, float y)
{
    nvgTranslate(nvg, x, y);
}

void GDIElementNVG::rotate(float r)
{
    nvgRotate(nvg, r);
}

void GDIElementNVG::move_to(float x, float y)
{
    nvgMoveTo(nvg, x, y);
}

void GDIElementNVG::line_to(float x, float y)
{
    nvgLineTo(nvg, x, y);
}

void GDIElementNVG::stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    gdi_paint = nullptr;
    nvgStrokeColor(nvg, nvgRGBA(r, g, b, a));
}

void GDIElementNVG::stroke_color(float r, float g, float b, float a)
{
    gdi_paint = nullptr;
    nvgStrokeColor(nvg, nvgRGBAf(r, g, b, a));
}

void GDIElementNVG::stroke_paint(GDIPaint* paint)
{
    gdi_paint = static_cast<GDIPaintNVG*>(paint);
    auto nvg_paint = static_cast<GDIPaintNVG*>(paint);
    nvgStrokePaint(nvg, nvg_paint->nvg_paint);
}

void GDIElementNVG::stroke_width(float size)
{
    nvgStrokeWidth(nvg, size);
}

void GDIElementNVG::stroke()
{
    nvgStroke(nvg);
}

void GDIElementNVG::path_winding(EGDISolidity dir)
{
    const uint32_t dirIndex = static_cast<uint32_t>(dir) - 1;
    const int32_t SolidityLUT[] = { NVG_SOLID, NVG_HOLE };
    nvgPathWinding(nvg, SolidityLUT[dirIndex]);
}

void GDIElementNVG::fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    nvgFillColor(nvg, nvgRGBA(r, g, b, a));
}

void GDIElementNVG::fill_color(float r, float g, float b, float a)
{
    nvgFillColor(nvg, nvgRGBAf(r, g, b, a));
}

void GDIElementNVG::fill_paint(GDIPaint* paint)
{
    gdi_paint = static_cast<GDIPaintNVG*>(paint);
    auto nvg_paint = static_cast<GDIPaintNVG*>(paint);
    nvgFillPaint(nvg, nvg_paint->nvg_paint);
}

void GDIElementNVG::fill()
{
    nvgFill(nvg);
}

void GDIElementNVG::fill_no_aa()
{
    auto params = nvgInternalParams(nvg);
    params->edgeAntiAlias = false;
    nvgFill(nvg);
    params->edgeAntiAlias = true;
}

void GDIElementNVG::restore()
{
    nvgRestore(nvg);
}

void GDIElementNVG::save()
{
    nvgSave(nvg);
}

void GDIPaintNVG::set_pattern(float cx, float cy, float w, float h, float angle, GDITextureId texture, skr_float4_t ocol) SKR_NOEXCEPT
{
    NVGcolor color = nvgRGBAf(ocol.x, ocol.y, ocol.z, ocol.w);
    nvg_paint = nvgImagePattern(nullptr, cx, cy, w, h, angle, texture, color);
}

void GDIPaintNVG::set_pattern(float cx, float cy, float w, float h, float angle, GDIMaterialId material, skr_float4_t ocol) SKR_NOEXCEPT
{
    NVGcolor color = nvgRGBAf(ocol.x, ocol.y, ocol.z, ocol.w);
    nvg_paint = nvgMaterialPattern(nullptr, cx, cy, w, h, angle, material, color);
} 

void GDIPaintNVG::enable_imagespace_coordinate(bool enable) SKR_NOEXCEPT
{
    coordinate_method_override = enable ? NVG : ImageSpace;
}

void GDIPaintNVG::custom_vertex_color(skr_gdi_custom_vertex_painter_t painter, void* usrdata) SKR_NOEXCEPT
{
    custom_painter = painter;
    custom_painter_data = usrdata;
}

void GDICanvasNVG::add_element(GDIElement* element) SKR_NOEXCEPT
{
    GDICanvasPrivate::add_element(element);
}

GDIViewport* GDIDeviceNVG::create_viewport()
{
    return SkrNew<GDIViewportNVG>();
}

void GDIDeviceNVG::free_viewport(GDIViewport* canvas)
{
    SkrDelete(canvas);
}

GDICanvas* GDIDeviceNVG::create_canvas()
{
    return SkrNew<GDICanvasNVG>();
}

void GDIDeviceNVG::free_canvas(GDICanvas* group)
{
    SkrDelete(group);
}

GDIElement* GDIDeviceNVG::create_element()
{
    auto element = SkrNew<GDIElementNVG>();
    auto params = make_zeroed<NVGparams>();
    params.renderFill = nvg__renderFill;
    params.renderStroke = nvg__renderStroke;
    params.userPtr = element;
    params.edgeAntiAlias = true;
    element->nvg = nvgCreateInternal(&params);
    return element;
}

void GDIDeviceNVG::free_element(GDIElement* element)
{
    SkrDelete(element);
}

GDIPaint* GDIDeviceNVG::create_paint()
{
    return SkrNew<GDIPaintNVG>();
}

void GDIDeviceNVG::free_paint(GDIPaint* paint)
{
    SkrDelete(paint);
}

} }