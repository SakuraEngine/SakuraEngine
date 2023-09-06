#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"
#include "SkrRT/containers_new/array.hpp"
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
struct ArrayType : public GenericType {
    ArrayType(Type* target_type, Span<size_t> dimensions)
        : _target_type(target_type)
        , _size(1)
        , _dimensions(dimensions.data(), dimensions.size())
    {
        _generic_guid = kArrayGenericGUID;
        for (auto d : dimensions)
        {
            _size *= d;
        }
    }

    bool call_ctor(void* ptr) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_dtor(void* ptr) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_copy(void* dst, const void* src) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_move(void* dst, void* src) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_assign(void* dst, const void* src) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_move_assign(void* dst, void* src) const override
    {
        // TODO. impl it
        return true;
    }
    bool call_hash(const void* ptr, size_t& result) const override
    {
        // TODO. impl it
        return true;
    }

private:
    Type*         _target_type;
    size_t        _size;
    Array<size_t> _dimensions;
};
} // namespace skr::rttr