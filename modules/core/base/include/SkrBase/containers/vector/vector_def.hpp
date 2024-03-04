#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"

namespace skr::container
{
// Vector 的数据引用，代替单纯的指针/Index返回
// 提供足够的信息，并将 npos 封装起来简化调用防止出错
template <typename T, typename TS, bool kConst>
struct VectorDataRef {
    using DataType = std::conditional_t<kConst, const T, T>;
    using SizeType = TS;

    // ctor
    SKR_INLINE
    VectorDataRef() = default;
    SKR_INLINE VectorDataRef(DataType* ptr, SizeType index)
        : _ptr(ptr)
        , _index(index)
    {
    }
    template <bool kConstRHS>
    SKR_INLINE VectorDataRef(const VectorDataRef<T, SizeType, kConstRHS>& rhs)
        : _ptr(const_cast<DataType*>(rhs.ptr()))
        , _index(rhs.index())
    {
    }

    // getter & validator
    SKR_INLINE DataType* ptr() const { return _ptr; }
    SKR_INLINE DataType& ref() const { return *_ptr; }
    SKR_INLINE SizeType  index() const { return _index; }
    SKR_INLINE bool      is_valid() const { return _ptr != nullptr && _index != npos_of<SizeType>; }

    // operators
    SKR_INLINE explicit operator bool() const { return is_valid(); }
    // SKR_INLINE DataType& operator*() const { return ref(); }
    // SKR_INLINE DataType* operator->() const { return ptr(); }

    // compare
    SKR_INLINE bool operator==(const VectorDataRef& rhs) const { return _ptr == rhs._ptr; }
    SKR_INLINE bool operator!=(const VectorDataRef& rhs) const { return _ptr != rhs._ptr; }

private:
    // add/append/emplace: 指向（第一个）添加的元素
    // find: 指向找到的元素
    DataType* _ptr = nullptr;

    // add/append/emplace: （第一个）添加的元素下标
    // find: 找到的元素下标
    SizeType _index = npos_of<SizeType>;
};
} // namespace skr::container