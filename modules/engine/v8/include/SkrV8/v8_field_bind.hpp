#pragma once

namespace skr::v8
{
// TODO. 负责 field 的导入导出，由于 field 的导入导出方式可以被特化，所以这里也需要一个 flag 参数，主要用于制造 getter/setter
template <typename T>
struct V8ParamBind {
};
} // namespace skr::v8