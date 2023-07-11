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
#include "SkrRT/misc/types.h"

// skr containers
#include "SkrRT/containers/lite.hpp"
#include "SkrRT/containers/span.hpp"
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/hashmap.hpp"
#include "SkrRT/containers/sptr.hpp"

// type system
#include "SkrGui/dev/type_system.hpp"

// function ref
#include "SkrRT/containers/function_ref.hpp"

// not_null
#include "SkrRT/containers/not_null.hpp"

#define SKR_GUI_RAII_MIX_IN()                                       \
    template <typename To>                                          \
    auto type_cast() const SKR_NOEXCEPT                             \
    {                                                               \
        return SKR_GUI_CAST<const std::remove_cv_t<To>>(this);      \
    }                                                               \
    template <typename To>                                          \
    auto type_cast() SKR_NOEXCEPT                                   \
    {                                                               \
        return SKR_GUI_CAST<To>(this);                              \
    }                                                               \
    template <typename To>                                          \
    auto type_cast_fast() SKR_NOEXCEPT                              \
    {                                                               \
        return SKR_GUI_CAST_FAST<To>(this);                         \
    }                                                               \
    template <typename To>                                          \
    auto type_cast_fast() const SKR_NOEXCEPT                        \
    {                                                               \
        return SKR_GUI_CAST_FAST<const std::remove_cv_t<To>>(this); \
    }                                                               \
    template <typename To>                                          \
    bool type_is() const SKR_NOEXCEPT                               \
    {                                                               \
        return SKR_GUI_CAST<To>(this) != nullptr;                   \
    }                                                               \
    bool type_based_on(SKR_GUI_TYPE_ID id) const SKR_NOEXCEPT       \
    {                                                               \
        return SKR_GUI_BASED_ON(this, id);                          \
    }                                                               \
    SKR_GUI_TYPE_ID type_id() const SKR_NOEXCEPT                    \
    {                                                               \
        return SKR_GUI_TYPE_ID_OF(this);                            \
    }

// assert
#define SKR_GUI_ASSERT(__EXPR) SKR_ASSERT(__EXPR)

// log
#include "SkrRT/misc/log.hpp"
#define SKR_GUI_LOG_ERROR(...) SKR_LOG_ERROR(__VA_ARGS__)
#define SKR_GUI_LOG_WARN(...) SKR_LOG_WARN(__VA_ARGS__)
#define SKR_GUI_LOG_INFO(...) SKR_LOG_INFO(__VA_ARGS__)
#define SKR_GUI_LOG_DEBUG(...) SKR_LOG_DEBUG(__VA_ARGS__)

// memory
#include "SkrRT/platform/memory.h"
#define SKR_GUI_NEW SkrNew
#define SKR_GUI_DELETE SkrDelete

namespace skr::gui
{
// !!! 生命周期无法保证，仅用于参数或局部使用
template <typename F>
using FunctionRef = ::skr::function_ref<F>;

template <typename F>
using Function = ::eastl::function<F>;

// smart ptr
template <typename T>
using SPtr = ::skr::SPtr<T>;

// not null
template <typename T>
using NotNull = ::skr::not_null<T>;
using ::skr::make_not_null;

// Lite container
template <typename T>
using Optional = skr::lite::LiteOptional<T>;
template <typename T>
using Span = skr::lite::LiteSpan<T>;

// containers
using String     = skr::string;
using StringView = skr::string_view;
template <typename T>
using Array = skr::vector<T>;
template <typename K, typename V>
using Map = skr::flat_hash_map<K, V>;
template <typename T>
using Set = skr::flat_hash_set<T>;

} // namespace skr::gui