#pragma once
#include "SkrRT/type/type.hpp" // IWYU pragma: keep
#include "detail/sptr.hpp" // IWYU pragma: export
#include "detail/sweak_ptr.hpp" // IWYU pragma: keep

namespace skr
{
namespace type
{
// SPtr wrapper
SKR_RUNTIME_API const skr_type_t* make_sptr_type(const skr_type_t* type);
template <class T>
struct type_of<skr::SPtr<T>> {
    static const skr_type_t* get()
    {
        return make_sptr_type(type_of<T>::get());
    }
};

// SObjectPtr wrapper
SKR_RUNTIME_API const skr_type_t* make_sobject_ptr_type(const skr_type_t* type);
template <class T>
struct type_of<skr::SObjectPtr<T>> {
    static const skr_type_t* get()
    {
        return make_sobject_ptr_type(type_of<T>::get());
    }
};
} // namespace type
} // namespace skr