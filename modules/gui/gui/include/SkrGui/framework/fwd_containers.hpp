#pragma once
#include <type_traits>
#ifdef SKR_GUI_IMPL
#include <containers/vector.hpp>
#include <containers/hashmap.hpp>
#include <new> // placement new operator
#endif

#include "utils/types.h"
#include "SkrGui/module.configure.h"

namespace skr {
namespace gui {

template<uint32_t _size, uint32_t _align>
struct AlignedStorage
{
    alignas(_align) uint8_t storage[_size];
};

using VectorStorageBase = AlignedStorage<24, 8>;
template<typename T> 
struct VectorStorage : public VectorStorageBase
{
#ifdef SKR_GUI_IMPL
    using type = skr::vector<T>;
    type& get() 
    {
        return *std::launder(reinterpret_cast<skr::vector<T>*>(this));
    }
    VectorStorage() { ctor(); }
    ~VectorStorage() { dtor(); }
private:
    void ctor() { new (&get()) type(); }
    void dtor() { get().~type(); }
    static_assert(sizeof(AlignedStorage<24, 8>) == sizeof(type), "Vector storage size mismatch!");
    static_assert(alignof(AlignedStorage<24, 8>) == alignof(type), "Vector storage alignment mismatch!"); 
#endif
};

using HashMapStorageBase = AlignedStorage<48, 8>;
template<typename K, typename V> 
struct HashMapStorage : public HashMapStorageBase
{
#ifdef SKR_GUI_IMPL
    using type = skr::flat_hash_map<K, V>;
    type& get() 
    {
        return *std::launder(reinterpret_cast<type*>(this));
    }
    HashMapStorage() { ctor(); }
    ~HashMapStorage() { dtor(); }
private:
    void ctor() { new (&get()) type(); }
    void dtor() { get().~type(); }
    static_assert(sizeof(HashMapStorageBase) == sizeof(type), "HashMap storage size mismatch!");
    static_assert(alignof(HashMapStorageBase) == alignof(type), "HashMap storage alignment mismatch!"); 
#endif
};

} }