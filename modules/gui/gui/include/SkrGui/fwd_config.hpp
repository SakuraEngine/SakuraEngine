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
#include "containers/lite.hpp"
#include "containers/vector.hpp"
#include "containers/string.hpp"
#include "containers/hashmap.hpp"

// type system
#include "SkrGui/dev/type_system.hpp"

// function ref
#include "containers/function_ref.hpp"

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

// memory
#include "platform/memory.h"
#define SKR_GUI_NEW SkrNew
#define SKR_GUI_DELETE SkrDelete

namespace skr::gui
{
// function
template <typename F>
using Callback = ::skr::function_ref<F>;

// not null
template <typename T>
using NotNull = ::skr::not_null<T>;

// Lite container
template <typename T>
using Optional = skr::lite::LiteOptional<T>;
template <typename T>
using Span = skr::lite::LiteSpan<T>;

// containers
template <typename T>
using Array = skr::vector<T>;
using String = skr::string;
template <typename K, typename V>
using HashMap = skr::flat_hash_map<K, V>;

} // namespace skr::gui