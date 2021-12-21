#pragma once
#include "platform/configure.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API void* nswindow_create();
RUNTIME_API void* nswindow_get_content_view(void*);

#ifdef __cplusplus
}
#endif