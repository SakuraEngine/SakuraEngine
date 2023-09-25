#pragma once
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
struct OPtrHeader {
    Type* type = nullptr;
};

struct OPtrBase {
    SKR_INLINE OPtrHeader* get_header() const
    {
        if (_ptr)
        {
            uint8_t* p_head        = reinterpret_cast<uint8_t*>(_ptr) - _head_offset;
            uint32_t header_offset = *(reinterpret_cast<uint32_t*>(p_head) - 1);
            uint8_t* p_header      = p_head - header_offset;
            return reinterpret_cast<OPtrHeader*>(p_header);
        }
        return nullptr;
    }

    SKR_INLINE Type* get_type() const
    {
        if (_ptr)
        {
            OPtrHeader* header = get_header();
            return header->type;
        }
        return nullptr;
    }

    SKR_INLINE bool type_equal(const Type* type) const
    {
        if (_ptr)
        {
            OPtrHeader* header = get_header();
            return header->type == type;
        }
        return false;
    }
    template <typename T>
    SKR_INLINE bool type_equal() const
    {
        return type_equal(RTTRTraits<T>::get_type());
    }

protected:
    void*  _ptr         = nullptr;
    size_t _head_offset = 0;
};

template <typename T>
struct OPtr : public OPtrBase {
};

} // namespace skr::rttr