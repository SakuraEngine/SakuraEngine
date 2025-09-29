#pragma once
#include <stdint.h>
#include <type_traits>

namespace skr::CppSL {

enum struct EVariableQualifier : uint32_t {
    None = 0,
    Const = 1,
    Inout = 2,
    Out = 3,
    GroupShared = 4
};

enum struct UnaryOp : uint32_t {
    PLUS,   // +x
    MINUS,  // -x
    NOT,    // !x
    BIT_NOT,// ~x

    POST_INC, // x++
    POST_DEC, // x--
    PRE_INC,  // ++x
    PRE_DEC,  // --x
};

enum struct BinaryOp : uint32_t {

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    BIT_AND,
    BIT_OR,
    BIT_XOR,
    SHL,
    SHR,
    AND,
    OR,

    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    EQUAL,
    NOT_EQUAL,

    ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,

    BIT_OR_ASSIGN,
    BIT_XOR_ASSIGN,
    BIT_AND_ASSIGN,
    SHL_ASSIGN,
    SHR_ASSIGN,
    
    COMMA,

    COUNT
};

enum struct ShaderStage : uint32_t
{
    None,
    Vertex,
    Hull,
    Domain,
    Geometry,
    Fragment,
    Compute,
    RayGen,
    ClosestHit,
    Miss,
    AnyHit,
    Count
};

enum struct SemanticType : uint32_t
{
    Invalid,
    Position,     // SV_Position / [[position]]
    ClipDistance, // SV_ClipDistance / [[clip_distance]]
    CullDistance, // SV_CullDistance / [[cull_distance]]

    RenderTarget0, // SV_Target0 / [[color(0)]]
    RenderTarget1, // SV_Target1 / [[color(1)]]
    RenderTarget2, // SV_Target2 / [[color(2)]]
    RenderTarget3, // SV_Target3 / [[color(3)]]
    RenderTarget4, // SV_Target4 / [[color(4)]]
    RenderTarget5, // SV_Target5 / [[color(5)]]
    RenderTarget6, // SV_Target6 / [[color(6)]]
    RenderTarget7, // SV_Target7 / [[color(7)]]

    Depth,             // SV_Depth / [[depth(any)]]
    DepthGreaterEqual, // SV_DepthGreaterEqual / [[depth(greater_equal)]]
    DepthLessEqual,    // SV_DepthLessEqual / [[depth(less_equal)]]
    StencilRef,        // SV_StencilRef / [[stencil]]
    
    VertexID,   // SV_VertexID / [[vertex_id]]
    InstanceID, // SV_InstanceID / [[instance_id]]

    GSInstanceID,     // SV_GSInstanceID / !
    TessFactor,       // SV_TessFactor / !
    InsideTessFactor, // SV_InsideTessFactor / !
    DomainLocation,   // SV_DomainLocation / !
    ControlPointID,   // SV_ControlPointID / !

    PrimitiveID,  // SV_PrimitiveID / [[primitive_id]]
    IsFrontFace,  // SV_IsFrontFace / [[front_facing]]
    SampleIndex,  // SV_SampleIndex / [[sample_id]]
    SampleMask,   // SV_Coverage / [[sample_mask]]
    Barycentrics, // SV_Barycentrics / [[barycentric_coord]]

    ThreadID,               // SV_DispatchThreadID / [[thread_position_in_grid]]
    GroupID,                // SV_GroupID / [[threadgroup_position_in_grid]]
    ThreadPositionInGroup,  // SV_GroupThreadID / [[thread_position_in_threadgroup]]
    ThreadIndexInGroup,     // SV_GroupIndex / [[thread_index_in_threadgroup]]3
    
    RayPayload,     // SV_RayPayload / !

    ViewID, // SV_ViewID / [[render_target_array_index]]
    Count
};

enum struct InterpolationMode : uint32_t
{
    invalid,
    linear,          // HLSL: linear / Metal: [[perspective]]
    nointerpolation, // HLSL: nointerpolation / Metal: [[flat]]
    centroid,        // HLSL: centroid / Metal: [[centroid_perspective]]
    sample,          // HLSL: sample / Metal: [[sample_perspective]]
    noperspective,   // HLSL: noperspective / Metal: [[center_no_perspective]]
    Count
};

enum struct BufferFlags : uint32_t
{
    None = 0x0,
    Read = 0x1,
    ReadWrite = 0x2,
};
inline constexpr BufferFlags operator|(BufferFlags lhs, BufferFlags rhs) {
    return static_cast<BufferFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
inline constexpr BufferFlags operator&(BufferFlags lhs, BufferFlags rhs) {
    return static_cast<BufferFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
inline constexpr bool has_flag(BufferFlags flags, BufferFlags flag) {
    return (flags & flag) != BufferFlags::None;
}

enum struct TextureFlags : uint32_t
{
    None = 0x0,
    Read = 0x1,
    ReadWrite = 0x2
};
inline constexpr TextureFlags operator|(TextureFlags lhs, TextureFlags rhs) {
    return static_cast<TextureFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
inline constexpr TextureFlags operator&(TextureFlags lhs, TextureFlags rhs) {
    return static_cast<TextureFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
inline constexpr bool has_flag(TextureFlags flags, TextureFlags flag) {
    return (flags & flag) != TextureFlags::None;
}

enum struct ArrayFlags : uint32_t
{
    None = 0x0,
    Shared = 0x1
};
inline constexpr ArrayFlags operator|(ArrayFlags lhs, ArrayFlags rhs) {
    return static_cast<ArrayFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
inline constexpr ArrayFlags operator&(ArrayFlags lhs, ArrayFlags rhs) {
    return static_cast<ArrayFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
inline constexpr bool has_flag(ArrayFlags flags, ArrayFlags flag) {
    return (flags & flag) != ArrayFlags::None;
}

enum struct RayQueryFlags : uint32_t
{
    None = 0x0,
    ForceOpaque = 0x1,
    ForceNonOpaque = 0x2,
    AcceptFirstAndEndSearch = 0x4,
    CullOpaque = 0x8,
    CullNonOpaque = 0x10,
    CullFrontFace = 0x20,
    CullBackFace = 0x40,
    CullTriangle = 0x80,
    CullProcedural = 0x100
};
inline constexpr RayQueryFlags operator|(RayQueryFlags lhs, RayQueryFlags rhs) {
    return static_cast<RayQueryFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}
inline constexpr RayQueryFlags operator&(RayQueryFlags lhs, RayQueryFlags rhs) {
    return static_cast<RayQueryFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}
inline constexpr bool has_flag(RayQueryFlags flags, RayQueryFlags flag) {
    return (flags & flag) != RayQueryFlags::None;
}

// https://www.w3.org/TR/WGSL/#alignment-and-size
struct alignas(2) GPUHalf
{
    int16_t v;
};

struct alignas(4) GPUBool
{
    int32_t v;
};

template <typename T, int N>
inline static constexpr int gpu_vec_align()
{
    constexpr bool is_float = std::is_same_v<T, float>;
    constexpr bool is_half = std::is_same_v<T, GPUHalf>;
    constexpr bool is_bool = std::is_same_v<T, GPUBool>;
    constexpr bool is_int = std::is_same_v<T, int32_t>;
    constexpr bool is_uint = std::is_same_v<T, uint32_t>;
    // Many GPUs cannot implement single-byte writes without introducing potential data races. By specifying that a bool value occupies 4 bytes with 4 byte alignment, implementations can support adjacent boolean values in memory without introducing data races.
    if constexpr (is_half && N == 3)
        return 8;
    else if constexpr (is_half && N == 4)
        return 8;
    else if constexpr ((is_bool || is_float || is_int || is_uint) && N == 2)
        return 8;
    else if constexpr ((is_bool || is_float || is_int || is_uint) && N == 3)
        return 16;
    else if constexpr ((is_bool || is_float || is_int || is_uint) && N == 4)
        return 16;
    return 4;
} 

template <typename T, int N, int M>
inline static constexpr int gpu_mat_align()
{
    constexpr bool is_float = std::is_same_v<T, float>;
    constexpr bool is_half = std::is_same_v<T, GPUHalf>;
    constexpr bool is_bool = std::is_same_v<T, GPUBool>;
    constexpr bool is_int = std::is_same_v<T, int32_t>;
    constexpr bool is_uint = std::is_same_v<T, uint32_t>;
    if constexpr (is_half && M == 2)
        return 4;
    else if constexpr (is_float && M > 2)
        return 16;
    return 8;
}

template <typename T, size_t N>
struct alignas(gpu_vec_align<T, N>()) vec { T v[N]; };

template <typename T, size_t N, size_t M>
struct alignas(gpu_mat_align<T, N, M>()) matrix { T v[N][M]; };

template <typename T, size_t N>
struct alignas(gpu_mat_align<T, N, 3>()) matrix<T, N, 3> { T v[N][4]; };

static_assert(sizeof(GPUBool) == 4, "GPU bool size mismatch");
static_assert(alignof(GPUBool) == 4, "GPU bool alignment mismatch");

static_assert(sizeof(int32_t) == 4, "GPU int32_t size mismatch");
static_assert(alignof(int32_t) == 4, "GPU int32_t alignment mismatch");

static_assert(sizeof(uint32_t) == 4, "GPU uint32_t size mismatch");
static_assert(alignof(uint32_t) == 4, "GPUU uint32_t alignment mismatch");

static_assert(sizeof(float) == 4, "GPU float size mismatch");
static_assert(alignof(float) == 4, "GPU float alignment mismatch");

static_assert(sizeof(GPUHalf) == 2, "GPU half size mismatch");
static_assert(alignof(GPUHalf) == 2, "GPU half alignment mismatch");

static_assert(sizeof(vec<GPUBool, 2>) == 8, "bool2 size mismatch");
static_assert(alignof(vec<GPUBool, 2>) == 8, "bool2 alignment mismatch");

static_assert(sizeof(vec<int32_t, 2>) == 8, "int2 size mismatch");
static_assert(alignof(vec<int32_t, 2>) == 8, "int2 alignment mismatch");

static_assert(sizeof(vec<uint32_t, 2>) == 8, "uint2 size mismatch");
static_assert(alignof(vec<uint32_t, 2>) == 8, "uint2 alignment mismatch");

static_assert(sizeof(vec<float, 2>) == 8, "float2 size mismatch");
static_assert(alignof(vec<float, 2>) == 8, "float2 alignment mismatch");

static_assert(sizeof(vec<GPUHalf, 2>) == 4, "half2 size mismatch");
static_assert(alignof(vec<GPUHalf, 2>) == 4, "half2 alignment mismatch");

static_assert(sizeof(vec<GPUBool, 3>) == 16, "bool3 size mismatch");
static_assert(alignof(vec<GPUBool, 3>) == 16, "bool3 alignment mismatch");

static_assert(sizeof(vec<int32_t, 3>) == 16, "int3 size mismatch");
static_assert(alignof(vec<int32_t, 3>) == 16, "int3 alignment mismatch");

static_assert(sizeof(vec<uint32_t, 3>) == 16, "uint3 size mismatch");
static_assert(alignof(vec<uint32_t, 3>) == 16, "uint3 alignment mismatch");

static_assert(sizeof(vec<float, 3>) == 16, "float3 size mismatch");
static_assert(alignof(vec<float, 3>) == 16, "float3 alignment mismatch");

static_assert(sizeof(vec<GPUHalf, 3>) == 8, "float3 size mismatch");
static_assert(alignof(vec<GPUHalf, 3>) == 8, "float3 alignment mismatch");

static_assert(sizeof(vec<GPUBool, 4>) == 16, "bool4 size mismatch");
static_assert(alignof(vec<GPUBool, 4>) == 16, "bool4 alignment mismatch");

static_assert(sizeof(vec<int32_t, 4>) == 16, "int4 size mismatch");
static_assert(alignof(vec<int32_t, 4>) == 16, "int4 alignment mismatch");

static_assert(sizeof(vec<uint32_t, 4>) == 16, "uint4 size mismatch");
static_assert(alignof(vec<uint32_t, 4>) == 16, "uint4 alignment mismatch");

static_assert(sizeof(vec<float, 4>) == 16, "float4 size mismatch");
static_assert(alignof(vec<float, 4>) == 16, "float4 alignment mismatch");

static_assert(sizeof(vec<GPUHalf, 4>) == 8, "float4 size mismatch");
static_assert(alignof(vec<GPUHalf, 4>) == 8, "float4 alignment mismatch");

static_assert(sizeof(matrix<float, 2, 2>) == 16, "float2x2 size mismatch");
static_assert(alignof(matrix<float, 2, 2>) == 8, "float2x2 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 2, 2>) == 8, "half2x2 size mismatch");
static_assert(alignof(matrix<GPUHalf, 2, 2>) == 4, "half2x2 alignment mismatch");

static_assert(sizeof(matrix<float, 3, 2>) == 24, "float3x2 size mismatch");
static_assert(alignof(matrix<float, 3, 2>) == 8, "float3x2 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 3, 2>) == 12, "half3x2 size mismatch");
static_assert(alignof(matrix<GPUHalf, 3, 2>) == 4, "half3x2 alignment mismatch");

static_assert(sizeof(matrix<float, 4, 2>) == 32, "float4x2 size mismatch");
static_assert(alignof(matrix<float, 4, 2>) == 8, "float4x2 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 4, 2>) == 16, "half4x2 size mismatch");
static_assert(alignof(matrix<GPUHalf, 4, 2>) == 4, "half4x2 alignment mismatch");

static_assert(sizeof(matrix<float, 2, 3>) == 32, "float2x3 size mismatch");
static_assert(alignof(matrix<float, 2, 3>) == 16, "float2x3 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 2, 3>) == 16, "half2x3 size mismatch");
static_assert(alignof(matrix<GPUHalf, 2, 3>) == 8, "half2x3 alignment mismatch");

static_assert(sizeof(matrix<float, 3, 3>) == 48, "float3x3 size mismatch");
static_assert(alignof(matrix<float, 3, 3>) == 16, "float3x3 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 3, 3>) == 24, "half3x3 size mismatch");
static_assert(alignof(matrix<GPUHalf, 3, 3>) == 8, "half3x3 alignment mismatch");

static_assert(sizeof(matrix<float, 4, 3>) == 64, "float4x3 size mismatch");
static_assert(alignof(matrix<float, 4, 3>) == 16, "float4x3 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 4, 3>) == 32, "half4x3 size mismatch");
static_assert(alignof(matrix<GPUHalf, 4, 3>) == 8, "half4x3 alignment mismatch");

static_assert(sizeof(matrix<float, 2, 4>) == 32, "float2x4 size mismatch");
static_assert(alignof(matrix<float, 2, 4>) == 16, "float2x4 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 2, 4>) == 16, "half2x4 size mismatch");
static_assert(alignof(matrix<GPUHalf, 2, 4>) == 8, "half2x4 alignment mismatch");

static_assert(sizeof(matrix<float, 3, 4>) == 48, "float3x4 size mismatch");
static_assert(alignof(matrix<float, 3, 4>) == 16, "float3x4 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 3, 4>) == 24, "half3x4 size mismatch");
static_assert(alignof(matrix<GPUHalf, 3, 4>) == 8, "half3x4 alignment mismatch");

static_assert(sizeof(matrix<float, 4, 4>) == 64, "float4x4 size mismatch");
static_assert(alignof(matrix<float, 4, 4>) == 16, "float4x4 alignment mismatch");

static_assert(sizeof(matrix<GPUHalf, 4, 4>) == 32, "half4x4 size mismatch");
static_assert(alignof(matrix<GPUHalf, 4, 4>) == 8, "half4x4 alignment mismatch");

} // namespace skr::CppSL