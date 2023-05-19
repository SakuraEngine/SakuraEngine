#pragma once
#include "misc/function_ref.hpp"
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