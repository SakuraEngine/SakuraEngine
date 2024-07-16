#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
// TODO. 实现
//  function 的实现与 std::function 一致，提供 SSO 的选项，copy/move 遵循 Functor 行为，主要是为了某些确实需要 SSO 的场景（比如多线程任务包）
template <typename Allocator>
struct Function {
};
} // namespace skr::container