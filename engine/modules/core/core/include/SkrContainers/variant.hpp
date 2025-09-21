#pragma once
#include "SkrContainersDef/variant.hpp"

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <class... Ts>
struct Serialize<skr::variant<Ts...>>
{
    inline static void read(ArchiveRead& r, skr::variant<Ts...>& v)
    {
        Archive::ObjectScope _obj_scope{ r };
        GUID type;
        SKR_FAST_CHECK(r.key_value_required<GUID>(u8"type", type), );
        (void)(((_read_by_type_id<Ts>(r, v, type)) != true) && ...);
    }
    inline static void write(ArchiveWrite& w, const skr::variant<Ts...>& v)
    {
        Archive::ObjectScope _obj_scope{ w };

        skr::visit(
            [&](auto&& value) {
                using VisitType = std::remove_cv_t<std::decay_t<decltype(value)>>;
                SKR_FAST_CHECK(w.key_value<GUID>(u8"type", ::skr::type_id_of<VisitType>()), );
                SKR_FAST_CHECK(w.key_value<VisitType>(u8"value", value), );
            },
            v
        );
    }

private:
    template <class T>
    inline static bool _read_by_type_id(ArchiveRead& r, skr::variant<Ts...>& v, GUID type_id)
    {
        using VisitType = std::remove_cv_t<std::decay_t<T>>;

        if (type_id == ::skr::type_id_of<VisitType>())
        {
            T t;
            SKR_FAST_CHECK(r.key_value<VisitType>(u8"value", t), true);
            v = std::move(t);
            return true;
        }
        return false;
    }
};
} // namespace skr