#pragma once
#include <SkrBase/type_info.hpp>
#include <SkrCore/serialize/fwd.hpp>
#include <SkrBase/types.h>
#include <SkrCore/log.hpp>
#include <SkrCore/serialize/archive_error_tracker.hpp>
#include <SkrContainersDef/string.hpp>
#include <SkrContainersDef/vector.hpp>
#include <source_location>

//! Archie 基本设计:
//! - Archive 提供了以下功能
//!   - 序列化器类型，是否是结构化的：
//!   - 常用的序列化选项
//!   - 错误追踪和日志
//!   - 基础的结构化 API
//! - ArchiveWrite 提供了以下功能：
//!   - primitive/str_view：带类型的读取
//!   - bytes/bits：非结构化的读取
//!   - has_value：结构化辅助，用于判断现在是否有用于消耗的 value，即读取调用是否安全
//!   - array_size_structured：结构化辅助，用于获取当前所处数组的大小
//!   - array_size：用于抹平结构化与非结构化的数组大小写入
//!   - adjust_array_size：用于调整已知长度的数组大小，并在不匹配时输出日志
//! - ArchiveRead 提供了以下功能：
//!   - primitive/str_view：带类型的写入
//!   - bytes/bits：非结构化的写入
//!   - array_size：用于抹平结构化与非结构化的数组大小读取
//!   - key_copy：结构化辅助，指示 key 的内存需要拷贝，用于非字面量的 key() 调用
//! - Read/Write 进行分离的理由是，两者 API 与实现逻辑都存在较大差异
//! - Structured/Unstructured 集中化的理由是，在书写复杂的高层逻辑时，可以混合使用两者 API，避免大量的逻辑重复
//!
//! Archive 错误处理：
//! - 如果发生了错误，check_point() 会返回 false，可以使用 SKR_FAST_CHECK 宏来简化错误处理
//! - 这样做的好处是，用户不必到处关心函数返回值，只需要关键点调用 check_point() 即可
//! - 同时，check_point() 还提供了错误堆栈追踪功能，方便定位问题
//! - error() 输出并触发错误，使用 fmt 语法
//! - warn() 输出警告不触发错误，使用 fmt 语法
//! - 所有 API 都会返回一个 bool 表示当前是否发生错误(除了 has_value 等状态获取 API)，以方便你书写 SKR_FAST_CHECK
//!
//! Structured/Non-structured API 设计：
//! - 当 Archive 是 Non-structured 时，所有结构化 API 都会被自动遮罩，以此减少你对 Archive 类型的关心
//! - Structured API：
//!   - key()：用于指定下一个 value 的 key
//!   - key_literal()：[WriterOnly] 用于字面量优化，减少内存分配
//!   - begin_object()/end_object()：用于标记一个对象的开始与结束
//!   - begin_array()/end_array()：用于标记一个数组的开始与结束
//! - Non-structured API：
//!   - bytes()：写字节
//!   - bits()：写 bit 位，通常用于带压缩的二进制序列化
//! - Shared API：
//!   - primitive()：基元类型
//!   - str_view()/str()：字符串
//! - extended API:
//!   - value()：对标准 Serialize<T>::read/write 的包装，用于减少你的代码量
//!   - key_value()：用于便利的表达键值对，在 Non-structured Archive 中 key 会被忽略
//!   - key_value_required()：[ReaderOnly] 用于表达必需的键值对，强制将 key 缺失提升位错误
//!   - key_value_literal()：[WriterOnly] 用于字面量优化，减少内存分配
//!
//! 关于 baseXX 编码：
//! - baseXX 编码一般是就地开 buffer 实现，如果期望相关优化由 archive 提供，可以考虑添加

//! 如果希望禁用 traceback 以期望减少二进制体积，请启用 DISABLE_SKR_ARCHIVE_TRACEBACK
// #define DISABLE_SKR_ARCHIVE_TRACEBACK

// Archive
namespace skr
{
enum class EArchivePrimitiveType : uint8_t
{
    // boolean
    Bool,

    // integer
    Int8,
    Int16,
    Int32,
    Int64,

    // unsigned integer
    UInt8,
    UInt16,
    UInt32,
    UInt64,

    // floating point
    Float,
    Double,

    // string will use standard api
};
enum class EArchiveLogBehaviour : uint8_t
{
    Silence, // do nothing
    Warn,    // log warning
    Error,   // log error
};

struct Archive
{
    // current location helper
    static std::source_location current_location(
#ifndef DISABLE_SKR_ARCHIVE_TRACEBACK
        std::source_location location = std::source_location::current()
#else
        std::source_location location = {}
#endif
    );

    // ctor & dtor
    Archive();
    virtual ~Archive();

    //========================== RAII scope ==========================
    //=== RAII scope for safe when work with SKR_FAST_CHECK        ===
    //================================================================
    struct ArrayScope
    {
        SKR_DELETE_COPY_MOVE(ArrayScope);
        ArrayScope(Archive& ar, std::source_location location = current_location());
        ~ArrayScope();
        bool is_success() const;

    private:
        Archive& _ar;
        std::source_location _location;
        bool _is_success;
    };
    struct ObjectScope
    {
        SKR_DELETE_COPY_MOVE(ObjectScope);
        ObjectScope(Archive& ar, std::source_location location = current_location());
        ~ObjectScope();
        bool is_success() const;

    private:
        Archive& _ar;
        std::source_location _location;
        bool _is_success;
    };

    //========================= archive info =========================
    //=== archive info effects how serializer works                ===
    //================================================================

    // if true, serializer must use structured api:
    // ```cpp
    //   ArchiveWrite::ObjectScope _{w};
    //   w.key("field1"); w.value(&field1);
    //   w.key("field2"); w.value(&field2);
    // ```
    // if false, serializer use bytes()/bits() or value() directly:
    // ```cpp
    //   w.value(&field1);
    //   w.value(&field2);
    // ```
    bool is_structured() const;

    // used for cast to ArchiveWrite/ArchiveRead
    // sometimes you may want to use Archive directly, these apis will be useful
    bool is_write() const;
    bool is_read() const;
    ArchiveWrite* try_as_write();
    ArchiveRead* try_as_read();

    //======================= archive options =======================
    //=== options for some hot archive behaviour                  ===
    //===============================================================

    // if true, enum will be serialized as string if possible
    // if false, enum will be serialized as it's underlying integer type
    bool enable_string_enum() const;
    void set_enable_string_enum(bool v);

    // if true, GUID will be serialized as string if possible
    // if false, GUID will be serialized as binary data
    bool enable_string_guid() const;
    void set_enable_string_guid(bool v);

    // if true, MD5 will be serialized as string if possible
    // if false, MD5 will be serialized as binary data
    bool enable_string_md5() const;
    void set_enable_string_md5(bool v);

    // if true, SHA256 will be serialized as string if possible
    // if false, SHA256 will be serialized as binary data
    bool enable_string_sha256() const;
    void set_enable_string_sha256(bool v);

    //====================== archive log behaviour =====================
    //=== control behavior when encountering some non-fault problems ===
    //==================================================================
    void when_lost_key(EArchiveLogBehaviour behaviour);
    EArchiveLogBehaviour get_behaviour_when_lost_key() const;
    void when_array_size_mismatch(EArchiveLogBehaviour behaviour);
    EArchiveLogBehaviour get_behaviour_when_array_size_mismatch() const;
    void when_primitive_type_mismatch(EArchiveLogBehaviour behaviour);
    EArchiveLogBehaviour get_behaviour_when_primitive_type_mismatch() const;
    void silence_all_problems();
    void warn_all_problems();
    void error_all_problems();

    //======================== structured API =======================
    //=== Basic structured API                                    ===
    //===============================================================
    bool key(StringView key, std::source_location location = current_location());
    bool begin_array(std::source_location location = current_location());
    bool end_array(std::source_location location = current_location());
    bool begin_object(std::source_location location = current_location());
    bool end_object(std::source_location location = current_location());

    //===================== archive error track =====================
    //=== track error and dump callstack                          ===
    //===============================================================
    // checkpoint for error tracking
    bool checkpoint(std::source_location location = current_location());

    // log api
    template <typename... Args>
    void log_by_behaviour(EArchiveLogBehaviour behaviour, StringView fmt, Args&&... args);
    template <typename... Args>
    void warn(StringView fmt, Args&&... args);
    template <typename... Args>
    void error(StringView fmt, Args&&... args);

    // dump & error handling
    const ArchiveErrorTracker& error_tracker() const;
    bool is_succeeded() const;
    bool is_failed() const;
    bool is_error_handled() const;
    void mark_error_handled() const;
    bool assume_succeeded_or_dump_error() const;

protected:
    // helpers
    static uint64_t _primitive_size(EArchivePrimitiveType type);

    //==> Archive APIs
    // basic structured API
    virtual void impl_key(StringView key) = 0;
    virtual void impl_begin_array() = 0;
    virtual void impl_end_array() = 0;
    virtual void impl_begin_object() = 0;
    virtual void impl_end_object() = 0;
    //==> Archive APIs end
protected:
    // archive info, setted by derived class
    bool _is_structured : 1 = false;
    bool _is_write : 1 = false; // true: ArchiveWrite, false: ArchiveRead

    // archive options
    bool _enable_string_enum : 1 = false;
    bool _enable_string_guid : 1 = false;
    bool _enable_string_md5 : 1 = false;
    bool _enable_string_sha256 : 1 = false;

    // behaviour
    EArchiveLogBehaviour _behaviour_lost_key = EArchiveLogBehaviour::Warn;
    EArchiveLogBehaviour _behaviour_array_size_mismatch = EArchiveLogBehaviour::Warn;
    EArchiveLogBehaviour _behaviour_primitive_type_mismatch = EArchiveLogBehaviour::Warn;

    // error tracker
    ArchiveErrorTracker _error_tracker;
};

//! NOTE. structured api must be used in structured archive
//!       non-structured api must be used in unstructured archive
//!       typed data api can be used in both archive
struct ArchiveRead : Archive
{
    // ctor & dtor
    ArchiveRead();
    virtual ~ArchiveRead();

    // structured api
    bool key_required(StringView key, std::source_location location = current_location());
    bool has_value();
    bool array_size_structured(uint64_t& out_size, std::source_location location = current_location());
    bool adjust_array_size(uint64_t actual, uint64_t expected, uint64_t& out_adjusted, std::source_location location = current_location());
    template <typename T>
    bool array_size(T& out_size, std::source_location location = current_location());

    // typed data, shared api
    bool primitive(EArchivePrimitiveType type, void* data, std::source_location location = current_location());
    //! view must be valid before any other archive operation, except checkpoint()
    bool str_view(StringView& out_view, std::source_location location = current_location());

    // non-structured api
    bool bytes(void* data, uint64_t size, std::source_location location = current_location());
    bool bits(void* data, uint64_t size, uint64_t offset, std::source_location location = current_location());

    // serialize helper
    template <concepts::HasSerdeRead T>
    bool value(T& v, std::source_location location = current_location());
    template <concepts::HasSerdeRead T>
    bool key_value(StringView key, T& v, std::source_location location = current_location());
    template <concepts::HasSerdeRead T>
    bool key_value_required(StringView key, T& v, std::source_location location = current_location());

protected:
    //==> ArchiveRead APIs
    // structured api
    virtual bool impl_has_value() = 0;
    virtual uint64_t impl_array_size() = 0;

    // typed data
    virtual void impl_primitive(EArchivePrimitiveType type, void* data) = 0;
    //! view must be valid before any other archive operation, except checkpoint()
    virtual StringView impl_str_view() = 0;

    // non-structured api
    virtual void impl_bytes(void* data, uint64_t size) = 0;
    virtual void impl_bits(void* data, uint64_t size, uint64_t offset) = 0;
    //==> ArchiveRead APIs end
};

//! NOTE. structured api must be used in structured archive
//!       non-structured api must be used in unstructured archive
//!       typed data api can be used in both archive
struct ArchiveWrite : Archive
{
    // ctor & dtor
    ArchiveWrite();
    virtual ~ArchiveWrite();

    // structured api
    bool key_copy(StringView key, std::source_location location = current_location());
    template <typename T>
    bool array_size(T size, std::source_location location = current_location());

    // typed data, shared api
    bool primitive(EArchivePrimitiveType type, const void* data, std::source_location location = current_location());
    bool str(StringView str, std::source_location location = current_location());

    // non-structured api
    bool bytes(const void* data, uint64_t size, std::source_location location = current_location());
    bool bits(const void* data, uint64_t size, uint64_t offset = 0, std::source_location location = current_location());

    // serialize helper
    template <concepts::HasSerdeWrite T>
    bool value(const T& v, std::source_location location = current_location());
    template <concepts::HasSerdeWrite T>
    bool key_value(StringView key, const T& v, std::source_location location = current_location());
    template <concepts::HasSerdeWrite T>
    bool key_value_copy(StringView key, const T& v, std::source_location location = current_location());

protected:
    //==> ArchiveWrite APIs
    // structured api
    virtual void impl_key_copy(StringView key) = 0;

    // typed data, shared api
    virtual void impl_primitive(EArchivePrimitiveType type, const void* data) = 0;
    virtual void impl_str(StringView str) = 0;

    // non-structured api
    virtual void impl_bytes(const void* data, uint64_t size) = 0;
    virtual void impl_bits(const void* data, uint64_t size, uint64_t offset) = 0;
    //==> ArchiveWrite APIs end
};
} // namespace skr

// type ids
SKR_TYPE_INFO(skr::Archive, "d1fa6490-f13a-4433-9f26-69c6a4dea336");
SKR_TYPE_INFO(skr::ArchiveRead, "8e51fc2b-fec6-4829-a006-582caf6fc073");
SKR_TYPE_INFO(skr::ArchiveWrite, "87919882-0cc7-48c0-a723-6f857930144a");

// Archive impl
namespace skr
{
// helpers
inline uint64_t Archive::_primitive_size(EArchivePrimitiveType type)
{
    // clang-format off
        switch (type)
        {
            case EArchivePrimitiveType::Bool:    return sizeof(bool);
            
            case EArchivePrimitiveType::Int8:    return sizeof(int8_t);
            case EArchivePrimitiveType::Int16:   return sizeof(int16_t);
            case EArchivePrimitiveType::Int32:   return sizeof(int32_t);
            case EArchivePrimitiveType::Int64:   return sizeof(int64_t);
            
            case EArchivePrimitiveType::UInt8:   return sizeof(uint8_t);
            case EArchivePrimitiveType::UInt16:  return sizeof(uint16_t);
            case EArchivePrimitiveType::UInt32:  return sizeof(uint32_t);
            case EArchivePrimitiveType::UInt64:  return sizeof(uint64_t);
            
            case EArchivePrimitiveType::Float:   return sizeof(float);
            case EArchivePrimitiveType::Double:  return sizeof(double);
            
            default: SKR_UNREACHABLE_CODE(); return 0;
        }
    // clang-format on
}

// current location helper
inline std::source_location Archive::current_location(
    std::source_location location
)
{
    return location;
}

// ctor & dtor
inline Archive::Archive()
{
}
inline Archive::~Archive()
{
}

// array scope
inline Archive::ArrayScope::ArrayScope(Archive& ar, std::source_location location)
    : _ar(ar)
    , _location(location)
    , _is_success(true)
{
    _is_success = _is_success && _ar.begin_array(_location);
}
inline Archive::ArrayScope::~ArrayScope()
{
    _is_success = _is_success && _ar.end_array(_location);
}
inline bool Archive::ArrayScope::is_success() const
{
    return _is_success;
}

// object scope
inline Archive::ObjectScope::ObjectScope(Archive& ar, std::source_location location)
    : _ar(ar)
    , _location(location)
    , _is_success(true)
{
    _is_success = _is_success && _ar.begin_object(_location);
}
inline Archive::ObjectScope::~ObjectScope()
{
    _is_success = _is_success && _ar.end_object(_location);
}
inline bool Archive::ObjectScope::is_success() const
{
    return _is_success;
}

// archive info
inline bool Archive::is_structured() const { return _is_structured; }
inline bool Archive::is_write() const
{
    return _is_write;
}
inline bool Archive::is_read() const
{
    return !_is_write;
}
inline ArchiveWrite* Archive::try_as_write()
{
    if (is_write())
        return static_cast<ArchiveWrite*>(this);
    return nullptr;
}
inline ArchiveRead* Archive::try_as_read()
{
    if (is_read())
        return static_cast<ArchiveRead*>(this);
    return nullptr;
}

// archive options
inline bool Archive::enable_string_enum() const { return _enable_string_enum; }
inline void Archive::set_enable_string_enum(bool v) { _enable_string_enum = v; }
inline bool Archive::enable_string_guid() const { return _enable_string_guid; }
inline void Archive::set_enable_string_guid(bool v) { _enable_string_guid = v; }
inline bool Archive::enable_string_md5() const { return _enable_string_md5; }
inline void Archive::set_enable_string_md5(bool v) { _enable_string_md5 = v; }
inline bool Archive::enable_string_sha256() const { return _enable_string_sha256; }
inline void Archive::set_enable_string_sha256(bool v) { _enable_string_sha256 = v; }

// archive log behaviour
inline void Archive::when_lost_key(EArchiveLogBehaviour behaviour) { _behaviour_lost_key = behaviour; }
inline EArchiveLogBehaviour Archive::get_behaviour_when_lost_key() const { return _behaviour_lost_key; }
inline void Archive::when_array_size_mismatch(EArchiveLogBehaviour behaviour) { _behaviour_array_size_mismatch = behaviour; }
inline EArchiveLogBehaviour Archive::get_behaviour_when_array_size_mismatch() const { return _behaviour_array_size_mismatch; }
inline void Archive::when_primitive_type_mismatch(EArchiveLogBehaviour behaviour) { _behaviour_primitive_type_mismatch = behaviour; }
inline EArchiveLogBehaviour Archive::get_behaviour_when_primitive_type_mismatch() const { return _behaviour_primitive_type_mismatch; }
inline void Archive::silence_all_problems()
{
    _behaviour_lost_key = EArchiveLogBehaviour::Silence;
    _behaviour_array_size_mismatch = EArchiveLogBehaviour::Silence;
    _behaviour_primitive_type_mismatch = EArchiveLogBehaviour::Silence;
}
inline void Archive::warn_all_problems()
{
    _behaviour_lost_key = EArchiveLogBehaviour::Warn;
    _behaviour_array_size_mismatch = EArchiveLogBehaviour::Warn;
    _behaviour_primitive_type_mismatch = EArchiveLogBehaviour::Warn;
}
inline void Archive::error_all_problems()
{
    _behaviour_lost_key = EArchiveLogBehaviour::Error;
    _behaviour_array_size_mismatch = EArchiveLogBehaviour::Error;
    _behaviour_primitive_type_mismatch = EArchiveLogBehaviour::Error;
}

// basic structured API
inline bool Archive::key(StringView key, std::source_location location)
{
    _error_tracker.key(key);
    if (is_structured())
        impl_key(key);
    return checkpoint(location);
}
inline bool Archive::begin_array(std::source_location location)
{
    _error_tracker.begin_array();
    if (is_structured())
        impl_begin_array();
    return checkpoint(location);
}
inline bool Archive::end_array(std::source_location location)
{
    _error_tracker.end_array();
    if (is_structured())
        impl_end_array();
    return checkpoint(location);
}
inline bool Archive::begin_object(std::source_location location)
{
    _error_tracker.begin_object();
    if (is_structured())
        impl_begin_object();
    return checkpoint(location);
}
inline bool Archive::end_object(std::source_location location)
{
    _error_tracker.end_object();
    if (is_structured())
        impl_end_object();
    return checkpoint(location);
}

// check point for error tracking
inline bool Archive::checkpoint(std::source_location location)
{
    return _error_tracker.checkpoint(location);
}

// log api
template <typename... Args>
inline void Archive::log_by_behaviour(EArchiveLogBehaviour behaviour, StringView fmt, Args&&... args)
{
    switch (behaviour)
    {
    case EArchiveLogBehaviour::Silence:
        break;
    case EArchiveLogBehaviour::Warn:
        warn(fmt, std::forward<Args>(args)...);
        break;
    case EArchiveLogBehaviour::Error:
        error(fmt, std::forward<Args>(args)...);
        break;
    default:
        SKR_UNREACHABLE_CODE();
    }
}
template <typename... Args>
inline void Archive::warn(StringView fmt, Args&&... args)
{
    _error_tracker.warn(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
inline void Archive::error(StringView fmt, Args&&... args)
{
    _error_tracker.error(fmt, std::forward<Args>(args)...);
}

// error handling
inline const ArchiveErrorTracker& Archive::error_tracker() const { return _error_tracker; }
inline bool Archive::is_succeeded() const { return !_error_tracker.any_error(); }
inline bool Archive::is_failed() const { return _error_tracker.any_error(); }
inline bool Archive::is_error_handled() const
{
    return _error_tracker.is_error_handled();
}
inline void Archive::mark_error_handled() const
{
    _error_tracker.mark_error_handled();
}
inline bool Archive::assume_succeeded_or_dump_error() const
{
    return _error_tracker.assume_succeeded_or_dump_error();
}
} // namespace skr

// ArchiveRead impl
namespace skr
{
// ctor & dtor
inline ArchiveRead::ArchiveRead()
{
    _is_write = false;
}
inline ArchiveRead::~ArchiveRead()
{
}

// structured api
inline bool ArchiveRead::key_required(StringView key, std::source_location location)
{
    if (is_structured())
    {
        SKR_FAST_CHECK(this->key(key, location), false);
        if (!this->has_value()) [[unlikely]]
        { // force error, not care about behaviour settings
            error(u8"required key '{}' not found", key);
        }
    }
    return checkpoint(location);
}
inline bool ArchiveRead::has_value()
{
    SKR_ASSERT(is_structured());
    return impl_has_value();
}
inline bool ArchiveRead::array_size_structured(uint64_t& out_size, std::source_location location)
{
    SKR_ASSERT(is_structured());
    out_size = impl_array_size();
    return checkpoint(location);
}
inline bool ArchiveRead::adjust_array_size(uint64_t actual, uint64_t expected, uint64_t& out_adjusted, std::source_location location)
{
    if (expected < actual) [[unlikely]]
    {
        log_by_behaviour(
            get_behaviour_when_array_size_mismatch(),
            u8"got too many elements ({} expected, given {})",
            expected,
            actual
        );
        out_adjusted = expected;
    }
    else if (actual < expected) [[unlikely]]
    {
        log_by_behaviour(
            get_behaviour_when_array_size_mismatch(),
            u8"got too few elements ({} expected, given {})",
            expected,
            actual
        );
        out_adjusted = actual;
    }
    else
    {
        out_adjusted = expected;
    }
    return checkpoint(location);
}
template <typename T>
inline bool ArchiveRead::array_size(T& out_size, std::source_location location)
{
    static_assert(std::is_integral_v<T>, "array_size only supports integral type");
    if (is_structured())
    {
        out_size = impl_array_size();
    }
    else
    {
        impl_bytes(&out_size, sizeof(T));
    }
    return checkpoint(location);
}

// typed data
inline bool ArchiveRead::primitive(EArchivePrimitiveType type, void* data, std::source_location location)
{
    impl_primitive(type, data);
    return checkpoint(location);
}
//! view must be valid before any other archive operation, except checkpoint()
inline bool ArchiveRead::str_view(StringView& out_view, std::source_location location)
{
    out_view = impl_str_view();
    return checkpoint(location);
}

// binary data support
inline bool ArchiveRead::bytes(void* data, uint64_t size, std::source_location location)
{
    SKR_ASSERT(!is_structured());
    impl_bytes(data, size);
    return checkpoint(location);
}
inline bool ArchiveRead::bits(void* data, uint64_t size, uint64_t offset, std::source_location location)
{
    SKR_ASSERT(!is_structured());
    impl_bits(data, size, offset);
    return checkpoint(location);
}

// serialize helper
template <concepts::HasSerdeRead T>
inline bool ArchiveRead::value(T& v, std::source_location location)
{
    Serialize<T>::read(*this, v);
    return checkpoint(location);
}
template <concepts::HasSerdeRead T>
inline bool ArchiveRead::key_value(StringView key, T& v, std::source_location location)
{
    if (is_structured())
    {
        SKR_FAST_CHECK(this->key(key, location), false);
        if (this->has_value())
        {
            Serialize<T>::read(*this, v);
        }
        else
        {
            log_by_behaviour(_behaviour_lost_key, u8"key '{}' not found", key);
        }
    }
    else
    {
        Serialize<T>::read(*this, v);
    }

    return checkpoint(location);
}
template <concepts::HasSerdeRead T>
inline bool ArchiveRead::key_value_required(StringView key, T& v, std::source_location location)
{
    if (is_structured())
    {
        SKR_FAST_CHECK(this->key(key, location), false);
        if (this->has_value()) [[likely]]
        {
            Serialize<T>::read(*this, v);
        }
        else
        { // force error, not care about behaviour settings
            error(u8"required key '{}' not found", key);
        }
    }
    else
    {
        Serialize<T>::read(*this, v);
    }

    return checkpoint(location);
}
} // namespace skr

// ArchiveWrite impl
namespace skr
{
// ctor & dtor
inline ArchiveWrite::ArchiveWrite()
{
    _is_write = true;
}
inline ArchiveWrite::~ArchiveWrite()
{
}

// structured api
inline bool ArchiveWrite::key_copy(StringView key, std::source_location location)
{
    _error_tracker.key(key);
    if (is_structured())
        impl_key_copy(key);
    return checkpoint(location);
}
template <typename T>
inline bool ArchiveWrite::array_size(T size, std::source_location location)
{
    static_assert(std::is_integral_v<T>, "array_size only supports integral type");
    if (is_structured())
    { // do noting, structured archive should know the size by itself
    }
    else
    {
        impl_bytes(&size, sizeof(T));
    }
    return checkpoint(location);
}

// typed data
inline bool ArchiveWrite::primitive(EArchivePrimitiveType type, const void* data, std::source_location location)
{
    impl_primitive(type, data);
    return checkpoint(location);
}
inline bool ArchiveWrite::str(StringView str, std::source_location location)
{
    impl_str(str);
    return checkpoint(location);
}

// binary data support
inline bool ArchiveWrite::bytes(const void* data, uint64_t size, std::source_location location)
{
    SKR_ASSERT(!is_structured());
    impl_bytes(data, size);
    return checkpoint(location);
}
inline bool ArchiveWrite::bits(const void* data, uint64_t size, uint64_t offset, std::source_location location)
{
    SKR_ASSERT(!is_structured());
    impl_bits(data, size, offset);
    return checkpoint(location);
}

// serialize helper
template <concepts::HasSerdeWrite T>
inline bool ArchiveWrite::value(const T& v, std::source_location location)
{
    Serialize<T>::write(*this, v);
    return checkpoint(location);
}
template <concepts::HasSerdeWrite T>
inline bool ArchiveWrite::key_value(StringView key, const T& v, std::source_location location)
{
    if (is_structured())
    {
        SKR_FAST_CHECK(this->key(key, location), false);
        Serialize<T>::write(*this, v);
    }
    else
    {
        Serialize<T>::write(*this, v);
    }
    return checkpoint(location);
}
template <concepts::HasSerdeWrite T>
inline bool ArchiveWrite::key_value_copy(StringView key, const T& v, std::source_location location)
{
    if (is_structured())
    {
        this->key_copy(key);
        Serialize<T>::write(*this, v);
    }
    else
    {
        Serialize<T>::write(*this, v);
    }
    return checkpoint(location);
}

} // namespace skr