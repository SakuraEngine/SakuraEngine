#pragma once
#include <cassert>
#include <string>
#include <vector>

namespace skr::CppSL {

enum ESourceBuilderStyle {
  // clear
  ESBS_Clear = 0,

  // style
  ESBS_Bold = 1,
  ESBS_NoBold = 2,
  ESBS_Underline = 3,
  ESBS_NoUnderline = 4,
  ESBS_Reverse = 5,
  ESBS_NoReverse = 6,

  // front color
  ESBS_FrontGray = 7,
  ESBS_FrontRed = 8,
  ESBS_FrontGreen = 9,
  ESBS_FrontYellow = 10,
  ESBS_FrontBlue = 11,
  ESBS_FrontMagenta = 12,
  ESBS_FrontCyan = 13,
  ESBS_FrontWhite = 14,
};

inline static const wchar_t *get_source_builder_style(ESourceBuilderStyle style) {
  switch (style) {
  // clear
  case ESBS_Clear:
    return L"\033[0m";

  // style
  case ESBS_Bold:
    return L"\033[1m";
  case ESBS_NoBold:
    return L"\033[22m";
  case ESBS_Underline:
    return L"\033[4m";
  case ESBS_NoUnderline:
    return L"\033[24m";
  case ESBS_Reverse:
    return L"\033[7m";
  case ESBS_NoReverse:
    return L"\033[27m";

  // front color
  case ESBS_FrontGray:
    return L"\033[30m";
  case ESBS_FrontRed:
    return L"\033[31m";
  case ESBS_FrontGreen:
    return L"\033[32m";
  case ESBS_FrontYellow:
    return L"\033[33m";
  case ESBS_FrontBlue:
    return L"\033[34m";
  case ESBS_FrontMagenta:
    return L"\033[35m";
  case ESBS_FrontCyan:
    return L"\033[36m";
  case ESBS_FrontWhite:
    return L"\033[37m";
  }
  assert(0 && "Invalid source builder style");
  return nullptr;
}

struct SourceBuilderNew {
  enum class EIndentNode {
    Empty,      // means last node in this indent level
    IndentHint, // means not last node in this indent level
    NodeEntry,  // means entry a sub node
    NodeExit,   // mean exit a sub node
  };
  struct LineData {
    std::wstring content = {};
    size_t indent = 0;
    std::vector<EIndentNode> indent_nodes = {};
  };

  //===================combine content===================
  SourceBuilderNew &append(const std::wstring &content) {
    // next line
    if (_is_line_start) {
      auto &line_data = _lines.emplace_back();
      line_data.indent = _cur_indent;
      _is_line_start = false;
    }

    // append content
    auto &line_data = _lines.back();
    line_data.content += content;

    return *this;
  }
  SourceBuilderNew &append_line() {
    append(L"");
    endline();
    return *this;
  }
  SourceBuilderNew &endline() {
    _is_line_start = true;
    return *this;
  }
  SourceBuilderNew &endline(wchar_t sep) {
    // append conent
    auto &line_data = _lines.back();
    line_data.content += sep;

    // setup flag
    _is_line_start = true;
    return *this;
  }
  SourceBuilderNew &endline(std::wstring_view sep) {
    // append conent
    auto &line_data = _lines.back();
    line_data.content += sep;

    // setup flag
    _is_line_start = true;
    return *this;
  }
  template <typename F>
  SourceBuilderNew &indent(F &&f) {
    ++_cur_indent;
    f();
    --_cur_indent;
    return *this;
  }

  //===================color & theme===================
  SourceBuilderNew &style(ESourceBuilderStyle style) {
    append(get_source_builder_style(style));
    return *this;
  }

  //===================themed append===================
  SourceBuilderNew &append(const std::wstring &content, ESourceBuilderStyle style) {
    append(get_source_builder_style(style));
    append(content);
    append(get_source_builder_style(ESBS_Clear));
    return *this;
  }
  SourceBuilderNew &append_expr(const std::wstring &content) {
    append(content, ESBS_FrontCyan);
    return *this;
  }
  SourceBuilderNew &append_decl(const std::wstring &content) {
    append(content, ESBS_FrontRed);
    return *this;
  }
  SourceBuilderNew &append_location(const std::wstring &content) {
    append(content, ESBS_FrontYellow);
    return *this;
  }
  SourceBuilderNew &append_type(const std::wstring &content) {
    append(content, ESBS_FrontGreen);
    return *this;
  }

  //===================build & clear===================
  template <typename F>
  std::wstring build(F &&line_builder) {
    if (_lines.empty()) {
      return L"";
    }

    // solve indent ops
    {
      // get max indent and resize indent nodes
      size_t max_indent = 0;
      for (auto &line_data : _lines) {
        max_indent = std::max(max_indent, line_data.indent);
        line_data.indent_nodes.resize(line_data.indent + 1, EIndentNode::Empty);
      }

      // last enter cache for detect last node
      std::vector<size_t> last_enter_line(max_indent + 1, size_t(-1));

      // process first line
      size_t cur_indent;
      {
        auto &line_data = _lines[0];
        cur_indent = line_data.indent;

        // record entry node
        for (size_t i = 0; i <= cur_indent; ++i) {
          last_enter_line[i] = 0;
          line_data.indent_nodes[i] = EIndentNode::NodeEntry;
        }
      }

      // process other lines
      for (size_t line_idx = 1; line_idx < _lines.size(); ++line_idx) {
        auto &line_data = _lines[line_idx];

        if (line_data.indent > cur_indent) {
          // add indent hint
          for (size_t i = 0; i <= line_data.indent; ++i) {
            line_data.indent_nodes[i] = EIndentNode::IndentHint;
          }

          // setup new indent entry node
          for (size_t i = cur_indent + 1; i <= line_data.indent; ++i) {
            last_enter_line[i] = line_idx;
            line_data.indent_nodes[i] = EIndentNode::NodeEntry;
          }

          // update cur indent
          cur_indent = line_data.indent;
        } else if (line_data.indent < cur_indent) {
          // take back overflow indent hint
          for (size_t indent_idx = line_data.indent + 1; indent_idx <= cur_indent; ++indent_idx) {
            size_t last_entry_line_idx = last_enter_line[indent_idx];

            // setup last entry line to exit node
            _lines[last_entry_line_idx].indent_nodes[indent_idx] = EIndentNode::NodeExit;

            // take back indent hint
            for (size_t i = last_entry_line_idx + 1; i < line_idx; ++i) {
              _lines[i].indent_nodes[indent_idx] = EIndentNode::Empty;
            }
          }

          // update cur indent
          cur_indent = line_data.indent;

          // add indent hint
          for (size_t i = 0; i <= line_data.indent; ++i) {
            line_data.indent_nodes[i] = i == cur_indent ? EIndentNode::NodeEntry : EIndentNode::IndentHint;
          }
        } else {
          // just add indent hint
          for (size_t i = 0; i <= cur_indent; ++i) {
            line_data.indent_nodes[i] = i == cur_indent ? EIndentNode::NodeEntry : EIndentNode::IndentHint;
          }
        }

        // update enter line
        last_enter_line[line_data.indent] = line_idx;
      }

      // process end of node
      for (size_t indent_idx = 0; indent_idx <= cur_indent; ++indent_idx) {
        size_t last_entry_line_idx = last_enter_line[indent_idx];

        // setup last entry line to exit node
        _lines[last_entry_line_idx].indent_nodes[indent_idx] = EIndentNode::NodeExit;

        // take back indent hint
        for (size_t i = last_entry_line_idx + 1; i < _lines.size(); ++i) {
          _lines[i].indent_nodes[indent_idx] = EIndentNode::Empty;
        }
      }
    }

    std::wstring result;
    for (size_t i = 0; i < _lines.size(); ++i) {
      const auto &line_data = _lines[i];

      // build line
      std::wstring line_content = line_builder(line_data);

      // append line
      result += line_content;
      result += L"\n";
    }

    return result;
  }
  void clear() {
    _lines.clear();
    _cur_indent = 0;
    _is_line_start = true;
  }
  bool is_empty() const {
    return _lines.empty();
  }

  //===================some builders===================
  static std::wstring line_builder_tree(const LineData &line_data) {
    std::wstring result;

    // build indent
    result += get_source_builder_style(ESBS_FrontBlue);
    for (const auto &indent_node : line_data.indent_nodes) {
      switch (indent_node) {
      case EIndentNode::Empty:
        result += L"  ";
        break;
      case EIndentNode::IndentHint:
        result += L"| ";
        break;
      case EIndentNode::NodeEntry:
        result += L"|-";
        break;
      case EIndentNode::NodeExit:
        result += L"`-";
        break;
      }
    }
    result += get_source_builder_style(ESBS_Clear);

    // build content
    result += line_data.content;

    return result;
  }

  static std::wstring line_builder_code(const LineData &line_data) {
    std::wstring result;

    // build indent
    for (size_t indent_idx = 0; indent_idx < line_data.indent; ++indent_idx) {
      result += L"  ";
    }

    // build content
    result += line_data.content;

    return result;
  }

  //===================merge & fork===================
  SourceBuilderNew fork() {
    SourceBuilderNew new_builder;
    new_builder._cur_indent = _cur_indent;
    return new_builder;
  }
  void merge(SourceBuilderNew &other) {
    // early out
    if (other.is_empty()) {
      return;
    }

    // append line if needed
    if (_is_line_start) {
      auto &line_data = _lines.emplace_back();
      line_data.indent = _cur_indent;
      line_data.content = {};
      _is_line_start = false;
    }

    // merge first line
    {
      // append line conent
      auto &line_data = _lines.back();
      line_data.content += other._lines.front().content;

      if (line_data.content.empty()) {
        line_data.indent = other._lines.front().indent;
      } else {
        // keep indent same when merge line
        assert(line_data.indent == other._lines.front().indent);
      }
    }

    // merge other lines
    for (size_t i = 1; i < other._lines.size(); ++i) {
      auto &line_data = _lines.emplace_back();
      line_data.content = other._lines[i].content;
      line_data.indent = other._lines[i].indent;
    }

    // merge line status
    _is_line_start = other._is_line_start;
  }

private:
  std::vector<LineData> _lines = {};
  size_t _cur_indent = 0;
  bool _is_line_start = true;
};
} // namespace skr::CppSL