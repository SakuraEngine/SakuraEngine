#pragma once
#include "SkrBase/config.h"
#include "SkrBase/tools/integer_tools.hpp"

namespace skr
{
template <typename K, typename V, typename TS>
struct SparseHashMapDataRef {
    // add/emplace: 添加的元素 key 指针
    // find: 找到的元素 key 指针
    // remove: nullptr // TODO. check it
    K* key = nullptr;

    // add/emplace: 添加的元素 value 指针
    // find: 找到的元素 value 指针
    // remove: nullptr // TODO. check it
    V* value = nullptr;

    // add/emplace: 添加的元素下标
    // find: 找到的元素下标
    // remove: 移除的元素下标
    TS index = npos_of<TS>;

    // add/emplace: 元素是否已经存在
    // find: false
    // remove: false
    bool already_exist = false;

    SKR_INLINE SparseHashMapDataRef()
    {
    }
    SKR_INLINE SparseHashMapDataRef(K* key, V* value, TS index, bool already_exist = false)
        : key(key)
        , value(value)
        , index(index)
        , already_exist(already_exist)
    {
    }
    SKR_INLINE operator bool() { return key != nullptr || index != npos_of<TS>; }
};

} // namespace skr