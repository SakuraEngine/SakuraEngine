#pragma once
#include "wasm/backend/wasm3/swa_wasm3.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API struct WASM3RuntimeFunctionTable* WASM3RuntimeFunctionTableCreate();
RUNTIME_API const char* WASM3RuntimeFunctionTableAdd(struct WASM3RuntimeFunctionTable* table, const char* name, IM3Function function);
RUNTIME_API IM3Function WASM3RuntimeFunctionTableTryFind(struct WASM3RuntimeFunctionTable* table, const char* name);
RUNTIME_API void WASM3RuntimeFunctionTableFree(struct WASM3RuntimeFunctionTable* table);

#ifdef __cplusplus
} // end extern "C"
#endif