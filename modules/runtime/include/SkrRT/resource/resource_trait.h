#pragma once
#include "SkrRT/containers/function_ref.hpp" // IWYU pragma: keep

namespace skr::resource
{
    /**
    template<>
    struct ResourceTrait<T>
    {
        static void EnumerateDependecies(const T& resource, skr::function_ref<void(const Resource&)> callback);
    }
    */
    template<class T>
    struct ResourceTrait;
}