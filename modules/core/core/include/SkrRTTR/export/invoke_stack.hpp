#pragma once

namespace skr::rttr
{
// TODO. 模拟的调用栈, 内部结构为 buffer 链表, 主要是为了应对 align 问题以及某些将亡值 (xvalue) 需要析构的情形，这在脚本导出中很有用
// TODO. 模拟调用栈涉及向目标值进行转换的问题, 这需要有一个地方注册转换函数
template <typename Buffer>
struct InvokeStack {
};
} // namespace skr::rttr