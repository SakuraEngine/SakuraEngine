#include "common_utils.h"
#include "wasm/api.h"
#include <EASTL/string_map.h>

struct SWANamedObjectTable : eastl::string_map<void*> {
    ~SWANamedObjectTable()
    {
        if (deletor)
        {
            for (const auto& iter : *this)
            {
                deletor(this, iter.first, iter.second);
            }
        }
    }
    SWANamedObjectTableDeletor deletor = SWA_NULLPTR;
};

struct SWANamedObjectTable* SWAObjectTableCreate()
{
    return swa_new<SWANamedObjectTable>();
}

RUNTIME_API void SWAObjectTableSetDeletor(struct SWANamedObjectTable* table, SWANamedObjectTableDeletor deletor)
{
    table->deletor = deletor;
}

const char* SWAObjectTableAdd(struct SWANamedObjectTable* table, const char* name, void* object)
{
    eastl::string auto_name;
    if (name == SWA_NULLPTR)
    {
        auto_name = "object#";
        auto_name.append(eastl::to_string(table->size()));
        name = auto_name.c_str();
    }
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        swa_warn("SWA object named %s already exists in table!", name);
        return SWA_NULLPTR;
    }
    return table->insert(name, object).first->first;
}

void* SWAObjectTableTryFind(struct SWANamedObjectTable* table, const char* name)
{
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        return iter->second;
    }
    return SWA_NULLPTR;
}

void SWAObjectTableRemove(struct SWANamedObjectTable* table, const char* name, bool delete_object)
{
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        if (delete_object)
        {
            table->deletor(table, iter->first, iter->second);
        }
        table->erase(name);
    }
}

void SWAObjectTableFree(struct SWANamedObjectTable* table)
{
    swa_delete(table);
}