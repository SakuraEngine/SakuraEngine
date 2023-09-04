#include "SkrRT/rttr/type_loader.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrBase/misc/hash.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct PrimitiveType final : public Type {
    PrimitiveType()
    {
        _type_category = ETypeCategory::SKR_TYPE_CATEGORY_PRIMITIVE;
        _alignment     = alignof(T);
        _size          = sizeof(T);
        _type_id       = RTTRTraits<T>::get_guid();
    }

    bool call_ctor(void* ptr) const override { return true; }
    bool call_dtor(void* ptr) const override { return true; }
    bool call_copy(void* dst, const void* src) const override
    {
        new (dst) T(*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move(void* dst, void* src) const override
    {
        new (dst) T(std::move(*reinterpret_cast<T*>(src)));
        return true;
    }
    bool call_assign(void* dst, const void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = (*reinterpret_cast<const T*>(src));
        return true;
    }
    bool call_move_assign(void* dst, void* src) const override
    {
        (*reinterpret_cast<T*>(dst)) = std::move(*reinterpret_cast<T*>(src));
        return true;
    }
    bool call_hash(const void* ptr, size_t& result) const override
    {
        result = Hash<T>{}(*reinterpret_cast<const T*>(ptr));
        return true;
    }
};

template <typename T>
struct PrimitiveTypeLoader final : public TypeLoader {
    Type* load() override
    {
        Type* type = SkrNew<PrimitiveType<T>>();
        return type;
    }
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};

// TODO. register types

} // namespace skr::rttr