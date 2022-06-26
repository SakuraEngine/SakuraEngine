#pragma once
#include "wasm/backend/wasm3/swa_wasm3.h"

#ifdef __cplusplus
extern "C" {
#endif

SKR_WASM_API struct WASM3RuntimeFunctionTable* WASM3RuntimeFunctionTableCreate();
SKR_WASM_API const char* WASM3RuntimeFunctionTableAdd(struct WASM3RuntimeFunctionTable* table, const char* name, IM3Function function);
SKR_WASM_API IM3Function WASM3RuntimeFunctionTableTryFind(struct WASM3RuntimeFunctionTable* table, const char* name);
SKR_WASM_API void WASM3RuntimeFunctionTableFree(struct WASM3RuntimeFunctionTable* table);

#ifdef __cplusplus
} // end extern "C"
#endif