#pragma once
//********************************************************
//**                  外部细节隔离头                     **
//********************************************************

// std includes
#include <type_traits>
#include <cinttypes>
#include <cstddef>
#include <limits>

// reflection
#include "SkrRT/config.h"
#include "SkrRT/rttr/iobject.hpp"

// export macro
#include "SkrBase/config.h"

// skr types
#include "SkrRT/misc/types.h"

// skr containers
#include "SkrContainers/vector.hpp"
#include "SkrContainers/map.hpp"
#include "SkrContainers/set.hpp"
#include "SkrContainers/optional.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrContainers/stl_function.hpp"

// function ref
#include "SkrContainers/function_ref.hpp"

// not_null
#include "SkrContainers/not_null.hpp"

// assert
#define SKR_GUI_ASSERT(__EXPR) SKR_ASSERT(__EXPR)

// log
#include "SkrCore/log.hpp"
#define SKR_GUI_LOG_ERROR(...) SKR_LOG_ERROR(__VA_ARGS__)
#define SKR_GUI_LOG_WARN(...) SKR_LOG_WARN(__VA_ARGS__)
#define SKR_GUI_LOG_INFO(...) SKR_LOG_INFO(__VA_ARGS__)
#define SKR_GUI_LOG_DEBUG(...) SKR_LOG_DEBUG(__VA_ARGS__)

// memory
#include "SkrMemory/memory.h"
#define SKR_GUI_NEW SkrNew
#define SKR_GUI_DELETE SkrDelete

namespace skr::gui
{
// !!! 生命周期无法保证，仅用于参数或局部使用
template <typename F>
using FunctionRef = ::skr::FunctionRef<F>;

template <typename F>
using Function = ::skr::stl_function<F>;

// smart ptr
template <typename T>
using SPtr = ::skr::SPtr<T>;

// not null
template <typename T>
using NotNull = ::skr::not_null<T>;

// Lite container
template <typename T>
using Optional = skr::Optional<T>;
template <typename T>
using Span = skr::span<T>;

// containers
using String     = skr::String;
using StringView = skr::StringView;
template <typename T>
using Array = skr::Vector<T>;
template <typename K, typename V>
using Map = skr::UMap<K, V>;
template <typename T>
using Set = skr::USet<T>;

} // namespace skr::gui