#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
// TODO. 实现
//  delegate 不提供 SSO 的实现，Copy/Move Functor 的行为是直接为 DelegateCore 添加引用计数，如果需要显式的拷贝行为，需要主动调用 copy 函数，
//  这是为了模拟高级语言的 delegate 行为
template <typename Allocator>
struct Delegate {
    enum class Type
    {
        None,    // unbind
        Static,  // function pointer
        Method,  // delegate core
        Functor, // delegate core
    };

private:
    Type type;
};

} // namespace skr::container