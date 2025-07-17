#pragma once
#include "SkrBase/config.h"
#include "SkrBase/meta.h"
#include "SkrContainersDef/set.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrRTTR/type.hpp"
#ifndef __meta__
    #include "SkrCore/cli.generated.h"
#endif

// attributes
namespace skr::attr
{
// clang-format off
sreflect_struct(guid = "5ec06e6b-ca62-45ca-aa35-d39e1aba88a4")
CmdOption {
    // clang-format on

    String    name        = {};
    skr_char8 short_name  = {};
    String    help        = {};
    bool      is_required = true;
};
// clang-format off
sreflect_struct(guid = "3bd3f46e-e8fd-41e8-9c94-c74e2d93141b")
CmdExec {
    // clang-format on
};
// clang-format off
sreflect_struct(guid = "098c17f7-d906-45ea-86d2-180d73a1cb40")
CmdSub {
    // clang-format on

    String    name       = {};
    skr_char8 short_name = {};
    String    help       = {};
    String    usage      = {};
};
} // namespace skr::attr

// cli output builder
namespace skr
{
namespace cli_style
{
static const char* clear = "\033[0m";

// style
static const char* bold         = "\033[1m";
static const char* no_bold      = "\033[22m";
static const char* underline    = "\033[4m";
static const char* no_underline = "\033[24m";
static const char* reverse      = "\033[7m";
static const char* no_reverse   = "\033[27m";

// front color
static const char* front_gray    = "\033[30m";
static const char* front_red     = "\033[31m";
static const char* front_green   = "\033[32m";
static const char* front_yellow  = "\033[33m";
static const char* front_blue    = "\033[34m";
static const char* front_magenta = "\033[35m";
static const char* front_cyan    = "\033[36m";
static const char* front_white   = "\033[37m";
} // namespace cli_style

struct CliOutputBuilder {
    // style
    inline CliOutputBuilder& style_clear()
    {
        content.append(cli_style::clear);
        return *this;
    }
    inline CliOutputBuilder& style_bold()
    {
        content.append(cli_style::bold);
        return *this;
    }
    inline CliOutputBuilder& style_no_bold()
    {
        content.append(cli_style::no_bold);
        return *this;
    }
    inline CliOutputBuilder& style_underline()
    {
        content.append(cli_style::underline);
        return *this;
    }
    inline CliOutputBuilder& style_no_underline()
    {
        content.append(cli_style::no_underline);
        return *this;
    }
    inline CliOutputBuilder& style_reverse()
    {
        content.append(cli_style::reverse);
        return *this;
    }
    inline CliOutputBuilder& style_no_reverse()
    {
        content.append(cli_style::no_reverse);
        return *this;
    }
    inline CliOutputBuilder& style_front_gray()
    {
        content.append(cli_style::front_gray);
        return *this;
    }
    inline CliOutputBuilder& style_front_red()
    {
        content.append(cli_style::front_red);
        return *this;
    }
    inline CliOutputBuilder& style_front_green()
    {
        content.append(cli_style::front_green);
        return *this;
    }
    inline CliOutputBuilder& style_front_yellow()
    {
        content.append(cli_style::front_yellow);
        return *this;
    }
    inline CliOutputBuilder& style_front_blue()
    {
        content.append(cli_style::front_blue);
        return *this;
    }
    inline CliOutputBuilder& style_front_magenta()
    {
        content.append(cli_style::front_magenta);
        return *this;
    }
    inline CliOutputBuilder& style_front_cyan()
    {
        content.append(cli_style::front_cyan);
        return *this;
    }
    inline CliOutputBuilder& style_front_white()
    {
        content.append(cli_style::front_white);
        return *this;
    }

    // content
    template <typename... Args>
    inline CliOutputBuilder& write(StringView fmt, Args... args)
    {
        String str = format(fmt, std::forward<Args>(args)...);
        _solve_indent(str, _indent_cache);
        content.append(str);
        return *this;
    }
    template <typename... Args>
    inline CliOutputBuilder& line(StringView fmt, Args... args)
    {
        format_to(content, fmt, std::forward<Args>(args)...);
        next_line();
        return *this;
    }
    inline CliOutputBuilder& next_line()
    {
        content.append(u8"\n");
        _indent_cache = 0;
        return *this;
    }
    template <typename... Args>
    inline CliOutputBuilder& write_indent(StringView fmt, Args... args)
    {
        String   str = format(fmt, std::forward<Args>(args)...);
        uint64_t new_indent;
        _solve_indent(str, new_indent);
        if (_indent_cache > 0)
        {
            String indent_str;
            indent_str.add(u8'\n');
            indent_str.add(u8' ', _indent_cache);
            str.replace(u8"\n", indent_str);
        }
        _indent_cache = new_indent;
        content.append(str);
        return *this;
    }

    // dump
    inline void dump()
    {
        printf( content.c_str_raw());
    }

    String content;

private:
    inline static void _solve_indent(const String& str, uint64_t& indent)
    {
        if (auto found_last = str.find_last(u8"\n"))
        {
            indent = str.length_buffer() - found_last.index() - 1;
        }
        else
        {
            indent += str.length_buffer();
        }
    }
    uint64_t _indent_cache = 0;
};

} // namespace skr

namespace skr
{
struct CmdToken {
    enum class Type
    {
        Option,
        ShortOption,
        Name,
    };

    inline bool parse(StringView arg, CliOutputBuilder& out_err)
    {
        if (arg.starts_with(u8"--"))
        {
            type  = Type::Option;
            token = arg.subview(2);
        }
        else if (arg.starts_with(u8"-"))
        {
            type  = Type::ShortOption;
            token = arg.subview(1);
        }
        else
        {
            type  = Type::Name;
            token = arg;
        }
        return true;
    }

    inline bool is_any_option() const
    {
        return type == Type::Option || type == Type::ShortOption;
    }
    inline bool is_option() const
    {
        return type == Type::Option;
    }
    inline bool is_short_option() const
    {
        return type == Type::ShortOption;
    }
    inline bool is_name() const
    {
        return type == Type::Name;
    }

    inline String raw() const
    {
        String result;
        if (type == Type::Option)
        {
            result.append(u8"--");
        }
        else if (type == Type::ShortOption)
        {
            result.append(u8"-");
        }
        result.append(token);
        return result;
    }

    Type   type;
    String token;
};
struct SKR_CORE_API CmdOptionData {
    enum class EKind
    {
        None,
        Bool,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float,
        Double,
        String,
        StringVector,
    };

    bool init(attr::CmdOption config, TypeSignatureView type_sig, void* memory);

    // getter
    inline EKind                  kind() const { return _kind; }
    inline bool                   is_optional() const { return _is_optional; }
    inline const attr::CmdOption& config() const { return _config; }
    inline bool                   is_bool() const { return _kind == EKind::Bool; }
    inline bool                   is_uint() const { return _kind >= EKind::UInt8 && _kind <= EKind::UInt64; }
    inline bool                   is_int() const { return _kind >= EKind::Int8 && _kind <= EKind::Int64; }
    inline bool                   is_float() const { return _kind == EKind::Float || _kind == EKind::Double; }
    inline bool                   is_string() const { return _kind == EKind::String; }
    inline bool                   is_string_vector() const { return _kind == EKind::StringVector; }
    inline bool                   is_required() const { return _config.is_required && !_is_optional; }
    inline String                 dump_type_name() const
    {
        switch (_kind)
        {
        case EKind::Bool:
            return u8"<bool>     ";
        case EKind::Int8:
        case EKind::Int16:
        case EKind::Int32:
        case EKind::Int64:
            return u8"<int>      ";
        case EKind::UInt8:
        case EKind::UInt16:
        case EKind::UInt32:
        case EKind::UInt64:
            return u8"<uint>     ";
        case EKind::Float:
        case EKind::Double:
            return u8"<float>    ";
        case EKind::String:
            return u8"<string>   ";
        case EKind::StringVector:
            return u8"<string...>";
        default:
            return u8"<unknown>  ";
        }
    }

    // setter
    void set_value(bool v);
    void set_value(int64_t v);
    void set_value(uint64_t v);
    void set_value(double v);
    void set_value(String v);
    void add_value(String v);

private:
    // helper
    static EKind _solve_kind(const GUID& type_id);

    // as
    template <typename T>
    inline void _set(T&& v)
    {
        if (_is_optional)
        {
            _as<Optional<T>>() = std::forward<T>(v);
        }
        else
        {
            _as<T>() = std::forward<T>(v);
        }
    }
    template <typename T>
    inline void _set(const T& v)
    {
        if (_is_optional)
        {
            _as<Optional<T>>() = v;
        }
        else
        {
            _as<T>() = v;
        }
    }
    template <typename T>
    inline T& _as() const
    {
        return *reinterpret_cast<T*>(_memory);
    }

private:
    // attr
    attr::CmdOption _config = {};

    // native info
    EKind _kind        = EKind::None;
    bool  _is_optional = false;
    void* _memory      = nullptr;
};
struct SKR_CORE_API CmdNode {
    // basic info
    attr::CmdSub    config = {};
    const RTTRType* type   = nullptr;
    void*           object = nullptr;

    // options & invoke info
    Vector<CmdOptionData>       options     = {};
    ExportMethodInvoker<void()> exec_method = {};

    // sub command info
    Vector<CmdNode*> sub_cmds = {};

    // dtor
    ~CmdNode();

    // solve cmd
    bool solve();
    bool check_options();
    bool check_sub_commands();
    void print_help();
};
struct SKR_CORE_API CmdParser {
    // ctor & dtor
    CmdParser();
    ~CmdParser();

    // register command
    void main_cmd(const RTTRType* type, void* object, attr::CmdSub config = {});
    void sub_cmd(const RTTRType* type, void* object, attr::CmdSub config = {});

    // register command helper
    template <typename T>
    inline void main_cmd(T* object, attr::CmdSub config = {})
    {
        main_cmd(type_of<T>(), object, config);
    }
    template <typename T>
    inline void sub_cmd(T* object, attr::CmdSub config = {})
    {
        sub_cmd(type_of<T>(), object, config);
    }

    // parse
    void parse(int argc, char* argv[]);

private:
    // helper
    static span<const CmdToken> _find_option_param_pack(const Vector<CmdToken>& args, uint64_t option_idx);
    static span<const CmdToken> _find_rest_params_pack(const Vector<CmdToken>& args, uint64_t name_idx);
    static void                 _error_require_params(CliOutputBuilder& builder, const CmdToken& arg);
    static void                 _error_parse_params(CliOutputBuilder& builder, const CmdToken& arg, StringView param, StringView type);
    static void                 _error_unknown_option(CliOutputBuilder& builder, const CmdToken& arg);
    static bool                 _process_option(
                        CmdOptionData*       found_option,
                        uint32_t&            current_idx,
                        const CmdToken&      arg,
                        span<const CmdToken> params,
                        CliOutputBuilder&    builder,
                        Set<CmdOptionData*>& required_options
                    );

private:
    CmdNode _root_cmd = {};
};
} // namespace skr