#pragma once
#include "SkrContainersDef/optional.hpp"
#include "SkrBase/types/guid.h"

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
static constexpr GUID kOptionalGenericId = u8"bc48634e-85dd-45ce-a7c7-f85d5bab9680"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::Optional<T>>
{
    inline static constexpr bool is_supported = concepts::WithRTTRTraits<T>;
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kOptionalGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename T>
struct Serialize<Optional<T>>
{
    inline static void read(ArchiveRead& r, Optional<T>& v)
    {
        Archive::ObjectScope _obj_scope{ r };

        // read has_value
        bool has_value = false;
        SKR_FAST_CHECK(r.key_value_required<bool>(u8"has_value", has_value), );

        // read value
        if (has_value)
        {
            T temp;
            SKR_FAST_CHECK(r.key_value_required<T>(u8"value", temp), );
            v = Optional<T>(std::move(temp));
        }
        else
        {
            v = Optional<T>();
        }
    }
    inline static void write(ArchiveWrite& w, const Optional<T>& v)
    {
        Archive::ObjectScope _obj_scope{ w };

        // write has value
        SKR_FAST_CHECK(w.key_value<bool>(u8"has_value", v.has_value()), );

        // write value
        if (v.has_value())
        {
            SKR_FAST_CHECK(w.key_value<T>(u8"value", v.value()), );
        }
    }
};
} // namespace skr