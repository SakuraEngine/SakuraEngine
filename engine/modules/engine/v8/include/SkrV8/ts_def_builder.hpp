#pragma once
#include <SkrContainers/string.hpp>
#include <SkrV8/v8_fwd.hpp>

namespace skr
{
struct SKR_V8_API TSDefBuilder
{
    // codegen tools
    inline void $line()
    {
        _result.append(u8'\n');
    }
    template <typename... Args>
    inline void $line(StringView fmt, Args&&... args)
    {
        _result.add(u8' ', _cur_indent * indent_size);
        format_to(_result, fmt, std::forward<Args>(args)...);
        _result.append(u8'\n');
    }
    template <typename Func>
    inline void $indent(Func&& func)
    {
        ++_cur_indent;
        func();
        --_cur_indent;
    }

    // generate
    String generate_global();
    String generate_module(StringView module_name);

    // helper
    String type_name_with_ns(const V8BindTemplate* bind_tp) const;
    String params_signature(const Vector<V8BTDataParam>& params) const;
    String return_signature(const V8BTDataFunctionBase& func_data) const;

private:
    // helpers
    void _gen();
    void _gen_ns_node(const V8VirtualModuleNode& node);

public:
    // codegen config
    uint32_t indent_size = 2;
    const V8VirtualModule* virtual_module = nullptr;

private:
    uint32_t _cur_indent = 0;
    String _result;
};
} // namespace skr
