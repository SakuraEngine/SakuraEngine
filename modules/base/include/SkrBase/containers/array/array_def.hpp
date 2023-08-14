#pragma once
#include "SkrBase/config.h"
#include "SkrBase/tools/integer_tools.hpp"

namespace skr
{
// Array 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
template <typename T, typename TS>
struct ArrayDataRef {
    // add/append/emplace: 指向（第一个）添加的元素
    // find: 指向找到的元素
    // remove: nullptr // TODO. check it
    T* data = nullptr;

    // add/append/emplace: （第一个）添加的元素下标
    // find: 找到的元素下标
    // remove: 移除的元素下标
    TS index = npos_of<TS>;

    SKR_INLINE ArrayDataRef()
    {
    }
    SKR_INLINE ArrayDataRef(T* data, TS index)
        : data(data)
        , index(index)
    {
    }
    SKR_INLINE operator bool() const { return data != nullptr || index != npos_of<TS>; }
    SKR_INLINE T& operator*() const { return *data; }
    SKR_INLINE T* operator->() const { return data; }
};
} // namespace skr