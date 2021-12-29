#include "utils.h"
#include <EASTL/string_map.h>

struct WASM3RuntimeFunctionTable : eastl::string_map<IM3Function> {
};

struct WASM3RuntimeFunctionTable* WASM3RuntimeFunctionTableCreate()
{
    return swa_new<WASM3RuntimeFunctionTable>();
}

const char* WASM3RuntimeFunctionTableAdd(struct WASM3RuntimeFunctionTable* table, const char* name, IM3Function function)
{
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        swa_warn("SWA function named %s already exists in table!", name);
        return SWA_NULLPTR;
    }
    return table->insert(name, function).first->first;
}

IM3Function WASM3RuntimeFunctionTableTryFind(struct WASM3RuntimeFunctionTable* table, const char* name)
{
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        return iter->second;
    }
    return SWA_NULLPTR;
}

void WASM3RuntimeFunctionTableFree(struct WASM3RuntimeFunctionTable* table)
{
    swa_delete(table);
}