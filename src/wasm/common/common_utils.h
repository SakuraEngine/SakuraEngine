#pragma once
#include "wasm/api.h"

typedef void (*SWANamedObjectTableDeletor)(SWANamedObjectTable* table, const char* name, void* object);

RUNTIME_API struct SWANamedObjectTable* SWAObjectTableCreate();
RUNTIME_API void SWAObjectTableSetDeletor(struct SWANamedObjectTable* table, SWANamedObjectTableDeletor deletor);
RUNTIME_API const char* SWAObjectTableAdd(struct SWANamedObjectTable* table, const char* name, void* object);
RUNTIME_API void SWAObjectTableRemove(struct SWANamedObjectTable* table, const char* name, bool delete_object);
RUNTIME_API void* SWAObjectTableTryFind(struct SWANamedObjectTable* table, const char* name);
RUNTIME_API void SWAObjectTableFree(struct SWANamedObjectTable* table);