#include "imgui.h"
#include "imgui_internal.h"

// API for exported functions
#define CIMGUI_API extern "C"

#include <stdio.h>
#include <stdarg.h>

// Wrap this in a namespace to keep it separate from the C++ API
namespace cimgui
{
extern "C"
{
#include "%OUTPUT_HEADER_NAME_NO_INTERNAL%"
#include "%OUTPUT_HEADER_NAME%"
}
}
