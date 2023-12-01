#include "common_utils.h"
#include "wasm/api.h"
#include "SkrRT/containers_new/hashmap.hpp"
#include "SkrRT/containers_new/stl_string.hpp"
#include <time.h>

struct SWANamedObjectTable : skr::flat_hash_map<skr::stl_string, void*> {
    ~SWANamedObjectTable()
    {
        if (deletor)
        {
            SWANamedObjectTable cpy = *this;
            cpy.deletor = nullptr;
            for (auto iter : cpy)
            {
                deletor(this, iter.first.c_str(), iter.second);
            }
        }
    }
    SWANamedObjectTableDeletor deletor = SWA_NULLPTR;
};

struct SWANamedObjectTable* SWAObjectTableCreate()
{
    return swa_new<SWANamedObjectTable>();
}

void SWAObjectTableSetDeletor(struct SWANamedObjectTable* table, SWANamedObjectTableDeletor deletor)
{
    table->deletor = deletor;
}

const char* SWAObjectTableAdd(struct SWANamedObjectTable* table, const char* name, void* object)
{
    skr::stl_string auto_name;
    if (name == SWA_NULLPTR)
    {
        srand((uint32_t)time(NULL));
        auto_name = "object#";
        auto_name.append(std::to_string(rand()));
        name = auto_name.c_str();
    }
    const auto& iter = table->find(name);
    if (iter != table->end())
    {
        swa_warn(u8"SWA object named %s already exists in table!", name);
        return SWA_NULLPTR;
    }
    return table->emplace(name, object).first->first.c_str();
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
            table->deletor(table, iter->first.c_str(), iter->second);
        }
        table->erase(name);
    }
}

void SWAObjectTableFree(struct SWANamedObjectTable* table)
{
    swa_delete(table);
}