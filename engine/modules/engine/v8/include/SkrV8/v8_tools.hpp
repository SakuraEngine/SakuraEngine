#pragma once
#include <SkrContainers/vector.hpp>

namespace skr
{
struct V8ErrorCache
{
    enum class EKind
    {
        Error,
        Warning,
    };

    struct Data
    {
        String content;
        EKind kind;
    };

    template <typename... Args>
    inline void error(StringView fmt, Args&&... args)
    {
        auto& data = _data.add_default().ref();
        format_to(data.content, fmt, std::forward<Args>(args)...);
        data.kind = EKind::Error;
        ++_error_count;
    }
    template <typename... Args>
    inline void warning(StringView fmt, Args&&... args)
    {
        auto& data = _data.add_default().ref();
        format_to(data.content, fmt, std::forward<Args>(args)...);
        data.kind = EKind::Warning;
        ++_warning_count;
    }

    inline bool has_any_msg() const { return _error_count > 0 || _warning_count > 0; }
    inline bool has_error() const { return _error_count > 0; }
    inline bool has_warning() const { return _warning_count > 0; }
    inline const Vector<Data>& data() const { return _data; }

private:
    Vector<Data> _data = {};
    uint32_t _error_count = 0;
    uint32_t _warning_count = 0;
};

struct V8ErrorBuilderTreeStyle
{
    enum class EIndentNode : uint8_t
    {
        Empty,      // means last node in this indent level
        IndentHint, // means not last node in this indent level
        NodeEntry,  // means entry a sub node
        NodeExit,   // mean exit a sub node
    };
    struct LineData
    {
        String content = {};
        uint32_t indent = 0;
        InlineVector<EIndentNode, 16> indent_nodes = {};
    };

    // combine content
    template <typename... Args>
    inline V8ErrorBuilderTreeStyle write(StringView fmt, Args&&... args)
    {
        // next line
        if (_is_line_start)
        {
            auto& line_data = _lines.add_default().ref();
            line_data.indent = _cur_indent;
            _is_line_start = false;
        }

        // append content
        auto& line_data = _lines.back();
        format_to(line_data.content, fmt, std::forward<Args>(args)...);

        return *this;
    }
    template <typename... Args>
    inline V8ErrorBuilderTreeStyle write_line(StringView fmt, Args&&... args)
    {
        write(fmt, std::forward<Args>(args)...);
        endline();
        return *this;
    }
    inline V8ErrorBuilderTreeStyle write_line()
    {
        write(u8"");
        endline();
        return *this;
    }
    inline V8ErrorBuilderTreeStyle& endline()
    {
        _is_line_start = true;
        return *this;
    }
    template <typename F>
    inline V8ErrorBuilderTreeStyle& indent(F&& f)
    {
        ++_cur_indent;
        f();
        --_cur_indent;
        return *this;
    }

    // build content
    inline void solve_indent()
    {
        {
            // get max indent and resize indent nodes
            uint32_t max_indent = 0;
            for (auto& line_data : _lines)
            {
                max_indent = std::max(max_indent, line_data.indent);
                line_data.indent_nodes.resize(line_data.indent + 1, EIndentNode::Empty);
            }

            // last enter cache for detect last node
            InlineVector<uint32_t, 16> last_enter_line(max_indent + 1, npos_of<uint32_t>);

            // process first line
            uint32_t cur_indent;
            {
                auto& line_data = _lines[0];
                cur_indent = line_data.indent;

                // record entry node
                for (uint32_t i = 0; i <= cur_indent; ++i)
                {
                    last_enter_line[i] = 0;
                    line_data.indent_nodes[i] = EIndentNode::NodeEntry;
                }
            }

            // process other lines
            for (uint64_t line_idx = 1; line_idx < _lines.size(); ++line_idx)
            {
                auto& line_data = _lines[line_idx];

                if (line_data.indent > cur_indent)
                {
                    // add indent hint
                    for (uint32_t i = 0; i <= line_data.indent; ++i)
                    {
                        line_data.indent_nodes[i] = EIndentNode::IndentHint;
                    }

                    // setup new indent entry node
                    for (uint32_t i = cur_indent + 1; i <= line_data.indent; ++i)
                    {
                        last_enter_line[i] = line_idx;
                        line_data.indent_nodes[i] = EIndentNode::NodeEntry;
                    }

                    // update cur indent
                    cur_indent = line_data.indent;
                }
                else if (line_data.indent < cur_indent)
                {
                    // take back overflow indent hint
                    for (uint32_t indent_idx = line_data.indent + 1; indent_idx <= cur_indent; ++indent_idx)
                    {
                        uint32_t last_entry_line_idx = last_enter_line[indent_idx];

                        // setup last entry line to exit node
                        _lines[last_entry_line_idx].indent_nodes[indent_idx] = EIndentNode::NodeExit;

                        // take back indent hint
                        for (uint32_t i = last_entry_line_idx + 1; i < line_idx; ++i)
                        {
                            _lines[i].indent_nodes[indent_idx] = EIndentNode::Empty;
                        }
                    }

                    // update cur indent
                    cur_indent = line_data.indent;

                    // add indent hint
                    for (uint32_t i = 0; i <= line_data.indent; ++i)
                    {
                        line_data.indent_nodes[i] = i == cur_indent ? EIndentNode::NodeEntry : EIndentNode::IndentHint;
                    }
                }
                else
                {
                    // just add indent hint
                    for (uint32_t i = 0; i <= cur_indent; ++i)
                    {
                        line_data.indent_nodes[i] = i == cur_indent ? EIndentNode::NodeEntry : EIndentNode::IndentHint;
                    }
                }

                // update enter line
                last_enter_line[line_data.indent] = line_idx;
            }

            // process end of node
            for (uint32_t indent_idx = 0; indent_idx <= cur_indent; ++indent_idx)
            {
                uint32_t last_entry_line_idx = last_enter_line[indent_idx];

                // setup last entry line to exit node
                _lines[last_entry_line_idx].indent_nodes[indent_idx] = EIndentNode::NodeExit;

                // take back indent hint
                for (uint32_t i = last_entry_line_idx + 1; i < _lines.size(); ++i)
                {
                    _lines[i].indent_nodes[indent_idx] = EIndentNode::Empty;
                }
            }
        }
    }
    inline String build()
    {
        if (_lines.is_empty())
        {
            return u8"";
        }

        solve_indent();

        String result;
        for (uint64_t i = 0; i < _lines.size(); ++i)
        {
            _build_line(_lines[i], result);
            result.append(u8"\n");
        }
        return result;
    }

    // dump error
    inline void dump_errors(const V8ErrorCache& error)
    {
        for (const auto& data : error.data())
        {
            switch (data.kind)
            {
            case V8ErrorCache::EKind::Error:
                write_line(u8"[ERROR] {}", data.content);
                break;
            case V8ErrorCache::EKind::Warning:
                write_line(u8"[WARNING] {}", data.content);
                break;
            }
        }
    }

private:
    inline void _build_line(const LineData& line_data, String& result) const
    {
        // build indent
        for (const auto& indent_node : line_data.indent_nodes)
        {
            switch (indent_node)
            {
            case EIndentNode::Empty:
                result.append(u8"  ");
                break;
            case EIndentNode::IndentHint:
                result.append(u8"| ");
                break;
            case EIndentNode::NodeEntry:
                result.append(u8"|-");
                break;
            case EIndentNode::NodeExit:
                result.append(u8"`-");
                break;
            }
        }

        // build content
        result.append(line_data.content);
    }

private:
    Vector<LineData> _lines = {};
    uint32_t _cur_indent = 0;
    bool _is_line_start = true;
};

} // namespace skr