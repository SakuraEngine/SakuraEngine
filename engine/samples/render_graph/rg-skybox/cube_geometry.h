#pragma once
#include "rtm/camera_utilsf.h"
#include "SkrBase/types.h"
#include "SkrBase/math.hpp"

struct CubeGeometry {
    struct InstanceData {
        skr::float4x4 world;
    };
    static InstanceData instance_data;

    inline static uint32_t _vector_to_snorm8(const skr::float4& v)
    {
        return skr::pack_snorm8(skr::normalize(v));
    }

    const skr::float3 g_Positions[24] = {
        { -0.5f, 0.5f, -0.5f }, // front face
        { 0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },

        { 0.5f, -0.5f, -0.5f }, // right side face
        { 0.5f, 0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { 0.5f, 0.5f, -0.5f },

        { -0.5f, 0.5f, 0.5f }, // left side face
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f },
        { -0.5f, 0.5f, -0.5f },

        { 0.5f, 0.5f, 0.5f }, // back face
        { -0.5f, -0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { -0.5f, 0.5f, 0.5f },

        { -0.5f, 0.5f, -0.5f }, // top face
        { 0.5f, 0.5f, 0.5f },
        { 0.5f, 0.5f, -0.5f },
        { -0.5f, 0.5f, 0.5f },

        { 0.5f, -0.5f, 0.5f }, // bottom face
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f },
    };
    const skr::float2 g_TexCoords[24] = {
        { 0.0f, 0.0f }, // front face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 1.0f }, // right side face
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },

        { 0.0f, 0.0f }, // left side face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 0.0f }, // back face
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f },

        { 0.0f, 1.0f }, // top face
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },

        { 1.0f, 1.0f }, // bottom face
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
    };
    const uint32_t g_Normals[24] = {
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 0.0f }), // front face
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 0.0f }),

        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 0.0f }), // right side face
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 0.0f }),

        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 0.0f }), // left side face
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 0.0f }),

        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 0.0f }), // back face
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 0.0f }),

        _vector_to_snorm8({ 0.0f, 1.0f, 0.0f, 0.0f }), // top face
        _vector_to_snorm8({ 0.0f, 1.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 1.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, 1.0f, 0.0f, 0.0f }),

        _vector_to_snorm8({ 0.0f, -1.0f, 0.0f, 0.0f }), // bottom face
        _vector_to_snorm8({ 0.0f, -1.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, -1.0f, 0.0f, 0.0f }),
        _vector_to_snorm8({ 0.0f, -1.0f, 0.0f, 0.0f }),
    };
    const uint32_t g_Tangents[24] = {
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }), // front face
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),

        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 1.0f }), // right side face
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 1.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 1.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, 1.0f, 1.0f }),

        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 1.0f }), // left side face
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 1.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 1.0f }),
        _vector_to_snorm8({ 0.0f, 0.0f, -1.0f, 1.0f }),

        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 1.0f }), // back face
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ -1.0f, 0.0f, 0.0f, 1.0f }),

        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }), // top face
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),

        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }), // bottom face
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
        _vector_to_snorm8({ 1.0f, 0.0f, 0.0f, 1.0f }),
    };
    static constexpr uint32_t g_Indices[] = {
        0, 1, 2, 0, 3, 1,       // front face
        4, 5, 6, 4, 7, 5,       // left face
        8, 9, 10, 8, 11, 9,     // right face
        12, 13, 14, 12, 15, 13, // back face
        16, 17, 18, 16, 19, 17, // top face
        20, 21, 22, 20, 23, 21, // bottom face
    };
};