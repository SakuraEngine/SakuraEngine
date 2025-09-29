#pragma once
#include "SkrRuntime/sugoi/type_registry.hpp"

#include <SkrContainers/hashmap.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrContainers/array.hpp>

namespace sugoi
{
struct TypeRegistry::Impl {
    Impl();
    Impl(const Impl&) = delete;

    type_index_t register_type(const sugoi_type_description_t& desc);
    type_index_t get_type(const guid_t& guid);
    type_index_t get_type(skr::StringView name);
    const sugoi_type_description_t* get_type_desc(sugoi_type_index_t idx);
    void foreach_types(sugoi_type_callback_t callback, void* u);

    guid_t make_guid();

    skr::Vector<type_description_t> descriptions;
    skr::FlatHashMap<skr::String, type_index_t, skr::Hash<skr::String>> name2type;
    skr::FlatHashMap<guid_t, type_index_t, skr::Hash<skr::GUID>> guid2type;
};
} // namespace sugoi