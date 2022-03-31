#pragma once
#include "platform/configure.h"

#include "wasm/api.h"

#ifdef __cplusplus
extern "C" {
#endif
void* watch_source();
SWAModuleId get_available_wasm(void* watcher);
void unwatch_wasm(void* watcher);
void unwatch_source(void* watcher);
void* watch_wasm();
#ifdef __cplusplus
}
#endif