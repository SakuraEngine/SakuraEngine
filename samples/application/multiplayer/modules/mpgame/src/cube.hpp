#include "misc/types.h"

struct CubeGeometry {
    const skr_float3_t g_Positions[24] = {
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
    const skr_float2_t g_TexCoords[24] = {
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
    const skr_float2_t g_TexCoords2[24] = {
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
    const skr_float3_t g_Normals[24] = {
        { 0.0f, 0.0f, -1.0f }, // front face
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },

        { 1.0f, 0.0f, 0.0f }, // right side face
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },

        { -1.0f, 0.0f, 0.0f }, // left side face
        { -1.0f, 0.0f, 0.0f },
        { -1.0f, 0.0f, 0.0f },
        { -1.0f, 0.0f, 0.0f },

        { 0.0f, 0.0f, 1.0f }, // back face
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },

        { 0.0f, 1.0f, 0.0f }, // top face
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 0.0f, -1.0f, 0.0f }, // bottom face
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f },
    };
    const skr_float4_t g_Tangents[24] = {

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