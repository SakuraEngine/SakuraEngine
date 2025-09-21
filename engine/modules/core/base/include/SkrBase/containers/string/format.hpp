#pragma once
#include <SkrBase/containers/string/string.hpp>
#include <SkrBase/misc/debug.h>
#include <charconv>
#include <cmath>

// format parser
namespace skr::container
{
// 字符串操作的函数指针类型定义
using StringAppendCStr  = void (*)(void* pString, const char* content, size_t size);
using StringAppendU8Str = void (*)(void* pString, const skr_char8* content, size_t size);
using StringAppendChar  = void (*)(void* pString, skr_char8 ch);
using StringAddChars    = void (*)(void* pString, skr_char8 ch, size_t count);
using StringAppendView  = void (*)(void* pString, const void* view_data, size_t view_size);

struct StringFunctionTable
{
    void*             string_ptr;   // 类型擦除的字符串指针
    StringAppendCStr  append_cstr;  // append(const char*, size_t)
    StringAppendU8Str append_u8str; // append(const skr_char8*, size_t)
    StringAppendChar  append_char;  // append(skr_char8)
    StringAddChars    add_chars;    // add(skr_char8, size_t)
    StringAppendView  append_view;  // append(ViewType)
};

// 创建字符串的虚函数表
template <typename TString>
inline StringFunctionTable __virtualize(TString& string)
{
    StringFunctionTable table;
    table.string_ptr = &string;

    // append(const char*, size_t)
    table.append_cstr = [](void* str, const char* content, size_t size) {
        static_cast<TString*>(str)->append(content, size);
    };

    // append(const skr_char8*, size_t)
    table.append_u8str = [](void* str, const skr_char8* content, size_t size) {
        static_cast<TString*>(str)->append(content, size);
    };

    // append(skr_char8)
    table.append_char = [](void* str, skr_char8 ch) {
        static_cast<TString*>(str)->append(ch);
    };

    // add(skr_char8, size_t)
    table.add_chars = [](void* str, skr_char8 ch, size_t count) {
        static_cast<TString*>(str)->add(ch, count);
    };

    // append(ViewType)
    table.append_view = [](void* str, const void* view_data, size_t view_size) {
        using ViewType = typename TString::ViewType;
        ViewType view(static_cast<const skr_char8*>(view_data), view_size);
        static_cast<TString*>(str)->append(view);
    };

    return table;
}

template <typename TSize>
struct FormatToken
{
    using ViewType = U8StringView<TSize>;

    enum class Kind
    {
        Unknow,       // bad token
        PlainText,    // normal text
        EscapedBrace, // {{ or }}
        Formatter,    // content inside {}
    };

    Kind     kind = Kind::Unknow;
    ViewType view = {};
};
template <typename TSize>
struct FormatTokenIter
{
    using ViewType  = U8StringView<TSize>;
    using TokenType = FormatToken<TSize>;

    inline FormatTokenIter(ViewType format_str)
        : _format_str(format_str)
        , _index(0)
        , _cur_token{}
    {
        if (!_format_str.is_empty())
        {
            _parse_token();
        }
    }

    inline TokenType ref()
    {
        return _cur_token;
    }
    inline void reset()
    {
        _index = 0;
        if (!_format_str.is_empty())
        {
            _parse_token();
        }
    }
    inline void move_next()
    {
        if (_index < _format_str.size())
        {
            _index += _cur_token.view.size();
            if (_index < _format_str.size())
            {
                _parse_token();
            }
        }
    }
    inline bool has_next()
    {
        return _index < _format_str.size();
    }

private:
    inline void _parse_token()
    {
        auto begin_ch = _format_str.at_buffer(_index);

        if (begin_ch == u8'{') // formatter or plain text
        {
            if (_index < _format_str.size() - 1 && _format_str.at_buffer(_index + 1) == u8'{')
            { // escaped brace
                _cur_token.kind = TokenType::Kind::EscapedBrace;
                _cur_token.view = _format_str.subview(_index, 2);
            }
            else
            { // formatted
                if (auto found = _format_str.subview(_index).find(UTF8Seq{ u8'}' }))
                {
                    _cur_token.kind = TokenType::Kind::Formatter;
                    _cur_token.view = _format_str.subview(_index, found.index() + 1);
                }
                else
                {
                    _cur_token.kind = TokenType::Kind::Unknow;
                    _cur_token.view = _format_str.subview(_index, 1);
                    SKR_VERIFY(false && "unclosed right brace is not allowed!");
                }
            }
        }
        else if (begin_ch == u8'}')
        {
            if (_index < _format_str.size() - 1 && _format_str.at_buffer(_index + 1) == u8'}')
            {
                _cur_token.kind = TokenType::Kind::EscapedBrace;
                _cur_token.view = _format_str.subview(_index, 2);
            }
            else
            {
                _cur_token.kind = TokenType::Kind::Unknow;
                _cur_token.view = _format_str.subview(_index, 1);
                SKR_VERIFY(false && "unclosed left brace is not allowed!");
            }
        }
        else
        {
            auto search_str        = _format_str.subview(_index);
            auto found_left_brace  = search_str.find(UTF8Seq{ u8'{' });
            auto found_right_brace = search_str.find(UTF8Seq{ u8'}' });
            if (found_left_brace || found_right_brace)
            {
                auto next_index = found_left_brace && found_right_brace ? std::min(found_left_brace.index(), found_right_brace.index()) :
                    found_left_brace                                    ? found_left_brace.index() :
                                                                          found_right_brace.index();
                _cur_token.kind = TokenType::Kind::PlainText;
                _cur_token.view = _format_str.subview(_index, next_index);
            }
            else
            {
                _cur_token.kind = TokenType::Kind::PlainText;
                _cur_token.view = _format_str.subview(_index);
            }
        }
    }

private:
    ViewType  _format_str;
    TSize     _index;
    TokenType _cur_token;
};
} // namespace skr::container

// formatter
namespace skr::container
{
// 使用 StringFunctionTable 的类型擦除版本
void format_float_erased(const StringFunctionTable& table, double value, const skr_char8* spec_data, size_t spec_size);
void format_float_erased(const StringFunctionTable& table, float value, const skr_char8* spec_data, size_t spec_size);

// 类型擦除的整数格式化函数
void format_integer_erased(const StringFunctionTable& table, uint64_t value, const skr_char8* spec_data, size_t spec_size, bool is_signed);
void format_integer_erased(const StringFunctionTable& table, int64_t value, const skr_char8* spec_data, size_t spec_size);

template <typename TString>
inline void format_float(TString& out, double value, typename TString::ViewType spec)
{
    auto table = __virtualize(out);
    format_float_erased(table, value, spec.data(), spec.size());
}

template <typename T>
struct Formatter
{
    Formatter() = delete;
    // template <typename TString>
    // static void format(TString& out, const T& value, typename TString::ViewType spec);
};

// primitive types
template <std::signed_integral T>
struct Formatter<T>
{
    template <typename TString>
    inline static void format(TString& out, T value, typename TString::ViewType spec)
    {
        auto table = __virtualize(out);
        format_integer_erased(table, static_cast<int64_t>(value), spec.data(), spec.size());
    }
};
template <std::unsigned_integral T>
struct Formatter<T>
{
    template <typename TString>
    inline static void format(TString& out, T value, typename TString::ViewType spec)
    {
        auto table = __virtualize(out);
        format_integer_erased(table, static_cast<uint64_t>(value), spec.data(), spec.size(), false);
    }
};
template <>
struct Formatter<float>
{
    template <typename TString>
    inline static void format(TString& out, float value, typename TString::ViewType spec)
    {
        auto table = __virtualize(out);
        format_float_erased(table, value, spec.data(), spec.size());
    }
};
template <>
struct Formatter<double>
{
    template <typename TString>
    inline static void format(TString& out, double value, typename TString::ViewType spec)
    {
        auto table = __virtualize(out);
        format_float_erased(table, value, spec.data(), spec.size());
    }
};
template <>
struct Formatter<bool>
{
    template <typename TString>
    inline static void format(TString& out, bool value, typename TString::ViewType spec)
    {
        out.append(value ? u8"true" : u8"false");
    }
};

// raw string
template <>
struct Formatter<char>
{
    template <typename TString>
    inline static void format(TString& out, char value, typename TString::ViewType spec)
    {
        out.append((char8_t)value);
    }
};
template <>
struct Formatter<const char*>
{
    template <typename TString>
    inline static void format(TString& out, const char* value, typename TString::ViewType spec)
    {
        out.append(value);
    }
};
template <size_t N>
struct Formatter<char[N]>
{
    template <typename TString>
    inline static void format(TString& out, const char (&value)[N], typename TString::ViewType spec)
    {
        out.append(value);
    }
};
template <>
struct Formatter<skr_char8>
{
    template <typename TString>
    inline static void format(TString& out, skr_char8 value, typename TString::ViewType spec)
    {
        out.append(value);
    }
};
template <>
struct Formatter<const skr_char8*>
{
    template <typename TString>
    inline static void format(TString& out, const skr_char8* value, typename TString::ViewType spec)
    {
        out.append(value);
    }
};
template <size_t N>
struct Formatter<skr_char8[N]>
{
    template <typename TString>
    inline static void format(TString& out, const skr_char8 (&value)[N], typename TString::ViewType spec)
    {
        out.append(value);
    }
};

// string types
template <typename TSize>
struct Formatter<U8StringView<TSize>>
{
    template <typename TString>
    inline static void format(TString& out, U8StringView<TSize> value, typename TString::ViewType spec)
    {
        out.append(value);
    }
};
template <typename Memory>
struct Formatter<U8String<Memory>>
{
    template <typename TString>
    inline static void format(TString& out, const U8String<Memory>& value, typename TString::ViewType spec)
    {
        out.append(value);
    }
};

// stl string types
template <>
struct Formatter<std::string_view>
{
    template <typename TString>
    inline static void format(TString& out, std::string_view value, typename TString::ViewType spec)
    {
        out.append(value.data(), value.size());
    }
};
template <>
struct Formatter<std::string>
{
    template <typename TString>
    inline static void format(TString& out, const std::string& value, typename TString::ViewType spec)
    {
        out.append(value.data(), value.size());
    }
};
template <>
struct Formatter<std::u8string_view>
{
    template <typename TString>
    inline static void format(TString& out, std::u8string_view value, typename TString::ViewType spec)
    {
        out.append(value.data(), value.size());
    }
};
template <>
struct Formatter<std::u8string>
{
    template <typename TString>
    inline static void format(TString& out, const std::u8string& value, typename TString::ViewType spec)
    {
        out.append(value.data(), value.size());
    }
};

// pointer
template <>
struct Formatter<std::nullptr_t>
{
    template <typename TString>
    inline static void format(TString& out, std::nullptr_t value, typename TString::ViewType spec)
    {
        out.append(u8"0x0");
    }
};
template <>
struct Formatter<void*>
{
    template <typename TString>
    inline static void format(TString& out, const void* value, typename TString::ViewType spec)
    {
        if (value == nullptr)
        {
            Formatter<std::nullptr_t>::format(out, {}, spec);
        }
        else
        {
            auto table = __virtualize(out);
            format_integer_erased(table, reinterpret_cast<uint64_t>(value), u8"#x", 2, false);
        }
    }
};
template <>
struct Formatter<const void*>
{
    template <typename TString>
    inline static void format(TString& out, const void* value, typename TString::ViewType spec)
    {
        if (value == nullptr)
        {
            Formatter<std::nullptr_t>::format(out, {}, spec);
        }
        else
        {
            auto table = __virtualize(out);
            format_integer_erased(table, reinterpret_cast<uint64_t>(value), u8"#x", 2, false);
        }
    }
};
} // namespace skr::container

// format
namespace skr::container
{
template <typename TString, typename T>
concept Formattable = requires(TString& out, const T& v, typename TString::ViewType spec) {
    Formatter<T>::format(out, v, spec);
};

template <typename TString>
struct ArgFormatterPack
{
    using FormatterFunc = void (*)(TString&, const void*, typename TString::ViewType);

    template <typename T>
    inline ArgFormatterPack(const T& v)
        : arg(reinterpret_cast<const void*>(&v))
        , formatter(_make_formatter<T>())
    {
        static_assert(Formattable<TString, T>, "Type is not formattable!");
    }

    inline void format(TString& out, typename TString::ViewType& spec)
    {
        formatter(out, arg, spec);
    }

private:
    template <typename T>
    inline FormatterFunc _make_formatter()
    {
        return +[](TString& out, const void* value, typename TString::ViewType spec) {
            Formatter<T>::format(out, *reinterpret_cast<const T*>(value), spec);
        };
    }

public:
    const void*   arg;
    FormatterFunc formatter;
};

template <typename TString, typename... Args>
inline void format_to(TString& out, typename TString::ViewType view, Args&&... args)
{
    using SizeType  = typename TString::SizeType;
    using TokenIter = FormatTokenIter<SizeType>;

    constexpr uint64_t                               arg_count = sizeof...(Args);
    std::array<ArgFormatterPack<TString>, arg_count> formatters{ { ArgFormatterPack<TString>{ std::forward<Args>(args) }... } };

    out.reserve(out.size() + view.size());

    enum class IndexingMode
    {
        Unknown,
        Auto,
        Manual
    };
    IndexingMode indexing_mode = IndexingMode::Unknown;
    uint64_t     auto_index    = 0;
    for (TokenIter iter{ view }; iter.has_next(); iter.move_next())
    {
        auto token = iter.ref();
        switch (token.kind)
        {
        case FormatToken<SizeType>::Kind::PlainText:
            out.append(token.view);
            break;
        case FormatToken<SizeType>::Kind::EscapedBrace:
            out.append(token.view.first(1));
            break;
        case FormatToken<SizeType>::Kind::Formatter: {
            auto [arg_idx, mid, spec] = token.view.subview(1, token.view.size() - 2).partition(UTF8Seq{ u8':' });
            uint64_t cur_idx          = auto_index;
            if (arg_idx.is_empty())
            {
                SKR_VERIFY(indexing_mode != IndexingMode::Manual && "Manual index is not allowed mixing with automatic index!");
                indexing_mode = IndexingMode::Auto;
                ++auto_index;
            }
            else
            {
                SKR_VERIFY(indexing_mode != IndexingMode::Auto && "Automatic index is not allowed mixing with manual index!");
                indexing_mode = IndexingMode::Manual;
                // clang-format off
                    auto [last, error] = std::from_chars(
                        (const char*)arg_idx.data(),
                        (const char*)(arg_idx.data() + arg_idx.size()), 
                        cur_idx
                    );
                // clang-format on
                SKR_VERIFY(last == (const char*)(arg_idx.data() + arg_idx.size()) && "Invalid format index!");
            }

            // protate
            if (cur_idx < arg_count)
            {
                formatters.at(cur_idx).format(out, spec);
            }
            else
            {
                out.append(token.view);
            }
        }
        break;
        default:
            break;
        }
    }
}

template <typename TString, typename... Args>
inline TString format(typename TString::ViewType view, Args&&... args)
{
    TString out;
    format_to(out, view, std::forward<Args>(args)...);
    return out;
}

// TODO. formatted_size, format 后的尺寸
// TODO. vformat, 使用 FormatArgs 结构
} // namespace skr::container