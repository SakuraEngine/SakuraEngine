#pragma once
#include "detail/sptr.hpp"
#include "detail/sweak_ptr.hpp"

#include "type/type.hpp"

namespace skr
{
namespace type
{
// SPtr wrapper
template <class T>
struct type_of<skr::SPtr<T>> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Shared,
            true,
            false,
            type_of<T>::get()
        };
        return &type;
    }
};

// SObjectPtr wrapper
template <class T>
struct type_of<skr::SObjectPtr<T>> {
    static const skr_type_t* get()
    {
        static ReferenceType type{
            ReferenceType::Shared,
            true,
            true,
            type_of<T>::get()
        };
        return &type;
    }
};
} // namespace type
} // namespace skr