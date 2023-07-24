#include "SkrRT/containers/variant.hpp"
#include "SkrRT/misc/hash.h"
#include "SkrRT/type/type.hpp"
#include "SkrRT/containers/hashmap.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr {
namespace type {
const skr_type_t* make_pointer_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ReferenceType{
        ReferenceType::Observed,
        true,
        false,
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_reference_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ReferenceType{
        ReferenceType::Observed,
        false,
        false,
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_sptr_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ReferenceType{
        ReferenceType::Shared,
        true,
        false,
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_sobject_ptr_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ReferenceType{
        ReferenceType::Shared,
        true,
        true,
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_array_type(const skr_type_t* type, uint64_t num, uint64_t size)
{
    static skr::flat_hash_map<std::pair<const skr_type_t*, size_t>, const skr_type_t*> cache;
    auto it = cache.find({ type, size });
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ArrayType{
        type,
        num,
        size
    };
    cache[{type, size}] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_array_view_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new ArrayViewType{
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
const skr_type_t* make_dynarray_type(const skr_type_t* type)
{
    static skr::flat_hash_map<const skr_type_t*, const skr_type_t*> cache;
    auto it = cache.find(type);
    if (it != cache.end()) {
        return it->second;
    }
    auto ptr_type = new DynArrayType{
        type
    };
    cache[type] = ptr_type;
    return ptr_type;
}
void* DynArrayType::Get(void* addr, uint64_t index) const
{
    auto ptr = ((DynArrayStorage*)addr)->begin;
    return ptr + index * elementType->Size();
}
uint64_t DynArrayType::Num(void* addr) const
{
    auto& storage = *(DynArrayStorage*)addr;
    return (storage.end - storage.begin) / elementType->Size();
}
void DynArrayType::Reset(void* addr, uint64_t size) const
{
    auto& storage = *(DynArrayStorage*)addr;
    uint64_t capacity = (storage.capacity - storage.end) / elementType->Size();
    uint64_t old_size = Num(addr);
    for(int i = 0; i < old_size; i++) {
        elementType->Destruct(storage.begin + i * elementType->Size());
    }
    if (capacity < size) {
        auto new_capacity = std::max(capacity * 2, size);
        auto new_begin = (uint8_t*)sakura_malloc_aligned(new_capacity * elementType->Size(), elementType->Align());
        if(storage.begin != nullptr)
            sakura_free_aligned(storage.begin, elementType->Align());
        storage.begin = new_begin;
        storage.capacity = new_begin + new_capacity * elementType->Size();
    }
    storage.end = storage.begin + size * elementType->Size();
}
void DynArrayType::Grow(void* data, uint64_t size) const
{
    auto& storage = *(DynArrayStorage*)data;
    uint64_t capacity = (storage.capacity - storage.end) / elementType->Size();
    uint64_t old_size = Num(data);
    if (capacity < size) {
        auto new_capacity = std::max(capacity * 2, size);
        auto new_begin = (uint8_t*)sakura_malloc_aligned(new_capacity * elementType->Size(), elementType->Align());
        for(int i = 0; i < old_size; i++) {
            elementType->Move(new_begin + i * elementType->Size(), storage.begin + i * elementType->Size());
            //elementType->Destruct(storage.begin + i * elementType->Size());
        }
        if(storage.begin != nullptr)
            sakura_free_aligned(storage.begin, elementType->Align());
        storage.begin = new_begin;
        storage.capacity = new_begin + new_capacity * elementType->Size();
        storage.end = storage.begin + old_size * elementType->Size();
    }
}
void DynArrayType::Resize(void *data, uint64_t size) const
{
    Grow(data, size);
    auto& storage = *(DynArrayStorage*)data;
    storage.end = storage.begin + size * elementType->Size();
}
void DynArrayType::Reserve(void *data, uint64_t size) const
{
    Grow(data, size);
}
void* DynArrayType::Insert(void* addr, uint64_t index) const
{
    auto& storage = *(DynArrayStorage*)addr;
    uint64_t capacity = (storage.capacity - storage.end) / elementType->Size();
    uint64_t old_size = Num(addr);
    SKR_ASSERT(old_size >= index);
    if(index == old_size) {
        Resize(addr, old_size + 1);
        storage.end += elementType->Size();
        return storage.begin + index * elementType->Size();
    }
    else {
        if(old_size+1 > capacity)
        {
            auto new_capacity = std::max(capacity * 2, old_size+1);
            auto new_begin = (uint8_t*)sakura_malloc_aligned(new_capacity * elementType->Size(), elementType->Align());
            for(int i = 0; i < index; i++) {
                elementType->Move(new_begin + i * elementType->Size(), storage.begin + i * elementType->Size());
            }
            for(int64_t i = index + 1; i <= (int64_t)old_size; i++) {
                elementType->Move(new_begin + i * elementType->Size(), storage.begin + (i - 1) * elementType->Size());
            }
            if(storage.begin != nullptr)
                sakura_free_aligned(storage.begin, elementType->Align());
            storage.begin = new_begin;
            storage.capacity = new_begin + new_capacity * elementType->Size();
            storage.end = storage.begin + (old_size + 1) * elementType->Size();
            return storage.begin + index * elementType->Size();
        }
        else
        {
            for(int64_t i = old_size - 1; i >= (int64_t)index; i--) {
                if(i != old_size - 1)
                    elementType->Destruct(storage.begin + (i + 1) * elementType->Size());
                elementType->Move(storage.begin + (i + 1) * elementType->Size(), storage.begin + i * elementType->Size());
            }
            
            storage.end += elementType->Size();
            return storage.begin + index * elementType->Size();
        }
    }
}
void DynArrayType::Erase(void* addr, uint64_t index, bool bKeepOrder) const
{
    auto& storage = *(DynArrayStorage*)addr;
    uint64_t old_size = Num(addr);
    SKR_ASSERT(old_size > index);
    if(bKeepOrder) {
        for(int64_t i = index + 1; i < (int64_t)old_size; i++) {
            elementType->Destruct(storage.begin + (i - 1) * elementType->Size());
            elementType->Move(storage.begin + (i - 1) * elementType->Size(), storage.begin + i * elementType->Size());
        }
        //elementType->Destruct(storage.begin + (old_size - 1) * elementType->Size());
    }
    else {
        elementType->Destruct(storage.begin + index * elementType->Size());
        elementType->Move(storage.begin + index * elementType->Size(), storage.end - elementType->Size());
        //elementType->Destruct(storage.end - elementType->Size());
    }
    storage.end -= elementType->Size();
}


const skr_type_t* make_variant_type(const skr::span<const skr_type_t*> types)
{
    static skr::flat_hash_map<skr::string, const skr_type_t*, skr::hash<skr::string>> cache;
    skr::string name;
    for (auto type : types) {
        name += type->Name();
    }
    auto it = cache.find(name);
    if (it != cache.end()) {
        return it->second;
    }
    size_t size = 0;
    size_t align = alignof(size_t);
    for(auto type : types) {
        size = std::max((uint64_t)size, type->Size());
        align = std::max((uint64_t)align, type->Align());
    }
    uint64_t padding = ((sizeof(size_t) + align - 1) & ~(align - 1)) - sizeof(size_t);
    size += sizeof(size_t) + padding;
    auto ptr_type = new VariantType{
        types,
        size,
        align,
        padding
    };
    cache[name] = ptr_type;
    return ptr_type;
}
void VariantType::Set(void* dst, size_t index, const void* src) const
{
    SKR_ASSERT(index < types.size());
    auto storage = (VariantStorage*)dst;
    auto oldType = types[storage->index];
    auto newType = types[index];
    auto data = storage->data + padding;
    oldType->Destruct(data);
    if(src)
        newType->Copy(data, src);
    else
        newType->Construct(data, nullptr, 0);
}
void* VariantType::Get(void* dst, size_t index) const
{
    SKR_ASSERT(index < types.size());
    auto storage = (VariantStorage*)dst;
    SKR_ASSERT(storage->index == index);
    return storage->data + padding;
}
size_t VariantType::Index(void* dst) const
{
    auto storage = (VariantStorage*)dst;
    return storage->index;
}
}
}