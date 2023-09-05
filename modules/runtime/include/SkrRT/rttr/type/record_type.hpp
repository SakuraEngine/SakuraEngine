#pragma once
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/containers_new/umap.hpp"

namespace skr::rttr
{
struct RecordType : public Type {
    // get methods
    // get fields

private:
    UMap<GUID, Type*> _base_types_map;
};
} // namespace skr::rttr