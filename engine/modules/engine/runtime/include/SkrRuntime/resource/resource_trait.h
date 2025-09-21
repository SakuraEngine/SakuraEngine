#pragma once
#include "SkrContainers/function_ref.hpp" // IWYU pragma: keep

namespace skr
{
    /**
    template<>
    struct ResourceTrait<T>
    {
        static void EnumerateDependecies(const T& resource, skr::FunctionRef<void(const Resource&)> callback);
    }
    */
    template<class T>
    struct ResourceTrait;
}