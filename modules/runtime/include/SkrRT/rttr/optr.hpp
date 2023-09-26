#pragma once
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrRT/platform/memory.h"

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

    SKR_INLINE bool operator==(std::nullptr_t) const
    {
        return _ptr == nullptr;
    }

protected:
    SKR_INLINE OPtrBase(void* ptr, size_t head_offset = 0)
        : _ptr(ptr)
        , _head_offset(head_offset)
    {
    }

protected:
    void*  _ptr         = nullptr;
    size_t _head_offset = 0;
};

template <typename T>
struct OPtr final : public OPtrBase {

    SKR_INLINE T& operator*() const
    {
        return *get();
    }
    SKR_INLINE T* operator->() const
    {
        return get();
    }

    SKR_INLINE T* get() const
    {
        return reinterpret_cast<T*>(_ptr);
    }

private:
    friend struct SkrTracedNewObj;
    SKR_INLINE OPtr(T* ptr, size_t head_offset = 0)
        : OPtrBase(ptr, head_offset)
    {
    }
};

struct SkrTracedNewObj {
    const char* file;
    const char* pool_name;

    struct AllocInfo {
        size_t align, size, object_offset;
    };

    template <class T>
    static inline constexpr AllocInfo _get_alloc_info()
    {
        constexpr size_t align         = std::max(alignof(uint32_t), std::max(alignof(T), alignof(OPtrHeader)));
        constexpr size_t total_size    = sizeof(T) + sizeof(OPtrHeader) + sizeof(uint32_t);
        constexpr size_t aligned_size  = (total_size + align - 1) & ~(align - 1);
        constexpr size_t object_offset = aligned_size - sizeof(T);
        return { align, aligned_size, object_offset };
    }

    template <class T, class... TArgs>
    SKR_INLINE OPtr<T> create(TArgs&&... params)
    {
        const std::string_view name = skr::demangle<T>();
        SkrCMessage(name.data(), name.size());

        constexpr AllocInfo alloc_info = _get_alloc_info<T>();
        void*               p_memory   = SkrNewAlignedWithCZone(alloc_info.size, alloc_info.align, file, pool_name);
        SKR_ASSERT(p_memory != nullptr);

        OPtrHeader* p_header = reinterpret_cast<OPtrHeader*>(p_memory);
        void*       p_obj    = reinterpret_cast<uint8_t*>(p_memory) + alloc_info.object_offset;
        uint32_t*   p_offset = reinterpret_cast<uint32_t*>(p_obj) - 1;

        *p_offset = static_cast<uint32_t>(alloc_info.object_offset);
        *p_header = { RTTRTraits<T>::get_type() };
        new (p_obj) DEBUG_NEW_SOURCE_LINE T{ skr::forward<TArgs>(params)... };

        return { p_obj };
    }

    template <class T>
    SKR_INLINE void destroy(OPtr<T> p_obj)
    {
        if (p_obj != nullptr)
        {
            const std::string_view name = skr::demangle<T>();
            SkrCMessage(name.data(), name.size());
            p_obj->~T();
            constexpr AllocInfo alloc_info = _get_alloc_info<T>();
            SkrFreeAlignedWithCZone((void*)p_obj.get_header(), alloc_info.align, file, pool_name);
        }
    }
};

} // namespace skr::rttr

#define SkrNewObj ::skr::rttr::SkrTracedNewObj{ SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)) }.create
#define SkrDeleteObj ::skr::rttr::SkrTracedNewObj{ SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)) }.destroy