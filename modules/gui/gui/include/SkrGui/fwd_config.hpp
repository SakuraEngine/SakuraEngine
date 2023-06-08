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

// function ref
#include "misc/function_ref.hpp"

// not_null
#include "containers/not_null.hpp"

#define SKR_GUI_RAII_MIX_IN()                                   \
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

// assert
#define SKR_GUI_ASSERT(__EXPR) SKR_ASSERT(__EXPR)

// log
#include "misc/log.hpp"
#define SKR_GUI_LOG_ERROR(...) SKR_LOG_ERROR(__VA_ARGS__)
#define SKR_GUI_LOG_WARN(...) SKR_LOG_WARN(__VA_ARGS__)
#define SKR_GUI_LOG_INFO(...) SKR_LOG_INFO(__VA_ARGS__)
#define SKR_GUI_LOG_DEBUG(...) SKR_LOG_DEBUG(__VA_ARGS__)

namespace skr::gui
{
// function
template <typename F>
using Callback = ::skr::function_ref<F>;

// not null
template <typename T>
using NotNull = ::skr::not_null<T>;

// Lite container
using skr::lite::HashMapStorage;
using skr::lite::LiteOptional;
using skr::lite::LiteSpan;
using skr::lite::TextStorage;
using skr::lite::VectorStorage;

// TODO. use Size class
using BoxSizeType = skr_float2_t;

} // namespace skr::gui