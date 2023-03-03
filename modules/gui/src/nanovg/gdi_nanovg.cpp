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

static void nvg__renderPath(SGDIElementNVG* element, const NVGpath& path, NVGpaint* paint, const skr_float4x4_t& transform, float fringe)
{
    skr_float2_t extend{paint->extent[0], paint->extent[1]};
    auto& vertices = element->vertices;
    auto& indices = element->indices;
    auto push_vertex = [&](const NVGvertex& nv)
    {
        SGDIVertex v;
        v.position = {nv.x, nv.y};
        v.aa = {nv.u, fringe};
        const auto pos = rtm::vector_load((const uint8_t*)&v.position);
        const auto col0 = rtm::vector_set(transform.M[0][0], transform.M[0][1], transform.M[0][2], transform.M[0][3]);
        const auto col1 = rtm::vector_set(transform.M[1][0], transform.M[1][1], transform.M[1][2], transform.M[1][3]);
        const auto col2 = rtm::vector_set(transform.M[2][0], transform.M[2][1], transform.M[2][2], transform.M[2][3]);
        const auto col3 = rtm::vector_set(transform.M[3][0], transform.M[3][1], transform.M[3][2], transform.M[3][3]);
        const auto trans = rtm::matrix_set(col0, col1, col2, col3);
        auto imgSpace = rtm::matrix_mul_vector(pos, trans);
        const auto imgSpaceX = rtm::vector_get_x(imgSpace);
        const auto imgSpaceY = rtm::vector_get_y(imgSpace);
        v.texcoord = nvg__remapUV({ imgSpaceX, imgSpaceY }, extend, paint->box);
        v.color = ToColor32ABGR(paint->innerColor);
        vertices.push_back(v);
    };
    //auto& path = paths[i];
    if(path.nfill)
    {
        vertices.reserve(vertices.size() + path.nfill);
        indices.reserve(indices.size() + path.nfill * 3);
        const auto start = static_cast<SGDIElementNVG::index_t>(vertices.size());
        for(int j=0; j<path.nfill; ++j)
        {
            push_vertex(path.fill[j]);
            if(j<path.nfill-2)
            {
                const auto id = static_cast<SGDIElementNVG::index_t>(vertices.size());
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
            push_vertex(path.stroke[j]);
            if(j<path.nstroke-2)
            {
                const auto id = static_cast<SGDIElementNVG::index_t>(vertices.size() - 1);
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
    auto element = (SGDIElementNVG*)uptr;
    /*
    PrimDrawResource resource;
    resource.texture = (TextureHandle)paint->image;
    resource.material = (MaterialHandle)paint->material;
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
            nvg__renderPath(element, paths[i], paint, invTransform, 1.f);
        command.index_count = static_cast<uint32_t>(element->indices.size() - begin);
        command.first_index = static_cast<uint32_t>(begin);
    }
    //slow path
    else 
    {
    }
}

static void nvg__renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, float fringe,
								float strokeWidth, const NVGpath* paths, int npaths)
{
    auto element = (SGDIElementNVG*)uptr;
    /*
    PrimDrawResource resource;
    resource.texture = (TextureHandle)paint->image;
    resource.material = (MaterialHandle)paint->material;
    resource.compositeOperation = compositeOperation;
    resource.noGamma = paint->noGamma;
    */
    auto invTransform = nvg__getMatrix(paint);
    //fast path
    /*
    auto& command = dc->current->GetCommand(resource);
    auto& indices = dc->current->indices;
    */
    auto& command = element->commands.emplace_back();
    auto begin = element->indices.size();
    float aa = (fringe * 0.5f + strokeWidth * 0.5f) / fringe;
    for(int i=0; i < npaths; ++i)
        nvg__renderPath(element, paths[i], paint, invTransform, aa);
    command.index_count = static_cast<uint32_t>(element->indices.size() - begin);
    command.first_index = static_cast<uint32_t>(begin);
}

//
// HACK

inline static void read_bytes(const char* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen(file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(
const char* virtual_path, uint32_t** bytes, uint32_t* length,
ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat(shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat(shader_file, ".dxil");
            break;
        default:
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}

CGPURenderPipelineId create_render_pipeline(CGPUDeviceId device, ECGPUFormat target_format, CGPUVertexLayout* pLayout)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("GUI/vertex", &vs_bytes, &vs_length, device->adapter->instance->backend);
    read_shader_bytes("GUI/pixel", &fs_bytes, &fs_length, device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "FragmentShaderLibrary";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    auto root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = pLayout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &target_format;
    auto pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    return pipeline;
}
// HACK

int SGDIDeviceNVG::initialize(CGPUDeviceId device) SKR_NOEXCEPT
{
    const uint32_t pos_offset = static_cast<uint32_t>(offsetof(SGDIVertex, position));
    const uint32_t texcoord_offset = static_cast<uint32_t>(offsetof(SGDIVertex, texcoord));
    const uint32_t aa_offset = static_cast<uint32_t>(offsetof(SGDIVertex, aa));
    const uint32_t uv_offset = static_cast<uint32_t>(offsetof(SGDIVertex, clipUV));
    const uint32_t uv2_offset = static_cast<uint32_t>(offsetof(SGDIVertex, clipUV2));
    const uint32_t color_offset = static_cast<uint32_t>(offsetof(SGDIVertex, color));
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 0, pos_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, texcoord_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "AA", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, aa_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "UV", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "UV_Two", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv2_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[5] = { "COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, color_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[6] = { "TRANSFORM", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 1, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 7;

    pipeline = create_render_pipeline(device, CGPU_FORMAT_B8G8R8A8_UNORM, &vertex_layout);
    
    return 0;
}

void SGDIDeviceNVG::finalize() SKR_NOEXCEPT
{
    auto rs = pipeline->root_signature;
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(rs);
}

SGDIDeviceNVG::~SGDIDeviceNVG()
{
    finalize();
}

void SGDIElementNVG::begin_frame(float devicePixelRatio)
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

void SGDIElementNVG::begin_path()
{
    nvgBeginPath(nvg);
}

void SGDIElementNVG::rect(float x, float y, float w, float h)
{
    nvgRect(nvg, x, y, w, h);
}

void SGDIElementNVG::rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft)
{
    nvgRoundedRectVarying(nvg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

void SGDIElementNVG::fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    nvgFillColor(nvg, nvgRGBA(r, g, b, a));
}

void SGDIElementNVG::fill_color(float r, float g, float b, float a)
{
    nvgFillColor(nvg, nvgRGBAf(r, g, b, a));
}

void SGDIElementNVG::fill_paint(SGDIPaint* paint)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void SGDIElementNVG::fill()
{
    nvgFill(nvg);
}

void SGDICanvasNVG::add_element(SGDIElement* element, const skr_float4_t& transform)
{
    SGDICanvas::add_element(element, transform);
}

SGDICanvasGroup* SGDIDeviceNVG::create_canvas_group()
{
    return SkrNew<SGDICanvasGroupNVG>();
}

void SGDIDeviceNVG::free_canvas_group(SGDICanvasGroup* canvas_group)
{
    SkrDelete(canvas_group);
}

SGDICanvas* SGDIDeviceNVG::create_canvas()
{
    return SkrNew<SGDICanvasNVG>();
}

void SGDIDeviceNVG::free_canvas(SGDICanvas* canvas)
{
    SkrDelete(canvas);
}

SGDIElement* SGDIDeviceNVG::create_element()
{
    auto element = SkrNew<SGDIElementNVG>();
    auto params = make_zeroed<NVGparams>();
    params.renderFill = nvg__renderFill;
    params.renderStroke = nvg__renderStroke;
    params.userPtr = element;
    params.edgeAntiAlias = true;
    element->nvg = nvgCreateInternal(&params);
    return element;
}

void SGDIDeviceNVG::free_element(SGDIElement* element)
{
    SkrDelete(element);
}

void SGDIDeviceNVG::render(skr::render_graph::RenderGraph* rg, SGDICanvasGroup* canvas_group)
{
    auto nvg_canvas_group = static_cast<SGDICanvasGroupNVG*>(canvas_group);
    const auto canvas_span = canvas_group->all_canvas();
    if (canvas_span.empty()) return;
    uint64_t vertex_count = 0u;
    uint64_t index_count = 0u;
    uint64_t transform_count = 0u;
    uint64_t command_count = 0u;
    // 1. loop prepare counters & render data
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        const auto nvg_element = static_cast<SGDIElementNVG*>(element);
        vertex_count += nvg_element->vertices.size();
        index_count += nvg_element->indices.size();
        transform_count += 1;
        command_count += nvg_element->commands.size();
    }
    }
    nvg_canvas_group->render_commands.clear();
    nvg_canvas_group->render_vertices.clear();
    nvg_canvas_group->render_indices.clear();
    nvg_canvas_group->render_transforms.clear();
    nvg_canvas_group->render_vertices.reserve(vertex_count);
    nvg_canvas_group->render_indices.reserve(index_count);
    nvg_canvas_group->render_transforms.reserve(transform_count);
    nvg_canvas_group->render_commands.reserve(command_count);
    uint64_t vb_cursor = 0u;
    uint64_t ib_cursor = 0u;
    uint64_t tb_cursor = 0u;
    for (auto canvas : canvas_span)
    {
    for (auto element : canvas->all_elements())
    {
        const auto nvg_element = static_cast<SGDIElementNVG*>(element);
        vb_cursor = nvg_canvas_group->render_vertices.size();
        ib_cursor = nvg_canvas_group->render_indices.size();
        tb_cursor = nvg_canvas_group->render_transforms.size();
        nvg_canvas_group->render_vertices.insert(nvg_canvas_group->render_vertices.end(), nvg_element->vertices.begin(), nvg_element->vertices.end());
        nvg_canvas_group->render_indices.insert(nvg_canvas_group->render_indices.end(), nvg_element->indices.begin(), nvg_element->indices.end());
        // TODO: deal with transform
        auto& transform = nvg_canvas_group->render_transforms.emplace_back();
        transform.transform.M[0][0] = 900.0f;
        transform.transform.M[0][1] = 900.0f;
        for (auto command : nvg_element->commands)
        {
            SGDIElementDrawCommandNVG2 command2 = {};
            command2.first_index = command.first_index;
            command2.index_count = command.index_count;
            command2.ib_offset = ib_cursor * sizeof(SGDIElementNVG::index_t);
            command2.vb_offset = vb_cursor * sizeof(SGDIVertex);
            command2.tb_offset = tb_cursor * sizeof(SGDITransform);
            nvg_canvas_group->render_commands.emplace_back(command2);
        }
    }
    }

    // 2. prepare render resource
    const uint64_t vertices_size = vertex_count * sizeof(SGDIVertex);
    const uint64_t indices_size = index_count * sizeof(SGDIElementNVG::index_t);
    const uint64_t transform_size = transform_count * sizeof(SGDITransform);
    const bool useCVV = false;
    auto vertex_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_vertex_buffer")
                .size(vertices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto transform_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_transform_buffer")
                .size(transform_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto index_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name("nvg_index_buffer")
                .size(indices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        });
    nvg_canvas_group->vertex_buffers.clear();
    nvg_canvas_group->transform_buffers.clear();
    nvg_canvas_group->index_buffers.clear();
    nvg_canvas_group->vertex_buffers.emplace_back(vertex_buffer);
    nvg_canvas_group->transform_buffers.emplace_back(transform_buffer);
    nvg_canvas_group->index_buffers.emplace_back(index_buffer);

    // 3. copy/upload geometry data to GPU
    if (!useCVV)
    {
        auto upload_buffer_handle = rg->create_buffer(
            [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            ZoneScopedN("ConstructUploadPass");
            builder.set_name("nvg_upload_buffer")
                    .size(indices_size + vertices_size + transform_size)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
            });
        rg->add_copy_pass(
            [=](render_graph::RenderGraph& g, render_graph::CopyPassBuilder& builder) {
            ZoneScopedN("ConstructCopyPass");
            builder.set_name("nvg_copy_pass")
                .buffer_to_buffer(upload_buffer_handle.range(0, vertices_size), vertex_buffer.range(0, vertices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size, vertices_size + indices_size), index_buffer.range(0, indices_size))
                .buffer_to_buffer(upload_buffer_handle.range(vertices_size + indices_size, vertices_size + indices_size + transform_size), transform_buffer.range(0, transform_size));
            },
            [upload_buffer_handle, canvas_group](render_graph::RenderGraph& g, render_graph::CopyPassContext& context){
                auto upload_buffer = context.resolve(upload_buffer_handle);
                auto nvg_canvas_group = static_cast<SGDICanvasGroupNVG*>(canvas_group);
                const uint64_t vertices_count = nvg_canvas_group->render_vertices.size();
                const uint64_t indices_count = nvg_canvas_group->render_indices.size();
                SGDIVertex* vtx_dst = (SGDIVertex*)upload_buffer->cpu_mapped_address;
                SGDIElementNVG::index_t* idx_dst = (SGDIElementNVG::index_t*)(vtx_dst + vertices_count);
                SGDITransform* transform_dst = (SGDITransform*)(idx_dst + indices_count);
                const skr::span<SGDIVertex> render_vertices = nvg_canvas_group->render_vertices;
                const skr::span<SGDIElementNVG::index_t> render_indices = nvg_canvas_group->render_indices;
                const skr::span<SGDITransform> render_transforms = nvg_canvas_group->render_transforms;
                memcpy(vtx_dst, render_vertices.data(), vertices_count * sizeof(SGDIVertex));
                memcpy(idx_dst, render_indices.data(), indices_count * sizeof(SGDIElementNVG::index_t));
                memcpy(transform_dst, render_transforms.data(), render_transforms.size() * sizeof(SGDITransform));
            });
    }

    // 4. loop & record render commands
    skr::render_graph::TextureRTVHandle target = rg->get_texture("backbuffer");
    // skr::vector<SGDICanvas*> canvas_copy(canvas_span.begin(), canvas_span.end());
    rg->add_render_pass([&](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
        ZoneScopedN("ConstructRenderPass");
        builder.set_name("nvg_gdi_render_pass")
            .set_pipeline(pipeline)
            .use_buffer(vertex_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(transform_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(index_buffer, CGPU_RESOURCE_STATE_INDEX_BUFFER)
            .write(0, target, CGPU_LOAD_ACTION_CLEAR);
    },
    [target, canvas_group, useCVV, index_buffer, vertex_buffer, transform_buffer]
    (render_graph::RenderGraph& g, render_graph::RenderPassContext& context) {
        ZoneScopedN("GDI(NVG)RenderPass");
        const auto target_desc = g.resolve_descriptor(target);
        auto resolved_ib = context.resolve(index_buffer);
        auto resolved_vb = context.resolve(vertex_buffer);
        auto resolved_tb = context.resolve(transform_buffer);
        CGPUBufferId vertex_streams[2] = { resolved_vb, resolved_tb };
        const uint32_t vertex_stream_strides[2] = { sizeof(SGDIVertex), sizeof(SGDITransform) };
        cgpu_render_encoder_set_viewport(context.encoder,
            0.0f, 0.0f,
            (float)target_desc->width,
            (float)target_desc->height,
            0.f, 1.f);
        cgpu_render_encoder_set_scissor(context.encoder,
            0, 0, 
            target_desc->width, target_desc->height);
        const auto nvg_canvs_group = static_cast<SGDICanvasGroupNVG*>(canvas_group);
        const skr::span<SGDIElementDrawCommandNVG2> render_commands = nvg_canvs_group->render_commands;
        for (const auto& command : render_commands)
        {
            const uint32_t vertex_stream_offsets[2] = { command.vb_offset, command.tb_offset };
            cgpu_render_encoder_bind_index_buffer(context.encoder,
                resolved_ib, sizeof(SGDIElementNVG::index_t), command.ib_offset);
            cgpu_render_encoder_bind_vertex_buffers(context.encoder,
                2, vertex_streams, vertex_stream_strides, vertex_stream_offsets);
            cgpu_render_encoder_draw_indexed_instanced(context.encoder,
                command.index_count,command.first_index,
                1, 0, 0);
        }
    });
}

} }