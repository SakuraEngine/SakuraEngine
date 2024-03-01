#include "utils.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainers/stl_string.hpp"

struct WASM3RuntimeFunctionTable : skr::FlatHashMap<skr::stl_string, IM3Function> {
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
        swa_warn(u8"SWA function named %s already exists in table!", name);
        return SWA_NULLPTR;
    }
    return table->emplace(name, function).first->first.c_str();
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