#pragma once
#include <SkrCore/serialize/serialize_traits.hpp>
#include <SkrContainersDef/function_ref.hpp>
#include <SkrContainers/path.hpp>

//! NOTE: current just a yyjson adapter
namespace skr
{
// fwd
struct JsonDocument;
struct JsonDocumentMut;
struct JsonValue;
struct JsonValueMut;

// json value type
enum class EJsonValueType : uint32_t
{
    Raw,    // 未解析的原始串
    String, // 字符串
    Null,   // null 字面量
    Bool,   // true/false 字面量
    Int,    // int64_t
    UInt,   // uint64_t
    Real,   // double
    Array,  // []
    Object, // {}
};

// json read flag
inline static constexpr uint64_t kJsonInSituPaddingCount = 4;
enum class EJsonReadFlags : uint32_t
{
    None = 0,

    // 允许修改输入串来防止 malloc 提升性能
    // 要求输入串有 kInSituPaddingCount 的剩余空间
    InSitu = 1 << 0,

    // 当解析完一串完整 Json 时，如果后面还有内容则停止而不是报错
    // 用于解析大量的小段 Json，如 NDJSON
    StopWhenDone = 1 << 1,

    //! [非标准]
    // 允许数组或对象以一个空逗号结束，如：
    // [1,2,3,] { "a":1, "b":2, }
    AllowTrailingCommas = 1 << 2,

    //! [非标准]
    // 允许存在 C 风格注释
    AllowComment = 1 << 3,

    //! [非标准]
    // 允许 inf/nan 字面量，大小写不敏感
    // 比如 1e999, inf, -Infinity ...
    AllowInfAndNan = 1 << 4,

    // 将数字读取为 Raw（未解析的串）
    ReadNumbersAsRaw = 1 << 5,

    // 允许无效的 unicode 编码
    AllowInvalidUnicode = 1 << 6,

    // 允许将大整数读取为 Raw（未解析的串）
    AllowBigIntAsRaw = 1 << 7,

    //! [非标准]
    // 允许使用 UTF-8 BOM 头
    AllowUtf8Bom = 1 << 8,

    //! [非标准]
    // 允许使用扩展的数字写法，如
    // - 十六进制 0x1A3F
    // - 简写浮点 123. .123
    // - 正数 +123
    AllowExtNum = 1 << 9,

    //! [非标准]
    // 允许一些扩展的反转义序列
    AllowExtEscape = 1 << 10,

    //! [非标准]
    // 允许一些扩展的空白字符
    AllowExtWhiteSpace = 1 << 11,

    //! [非标准]
    // 允许单引号字符串
    AllowSingleQuoteStr = 1 << 12,

    //! [非标准]
    // 允许无引号的键名
    AllowUnquotedKey = 1 << 13,

    // json5 标准的特性
    Json5 =
        AllowTrailingCommas |
        AllowComment |
        AllowInfAndNan |
        AllowExtNum |
        AllowExtEscape |
        AllowExtWhiteSpace |
        AllowSingleQuoteStr |
        AllowUnquotedKey,
};
enum class EJsonWriteFlags : uint32_t
{
    None = 0,

    // 输出可读的 4 空格缩进的格式
    Pretty = 1 << 0,

    // 对 unicode 字符进行转义
    EscapeUnicode = 1 << 1,

    // 将 '/' 转义为 '\/'
    EscapeSlash = 1 << 2,

    //! [非标准]
    // 支持 inf 和 nan 字面量
    AllowInfAndNan = 1 << 3,

    //! [非标准]
    // 将 inf 和 nan 写为 null 字面量
    WriteInfAndNanAsNull = 1 << 4,

    //! [非标准]
    // 支持非法的 unicode 字符
    AllowInvalidUnicode = 1 << 5,

    // 输出可读的 2 空格缩进的格式
    Pretty2Spaces = 1 << 6,

    // 在输出的末尾附加一个换行符，方便构建 NDJSON
    AppendNewline = 1 << 7,
};

// iterator
struct SKR_CORE_API JsonIterArray
{
    inline ~JsonIterArray() = default;

    // copy & move
    inline JsonIterArray(const JsonIterArray&) = default;
    inline JsonIterArray(JsonIterArray&&) = default;

    // assign & move assign
    inline JsonIterArray& operator=(const JsonIterArray&) = default;
    inline JsonIterArray& operator=(JsonIterArray&&) = default;

    // has next value
    bool has_next();

    // return next value, if not has next, return nullptr
    JsonValue* move_next();

private:
    friend struct JsonValue;
    inline JsonIterArray() = default; // construct by JsonValue

private:
    size_t _payload[3];
};
struct SKR_CORE_API JsonIterObject
{
    inline ~JsonIterObject() = default;

    // copy & move
    inline JsonIterObject(const JsonIterObject&) = default;
    inline JsonIterObject(JsonIterObject&&) = default;

    // assign & move assign
    inline JsonIterObject& operator=(const JsonIterObject&) = default;
    inline JsonIterObject& operator=(JsonIterObject&&) = default;

    // has next key-value pair
    bool has_next();

    // return next key, if not has next, return nullptr
    // use value_of to get it's value
    JsonValue* move_next();

    // return next found value of key, if not found, return nullptr
    JsonValue* move_next_key(StringView key);

    // get bounded value of key
    JsonValue* value_of(JsonValue* key);

private:
    friend struct JsonValue;
    inline JsonIterObject() = default; // construct by JsonValue

private:
    size_t _payload[4];
};
struct SKR_CORE_API JsonIterArrayMut
{
    inline ~JsonIterArrayMut() = default;

    // copy & move
    inline JsonIterArrayMut(const JsonIterArrayMut&) = default;
    inline JsonIterArrayMut(JsonIterArrayMut&&) = default;

    // assign & move assign
    inline JsonIterArrayMut& operator=(const JsonIterArrayMut&) = default;
    inline JsonIterArrayMut& operator=(JsonIterArrayMut&&) = default;

    // has next value
    bool has_next();

    // return next value, if not has next, return nullptr
    JsonValueMut* move_next();

    // remove and return current value
    JsonValueMut* remove_current();

private:
    friend struct JsonValueMut;
    inline JsonIterArrayMut() = default; // construct by JsonValueMut

private:
    size_t _payload[5];
};
struct SKR_CORE_API JsonIterObjectMut
{
    inline ~JsonIterObjectMut() = default;

    // copy & move
    inline JsonIterObjectMut(const JsonIterObjectMut&) = default;
    inline JsonIterObjectMut(JsonIterObjectMut&&) = default;

    // assign & move assign
    inline JsonIterObjectMut& operator=(const JsonIterObjectMut&) = default;
    inline JsonIterObjectMut& operator=(JsonIterObjectMut&&) = default;

    // has next key-value pair
    bool has_next();

    // return next key, if not has next, return nullptr
    // use value_of to get it's value
    JsonValueMut* move_next();

    // return next found value of key, if not found, return nullptr
    JsonValueMut* move_next_key(StringView key);

    // get bounded value of key
    JsonValueMut* value_of(JsonValueMut* key);

    // remove and return current key-value pair's value
    JsonValueMut* remove_current();

private:
    friend struct JsonValueMut;
    inline JsonIterObjectMut() = default; // construct by JsonValueMut

private:
    size_t _payload[5];
};

// error
struct JsonReadResult
{
    StringView error_msg = {};
    uint64_t error_line = 0;
    uint64_t error_col = 0;
    JsonDocument* value = nullptr;

    JsonReadResult(StringView msg, uint64_t line, uint64_t col);
    JsonReadResult(JsonDocument* doc);
    ~JsonReadResult();
    operator bool() const;
    bool is_success() const;
    bool is_failed() const;
    void dump_error() const;
    void mark_processed() const;

private:
    mutable bool _error_processed = false;
};
struct JsonWriteResult
{
    StringView error_msg = {};

    JsonWriteResult();
    JsonWriteResult(StringView msg);
    ~JsonWriteResult();
    operator bool() const;
    bool is_success() const;
    bool is_failed() const;
    void dump_error() const;
    void mark_processed() const;

private:
    mutable bool _error_processed = false;
};

// write callback
using JsonWriteCallback = FunctionRef<void(const void* data, uint64_t size)>;

// document & value
struct SKR_CORE_API JsonDocument
{
    // non-instantiable
    JsonDocument() = delete;
    ~JsonDocument() = delete;
    SKR_DELETE_COPY_MOVE(JsonDocument);

    // read from buffer, null for failed
    static JsonReadResult ReadBuffer(const void* buffer, uint64_t len, EJsonReadFlags flags = EJsonReadFlags::None);

    // read from file, null for failed
    static JsonReadResult ReadFile(const Path& path, EJsonReadFlags flags = EJsonReadFlags::None);

    // destroy a document
    static void Destroy(JsonDocument* doc);

    // getter
    JsonValue* root();
    uint64_t read_size();
    uint64_t value_count();

    // to mut doc
    JsonDocumentMut* to_mut();

    // write
    JsonWriteResult write_to(JsonWriteCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None);
};
struct SKR_CORE_API JsonDocumentMut
{
    // non-instantiable
    JsonDocumentMut() = delete;
    ~JsonDocumentMut() = delete;
    SKR_DELETE_COPY_MOVE(JsonDocumentMut);

    // create a mutable document
    static JsonDocumentMut* Create();
    static SP<JsonDocumentMut> CreateSP();
    static UPtr<JsonDocumentMut> CreateUPtr();

    // destroy a mutable document
    static void Destroy(JsonDocumentMut* doc);

    // root value
    JsonValueMut* root();
    void set_root(JsonValueMut* value);

    // create value
    JsonValueMut* create_raw(StringView raw);
    JsonValueMut* create_string(StringView str);
    JsonValueMut* create_raw_non_copy(StringView raw);
    JsonValueMut* create_string_non_copy(StringView str);
    JsonValueMut* create_null();
    JsonValueMut* create_bool(bool b);
    JsonValueMut* create_int(int64_t i);
    JsonValueMut* create_uint(uint64_t u);
    JsonValueMut* create_real(double f);
    JsonValueMut* create_object();
    JsonValueMut* create_array();

    // copy a value from another doc
    JsonValueMut* copy_value(JsonValue* other);
    JsonValueMut* copy_value(JsonValueMut* other);

    // copy document
    JsonDocumentMut* copy();
    JsonDocument* copy_readonly();

    // pooling optimize
    void set_pool_size_str(uint64_t size);
    void set_pool_size_val(uint64_t val_count);

    // write
    JsonWriteResult write_to(JsonWriteCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None);
};
struct SKR_CORE_API JsonValue
{
    // non-instantiable
    JsonValue() = delete;
    ~JsonValue() = delete;
    SKR_DELETE_COPY_MOVE(JsonValue);

    // get type
    EJsonValueType type();
    bool is_raw();
    bool is_null();
    bool is_bool();
    bool is_int();
    bool is_uint();
    bool is_real();
    bool is_number();
    bool is_string();
    bool is_object();
    bool is_array();
    bool is_container(); // array / object

    // get value
    String get_raw();
    StringView get_raw_view();
    String get_string();
    StringView get_string_view();
    bool get_bool();
    int64_t get_int();
    uint64_t get_uint();
    double get_real();

    // convert to api
    template <typename T>
    Optional<T> try_convert_to();

    // get or
    inline String get_raw_or(StringView default_v = {}) { return is_raw() ? get_raw() : String(default_v); }
    inline StringView get_raw_view_or(StringView default_v = {}) { return is_raw() ? get_raw_view() : default_v; }
    inline String get_string_or(StringView default_v = {}) { return is_string() ? get_string() : String(default_v); }
    inline StringView get_string_view_or(StringView default_v = {}) { return is_string() ? get_string_view() : default_v; }
    inline bool get_bool_or(bool default_v = false) { return is_bool() ? get_bool() : default_v; }
    inline int64_t get_int_or(int64_t default_v = 0) { return is_int() ? get_int() : default_v; }
    inline uint64_t get_uint_or(uint64_t default_v = 0) { return is_uint() ? get_uint() : default_v; }
    inline double get_real_or(double default_v = 0.0) { return is_real() ? get_real() : default_v; }

    // get length
    uint64_t get_raw_length();
    uint64_t get_string_length();
    uint64_t array_length();
    uint64_t object_length();

    // array api
    using ArrayForeachCallback = FunctionRef<bool(JsonValue* value, uint64_t index)>;
    void array_foreach(ArrayForeachCallback callback);
    JsonValue* array_at(uint64_t index);
    JsonIterArray array_iter();

    // object api
    using ObjectForeachCallback = FunctionRef<bool(StringView key, JsonValue* value)>;
    void object_foreach(ObjectForeachCallback callback);
    JsonValue* object_get(StringView key);
    JsonIterObject object_iter();

    // write
    JsonWriteResult write_to(JsonWriteCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None);
};
struct SKR_CORE_API JsonValueMut
{
    // non-instantiable
    JsonValueMut() = delete;
    ~JsonValueMut() = delete;
    SKR_DELETE_COPY_MOVE(JsonValueMut);

    // get type
    EJsonValueType type();
    bool is_raw();
    bool is_null();
    bool is_bool();
    bool is_int();
    bool is_uint();
    bool is_real();
    bool is_number();
    bool is_string();
    bool is_object();
    bool is_array();
    bool is_container(); // array / object

    // get value
    String get_raw();
    StringView get_raw_view();
    String get_string();
    StringView get_string_view();
    bool get_bool();
    int64_t get_int();
    uint64_t get_uint();
    double get_real();

    // convert to api
    template <typename T>
    Optional<T> try_convert_to();

    // get or
    inline String get_raw_or(StringView default_v = {}) { return is_raw() ? get_raw() : String(default_v); }
    inline StringView get_raw_view_or(StringView default_v = {}) { return is_raw() ? get_raw_view() : default_v; }
    inline String get_string_or(StringView default_v = {}) { return is_string() ? get_string() : String(default_v); }
    inline StringView get_string_view_or(StringView default_v = {}) { return is_string() ? get_string_view() : default_v; }
    inline bool get_bool_or(bool default_v = false) { return is_bool() ? get_bool() : default_v; }
    inline int64_t get_int_or(int64_t default_v = 0) { return is_int() ? get_int() : default_v; }
    inline uint64_t get_uint_or(uint64_t default_v = 0) { return is_uint() ? get_uint() : default_v; }
    inline double get_real_or(double default_v = 0.0) { return is_real() ? get_real() : default_v; }

    // get length
    uint64_t get_raw_length();
    uint64_t get_string_length();
    uint64_t array_length();
    uint64_t object_length();

    // array api
    using ArrayForeachCallback = FunctionRef<bool(JsonValueMut* value, uint64_t index)>;
    void array_foreach(ArrayForeachCallback callback);
    JsonValueMut* array_at(uint64_t index);
    JsonIterArrayMut array_iter();

    // object api
    using ObjectForeachCallback = FunctionRef<bool(StringView key, JsonValueMut* value)>;
    void object_foreach(ObjectForeachCallback callback);
    JsonValueMut* object_get(StringView key);
    JsonIterObjectMut object_iter();

    // write
    JsonWriteResult write_to(JsonWriteCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None);

    // copy to readonly document
    JsonDocument* copy_readonly();

    // array modify api
    bool arrray_insert(JsonValueMut* value, uint64_t index);
    JsonValueMut* arrray_replace(JsonValueMut* value, uint64_t index);
    bool arrray_push_back(JsonValueMut* value);
    bool arrray_push_front(JsonValueMut* value);
    JsonValueMut* arrray_pop_back();
    JsonValueMut* arrray_pop_front();
    JsonValueMut* arrray_remove_at(uint64_t index);
    bool arrray_remove_range(uint64_t index, uint64_t count);
    bool arrray_clear();
    bool arrray_rotate(uint64_t mid_index);

    // object modify api
    bool object_add(JsonValueMut* key, JsonValueMut* value);     // allow duplicate key
    bool object_set(JsonValueMut* key, JsonValueMut* value);     // not allow duplicate key
    bool object_replace(JsonValueMut* key, JsonValueMut* value); // only replace exist key
    bool object_remove_all(StringView key);
    bool object_rename_key(JsonDocumentMut* doc, StringView old_key, StringView new_key);
    bool object_clear();
};

// impl delete traits for use SP/UPtr to manage it
template <>
struct SPDeleterTraits<JsonDocument>
{
    inline static void do_delete(JsonDocument* p)
    {
        JsonDocument::Destroy(p);
    }
};
template <>
struct SPDeleterTraits<JsonDocumentMut>
{
    inline static void do_delete(JsonDocumentMut* p)
    {
        JsonDocumentMut::Destroy(p);
    }
};
} // namespace skr

// error impl
namespace skr
{
// json read result
inline JsonReadResult::JsonReadResult(StringView msg, uint64_t line, uint64_t col)
    : error_msg(msg)
    , error_line(line)
    , error_col(col)
{
}
inline JsonReadResult::JsonReadResult(JsonDocument* doc)
    : value(doc)
{
}
inline JsonReadResult::~JsonReadResult()
{
    if (is_failed() && !_error_processed)
    { // auto dump
        dump_error();
    }
}
inline JsonReadResult::operator bool() const
{
    return is_success();
}
inline bool JsonReadResult::is_success() const
{
    return error_msg.is_empty();
}
inline bool JsonReadResult::is_failed() const
{
    return !is_success();
}
inline void JsonReadResult::mark_processed() const
{
    _error_processed = true;
}
inline void JsonReadResult::dump_error() const
{
    if (is_failed())
    {
        SKR_LOG_FMT_ERROR(
            u8"Json parse error at {}:{}: {}",
            error_line,
            error_col,
            error_msg
        );
        mark_processed();
    }
}

// json write result
inline JsonWriteResult::JsonWriteResult()
{
}
inline JsonWriteResult::JsonWriteResult(StringView msg)
    : error_msg(msg)
{
}
inline JsonWriteResult::~JsonWriteResult()
{
    if (is_failed() && !_error_processed)
    { // auto dump
        dump_error();
    }
}
inline JsonWriteResult::operator bool() const
{
    return is_success();
}
inline bool JsonWriteResult::is_success() const
{
    return error_msg.is_empty();
}
inline bool JsonWriteResult::is_failed() const
{
    return !is_success();
}
inline void JsonWriteResult::dump_error() const
{
    if (is_failed())
    {
        SKR_LOG_FMT_ERROR(
            u8"Json write error: {}",
            error_msg
        );
        mark_processed();
    }
}
inline void JsonWriteResult::mark_processed() const
{
    _error_processed = true;
}
} // namespace skr

// some impl
namespace skr
{
namespace json_helper
{
template <typename T, typename JsonValueType>
inline Optional<T> _try_convert_to_impl(JsonValueType* json_value)
{
    if constexpr (std::is_same_v<T, bool>)
    {
        if (json_value->is_bool())
        {
            return json_value->get_bool();
        }
        else
        {
            return {};
        }
    }
    else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
    {
        auto min_v = (int64_t)std::numeric_limits<T>::min();
        auto max_v = (int64_t)std::numeric_limits<T>::max();

        switch (json_value->type())
        {
        case EJsonValueType::Int: {
            auto v = json_value->get_int();
            if (v >= min_v && v <= max_v)
                return static_cast<T>(v);
            else
                return {};
        }
        case EJsonValueType::UInt: {
            auto v = json_value->get_uint();
            if (v <= static_cast<uint64_t>(max_v))
                return static_cast<T>(v);
            else
                return {};
        }
        default:
            return {};
        }
    }
    else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
    {
        auto max_v = (uint64_t)std::numeric_limits<T>::max();

        switch (json_value->type())
        {
        case EJsonValueType::Int: {
            auto v = json_value->get_int();
            if (v >= 0 && static_cast<uint64_t>(v) <= max_v)
                return static_cast<T>(v);
            else
                return {};
        }
        case EJsonValueType::UInt: {
            auto v = json_value->get_uint();
            if (v <= max_v)
                return static_cast<T>(v);
            else
                return {};
        }
        default:
            return {};
        }
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        if (json_value->is_real())
        {
            auto v = json_value->get_real();
            if (v >= std::numeric_limits<T>::lowest() && v <= std::numeric_limits<T>::max())
                return static_cast<T>(v);
            else
                return {};
        }
        else if (json_value->is_int())
        { //! cast but may lost precision
            return static_cast<T>(json_value->get_int());
        }
        else if (json_value->is_uint())
        { //! cast but may lost precision
            return static_cast<T>(json_value->get_uint());
        }
        else
        {
            return {};
        }
    }
    else
    {
        static_assert(std::is_same_v<T, T*>, "JsonValue::try_convert_to<T> not support this type");
        return {};
    }
}
} // namespace json_helper

// convert to api
template <typename T>
inline Optional<T> JsonValue::try_convert_to()
{
    return json_helper::_try_convert_to_impl<T>(this);
}

// convert to api
template <typename T>
inline Optional<T> JsonValueMut::try_convert_to()
{
    return json_helper::_try_convert_to_impl<T>(this);
}
} // namespace skr

// common helper use handle both JsonValue and JsonValueMut
namespace skr
{
struct JsonValueCommon
{
    // ctor
    inline JsonValueCommon(JsonValue* value)
        : _has_value(value != nullptr)
        , _is_mut(false)
        , value(value)
    {
    }
    inline JsonValueCommon(JsonValueMut* value_mut)
        : _has_value(value_mut != nullptr)
        , _is_mut(true)
        , value_mut(value_mut)
    {
    }
    inline JsonValueCommon()
        : _has_value(false)
        , _is_mut(false)
        , value(nullptr)
    {
    }
    inline JsonValueCommon(std::nullptr_t)
        : _has_value(false)
        , _is_mut(false)
        , value(nullptr)
    {
    }

    // copy & move
    inline JsonValueCommon(const JsonValueCommon& rhs)
        : _has_value(rhs._has_value)
        , _is_mut(rhs._is_mut)
    {
        if (_is_mut)
            value_mut = rhs.value_mut;
        else
            value = rhs.value;
    }
    inline JsonValueCommon(JsonValueCommon&& rhs) noexcept
        : _has_value(rhs._has_value)
        , _is_mut(rhs._is_mut)
    {
        if (_is_mut)
            value_mut = rhs.value_mut;
        else
            value = rhs.value;
        rhs._has_value = false;
        rhs._is_mut = false;
        rhs.value = nullptr;
    }

    // assign & move assign
    inline JsonValueCommon& operator=(const JsonValueCommon& rhs)
    {
        if (this != &rhs)
        {
            _has_value = rhs._has_value;
            _is_mut = rhs._is_mut;
            if (_is_mut)
                value_mut = rhs.value_mut;
            else
                value = rhs.value;
        }
        return *this;
    }
    inline JsonValueCommon& operator=(JsonValueCommon&& rhs) noexcept
    {
        if (this != &rhs)
        {
            _has_value = rhs._has_value;
            _is_mut = rhs._is_mut;
            if (_is_mut)
                value_mut = rhs.value_mut;
            else
                value = rhs.value;
            rhs._has_value = false;
            rhs._is_mut = false;
            rhs.value = nullptr;
        }
        return *this;
    }

    // assign
    inline JsonValueCommon& operator=(JsonValue* v)
    {
        _has_value = (v != nullptr);
        _is_mut = false;
        value = v;
        return *this;
    }
    inline JsonValueCommon& operator=(JsonValueMut* v_mut)
    {
        _has_value = (v_mut != nullptr);
        _is_mut = true;
        value_mut = v_mut;
        return *this;
    }
    inline JsonValueCommon& operator=(std::nullptr_t)
    {
        _has_value = false;
        _is_mut = false;
        value = nullptr;
        return *this;
    }

    // has value
    inline bool has_value() const { return _has_value; }
    inline operator bool() const { return _has_value; }
    inline bool operator==(std::nullptr_t) const { return !_has_value; }
    inline bool operator!=(std::nullptr_t) const { return _has_value; }

    // reset
    inline void reset()
    {
        _has_value = false;
        _is_mut = false;
        value = nullptr;
    }

    // is mut
    inline bool is_mut() const { return _is_mut; }
    inline JsonValue* get_value() const
    {
        SKR_ASSERT(_has_value);
        SKR_ASSERT(!_is_mut);
        return value;
    }
    inline JsonValueMut* get_value_mut() const
    {
        SKR_ASSERT(_has_value);
        SKR_ASSERT(_is_mut);
        return value_mut;
    }

    // get type
    inline EJsonValueType type()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->type() : value->type();
    }
    inline bool is_raw()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_raw() : value->is_raw();
    }
    inline bool is_null()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_null() : value->is_null();
    }
    inline bool is_bool()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_bool() : value->is_bool();
    }
    inline bool is_int()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_int() : value->is_int();
    }
    inline bool is_uint()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_uint() : value->is_uint();
    }
    inline bool is_real()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_real() : value->is_real();
    }
    inline bool is_number()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_number() : value->is_number();
    }
    inline bool is_string()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_string() : value->is_string();
    }
    inline bool is_object()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_object() : value->is_object();
    }
    inline bool is_array()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_array() : value->is_array();
    }
    inline bool is_container()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->is_container() : value->is_container();
    }

    // get value
    inline String get_raw()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_raw() : value->get_raw();
    }
    inline StringView get_raw_view()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_raw_view() : value->get_raw_view();
    }
    inline String get_string()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_string() : value->get_string();
    }
    inline StringView get_string_view()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_string_view() : value->get_string_view();
    }
    inline bool get_bool()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_bool() : value->get_bool();
    }
    inline int64_t get_int()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_int() : value->get_int();
    }
    inline uint64_t get_uint()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_uint() : value->get_uint();
    }
    inline double get_real()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_real() : value->get_real();
    }

    // convert to api
    template <typename T>
    inline Optional<T> try_convert_to()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->try_convert_to<T>() : value->try_convert_to<T>();
    }

    // get or
    inline String get_raw_or(StringView default_v = {})
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_raw_or(default_v) : value->get_raw_or(default_v);
    }
    inline StringView get_raw_view_or(StringView default_v = {})
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_raw_view_or(default_v) : value->get_raw_view_or(default_v);
    }
    inline String get_string_or(StringView default_v = {})
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_string_or(default_v) : value->get_string_or(default_v);
    }
    inline StringView get_string_view_or(StringView default_v = {})
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_string_view_or(default_v) : value->get_string_view_or(default_v);
    }
    inline bool get_bool_or(bool default_v = false)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_bool_or(default_v) : value->get_bool_or(default_v);
    }
    inline int64_t get_int_or(int64_t default_v = 0)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_int_or(default_v) : value->get_int_or(default_v);
    }
    inline uint64_t get_uint_or(uint64_t default_v = 0)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_uint_or(default_v) : value->get_uint_or(default_v);
    }
    inline double get_real_or(double default_v = 0.0)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_real_or(default_v) : value->get_real_or(default_v);
    }

    // get length
    inline uint64_t get_raw_length()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_raw_length() : value->get_raw_length();
    }
    inline uint64_t get_string_length()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->get_string_length() : value->get_string_length();
    }
    inline uint64_t array_length()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->array_length() : value->array_length();
    }
    inline uint64_t object_length()
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->object_length() : value->object_length();
    }

    // array api
    using ArrayForeachCallback = FunctionRef<bool(JsonValueCommon value, uint64_t index)>;
    inline void array_foreach(ArrayForeachCallback callback)
    {
        SKR_ASSERT(_has_value);
        if (_is_mut)
        {
            value_mut->array_foreach([&](JsonValueMut* v, uint64_t i) {
                return callback(JsonValueCommon(v), i);
            });
        }
        else
        {
            value->array_foreach([&](JsonValue* v, uint64_t i) {
                return callback(JsonValueCommon(v), i);
            });
        }
    }
    inline JsonValueCommon array_at(uint64_t index)
    {
        SKR_ASSERT(_has_value);
        if (_is_mut)
        {
            return JsonValueCommon(value_mut->array_at(index));
        }
        else
        {
            return JsonValueCommon(value->array_at(index));
        }
    }

    // object api
    using ObjectForeachCallback = FunctionRef<bool(StringView key, JsonValueCommon value)>;
    inline void object_foreach(ObjectForeachCallback callback)
    {
        SKR_ASSERT(_has_value);
        if (_is_mut)
        {
            value_mut->object_foreach([&](StringView k, JsonValueMut* v) {
                return callback(k, JsonValueCommon(v));
            });
        }
        else
        {
            value->object_foreach([&](StringView k, JsonValue* v) {
                return callback(k, JsonValueCommon(v));
            });
        }
    }
    inline JsonValueCommon object_get(StringView key)
    {
        SKR_ASSERT(_has_value);
        if (_is_mut)
        {
            return JsonValueCommon(value_mut->object_get(key));
        }
        else
        {
            return JsonValueCommon(value->object_get(key));
        }
    }

    // write
    using WriteToCallback = FunctionRef<void(const void* data, uint64_t size)>;
    inline JsonWriteResult write_to(WriteToCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->write_to(callback, flags) : value->write_to(callback, flags);
    }
    inline JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->write_to_string(str, flags) : value->write_to_string(str, flags);
    }
    inline JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None)
    {
        SKR_ASSERT(_has_value);
        return _is_mut ? value_mut->write_to_file(path, flags) : value->write_to_file(path, flags);
    }

private:
    bool _has_value = false;
    bool _is_mut = false;
    union
    {
        JsonValue* value = nullptr;
        JsonValueMut* value_mut;
    };
};
} // namespace skr