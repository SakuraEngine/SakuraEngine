#pragma once
#include "types/md5.h"
#include "types/guid.h"
#include "types/vec.h"

#ifdef __cplusplus
#include "types/binary_serde.hpp"
#include "types/json_serde.hpp"

namespace skr {

using SInterfaceDeleter = void (*)(struct SInterface*);
struct SInterface {
    virtual ~SInterface() SKR_NOEXCEPT       = default;
    virtual uint32_t          add_refcount() = 0;
    virtual uint32_t          release()      = 0;
    virtual skr_guid_t        get_type() { return {}; }
    virtual SInterfaceDeleter custom_deleter() const { return nullptr; }
};
template <class T>
constexpr bool is_object_v = std::is_base_of_v<skr::SInterface, T>;

} // namespace skr

#ifndef sobject_cast
    #define sobject_cast static_cast
#endif

#endif