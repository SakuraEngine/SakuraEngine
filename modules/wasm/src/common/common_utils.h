#pragma once
#include "wasm/api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SWANamedObjectTableDeletor)(SWANamedObjectTable* table, const char* name, void* object);

SKR_WASM_API struct SWANamedObjectTable* SWAObjectTableCreate();
SKR_WASM_API void SWAObjectTableSetDeletor(struct SWANamedObjectTable* table, SWANamedObjectTableDeletor deletor);
SKR_WASM_API const char* SWAObjectTableAdd(struct SWANamedObjectTable* table, const char* name, void* object);
SKR_WASM_API void SWAObjectTableRemove(struct SWANamedObjectTable* table, const char* name, bool delete_object);
SKR_WASM_API void* SWAObjectTableTryFind(struct SWANamedObjectTable* table, const char* name);
SKR_WASM_API void SWAObjectTableFree(struct SWANamedObjectTable* table);

#ifdef __cplusplus
} // end extern "C"
#endif