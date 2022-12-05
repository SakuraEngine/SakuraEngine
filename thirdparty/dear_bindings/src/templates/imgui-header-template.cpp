#include "imgui.h"
#include "imgui_internal.h"

#include <stdio.h>
#include <stdarg.h>

// Wrap this in a namespace to keep it separate from the C++ API
namespace cimgui
{
#include "%OUTPUT_HEADER_NAME%"
}

// Manual helpers
// These implement functionality that isn't in the original C++ API, but is useful to callers from other languages

CIMGUI_API void cimgui::ImVector_Construct(void* vector)
{
    // All ImVector classes are the same size, so it doesn't matter which we use for sizeof() here
    memset(vector, 0, sizeof(::ImVector<int>));
}

CIMGUI_API void cimgui::ImVector_Destruct(void* vector)
{
    // As with ImVector_construct(), it doesn't matter what the type parameter is here as we just want to get the
    // pointer and free it (without calling destructors or anything similar)
    ::ImVector<int>* real_vector = reinterpret_cast<::ImVector<int>*>(vector);
    if (real_vector->Data)
    {
        IM_FREE(real_vector->Data);
    }
}

#if defined(IMGUI_HAS_IMSTR)
#if IMGUI_HAS_IMSTR

// User-facing helper to convert char* to ImStr
CIMGUI_API cimgui::ImStr cimgui::ImStr_FromCharStr(const char* b)
{
    ImStr str;
    str.Begin = b;
    str.End = b ? b + strlen(b) : NULL;
    return str;
}

// Internal helper to convert char* directly to C++-style ImStr
static inline ::ImStr MarshalToCPP_ImStr_FromCharStr(const char* b)
{
    ::ImStr str;
    str.Begin = b;
    str.End = b ? b + strlen(b) : NULL;
    return str;
}
#endif // IMGUI_HAS_IMSTR
#endif // defined(IMGUI_HAS_IMSTR)
