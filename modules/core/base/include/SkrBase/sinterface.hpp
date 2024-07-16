#pragma once
#include "SkrBase/types.h"

namespace skr
{

using SInterfaceDeleter = void (*)(struct SInterface*);
struct SInterface {
    virtual ~SInterface() SKR_NOEXCEPT       = default;
    virtual uint32_t          add_refcount() = 0;
    virtual uint32_t          release()      = 0;
    virtual SInterfaceDeleter custom_deleter() const { return nullptr; }
};
template <class T>
constexpr bool is_object_v = std::is_base_of_v<skr::SInterface, T>;

} // namespace skr
