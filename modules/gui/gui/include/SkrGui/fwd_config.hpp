#pragma once
//********************************************************
//**                  外部细节隔离头                     **
//********************************************************

// std includes
#include <type_traits>
#include <cinttypes>
#include <cstddef>
#include <limits>

// export macro
#include "SkrGui/module.configure.h"

// skr types
#include "misc/types.h"

// skr containers
#ifdef SKR_GUI_IMPL
    #define CONTAINER_LITE_IMPL
#endif
#include "containers/lite.hpp"

// type system
#include "SkrGui/dev/type_system.hpp"

#define SKR_GUI_RAII_INJECT()                                   \
    template <typename To>                                      \
    auto type_cast() const SKR_NOEXCEPT                         \
    {                                                           \
        return SKR_GUI_CAST<const std::remove_cv_t<To>*>(this); \
    }                                                           \
    template <typename To>                                      \
    auto type_cast() SKR_NOEXCEPT                               \
    {                                                           \
        return SKR_GUI_CAST<To>(this);                          \
    }                                                           \
    template <typename To>                                      \
    bool type_is() const SKR_NOEXCEPT                           \
    {                                                           \
        return SKR_GUI_CAST<To>(this) != nullptr;               \
    }                                                           \
    SKR_GUI_TYPE_ID type_id() const SKR_NOEXCEPT                \
    {                                                           \
        return SKR_GUI_TYPE_ID_OF(this);                        \
    }

namespace skr::gui
{
// Lite container
using skr::lite::HashMapStorage;
using skr::lite::LiteOptional;
using skr::lite::LiteSpan;
using skr::lite::TextStorage;
using skr::lite::VectorStorage;

// TODO. use Size class
using BoxSizeType = skr_float2_t;

} // namespace skr::gui