#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"

namespace skr::container
{
// Array 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
template <typename T, typename TS>
struct ArrayDataRef {
    // ctor
    SKR_INLINE ArrayDataRef()
    {
    }
    SKR_INLINE ArrayDataRef(T* data, TS index)
        : _data(data)
        , _index(index)
    {
    }
    template <typename U>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>)
    SKR_INLINE ArrayDataRef(const ArrayDataRef<U, TS>& rhs)
        : _data(const_cast<T*>(rhs.ptr()))
        , _index(rhs.index())
    {
    }

    // getter & validator
    SKR_INLINE T*   ptr() const { return _data; }
    SKR_INLINE TS   index() const { return _index; }
    SKR_INLINE T&   ref() const { return *_data; }
    SKR_INLINE bool is_valid() const { return _data != nullptr && _index != npos_of<TS>; }

    // operators
    SKR_INLINE operator bool() const { return is_valid(); }
    // SKR_INLINE T& operator*() const { return ref(); }
    // SKR_INLINE T* operator->() const { return ptr(); }

private:
    // add/append/emplace: 指向（第一个）添加的元素
    // find: 指向找到的元素
    T* _data = nullptr;

    // add/append/emplace: （第一个）添加的元素下标
    // find: 找到的元素下标
    TS _index = npos_of<TS>;
};
} // namespace skr::container