#pragma once
#include "SkrRT/config.h"

#ifdef __cplusplus
extern "C" {
#endif

SKR_RUNTIME_API void* nswindow_create();
SKR_RUNTIME_API void* nswindow_get_content_view(void*);

#ifdef __cplusplus
}
#endif