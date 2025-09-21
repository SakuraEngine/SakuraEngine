#include <SkrCore/serialize/json_archive.hpp>
#include <yyjson/yyjson.h>

// yyjson alc
namespace skr
{
static constexpr const char* kYYJsonPoolName = "yyjson";
void* skr_yyjson_malloc(void* ctx, size_t size)
{
#if defined(TRACY_TRACE_ALLOCATION)
    SkrCZoneNCS(z, "yyjson::allocate", SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
    void* p = sakura_malloc_alignedN(size, alignof(size_t), kYYJsonPoolName);
    SkrCZoneEnd(z);
    return p;
#else
    return sakura_malloc_aligned(size, alignof(size_t));
#endif
}
void* skr_yyjson_realloc(void* ctx, void* ptr, size_t old_size, size_t size)
{
#if defined(TRACY_TRACE_ALLOCATION)
    SkrCZoneNCS(z, "v8::realloc", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
    void* new_mem = sakura_realloc_alignedN(ptr, size, alignof(size_t), kYYJsonPoolName);
    SkrCZoneEnd(z);
    return new_mem;
#else
    return sakura_realloc_aligned(ptr, size, alignof(size_t));
#endif
}
void skr_yyjson_free(void* ctx, void* ptr)
{
    if (ptr) [[likely]]
    {
#if defined(TRACY_TRACE_ALLOCATION)
        SkrCZoneNCS(z, "containers::free", SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
        sakura_free_alignedN(ptr, alignof(size_t), kYYJsonPoolName);
        SkrCZoneEnd(z);
#else
        sakura_free_aligned(ptr, alignof(size_t));
#endif
    }
}

inline static yyjson_alc _make_yyjson_alc()
{
    yyjson_alc alc;
    alc.malloc = skr_yyjson_malloc;
    alc.realloc = skr_yyjson_realloc;
    alc.free = skr_yyjson_free;
    alc.ctx = nullptr;
    return alc;
}
} // namespace skr

// convert & helpers
namespace skr
{
static_assert(kJsonInSituPaddingCount == YYJSON_PADDING_SIZE, "kJsonInSituPaddingCount must equal to YYJSON_PADDING_SIZE");

// json iterator array
inline static yyjson_arr_iter* _to_yyjson(JsonIterArray* iter)
{
    return reinterpret_cast<yyjson_arr_iter*>(iter);
}
inline static JsonIterArray* _from_yyjson(yyjson_arr_iter* yy_iter)
{
    return reinterpret_cast<JsonIterArray*>(yy_iter);
}

// json iterator object
inline static yyjson_obj_iter* _to_yyjson(JsonIterObject* iter)
{
    return reinterpret_cast<yyjson_obj_iter*>(iter);
}
inline static JsonIterObject* _from_yyjson(yyjson_obj_iter* yy_iter)
{
    return reinterpret_cast<JsonIterObject*>(yy_iter);
}

// json iterator array mut
inline static yyjson_mut_arr_iter* _to_yyjson(JsonIterArrayMut* iter)
{
    return reinterpret_cast<yyjson_mut_arr_iter*>(iter);
}
inline static JsonIterArrayMut* _from_yyjson(yyjson_mut_arr_iter* yy_iter)
{
    return reinterpret_cast<JsonIterArrayMut*>(yy_iter);
}

// json iterator object mut
inline static yyjson_mut_obj_iter* _to_yyjson(JsonIterObjectMut* iter)
{
    return reinterpret_cast<yyjson_mut_obj_iter*>(iter);
}
inline static JsonIterObjectMut* _from_yyjson(yyjson_mut_obj_iter* yy_iter)
{
    return reinterpret_cast<JsonIterObjectMut*>(yy_iter);
}

// json value
inline static yyjson_val* _to_yyjson(JsonValue* val)
{
    return reinterpret_cast<yyjson_val*>(val);
}
inline static JsonValue* _from_yyjson(yyjson_val* yy_val)
{
    return reinterpret_cast<JsonValue*>(yy_val);
}

// json document
inline static yyjson_doc* _to_yyjson(JsonDocument* doc)
{
    return reinterpret_cast<yyjson_doc*>(doc);
}
inline static JsonDocument* _from_yyjson(yyjson_doc* yy_doc)
{
    return reinterpret_cast<JsonDocument*>(yy_doc);
}

// json value mut
inline static yyjson_mut_val* _to_yyjson(JsonValueMut* val)
{
    return reinterpret_cast<yyjson_mut_val*>(val);
}
inline static JsonValueMut* _from_yyjson(yyjson_mut_val* yy_val)
{
    return reinterpret_cast<JsonValueMut*>(yy_val);
}

// json document mut
inline static yyjson_mut_doc* _to_yyjson(JsonDocumentMut* doc)
{
    return reinterpret_cast<yyjson_mut_doc*>(doc);
}
inline static JsonDocumentMut* _from_yyjson(yyjson_mut_doc* yy_doc)
{
    return reinterpret_cast<JsonDocumentMut*>(yy_doc);
}

// read flags
inline static yyjson_read_flag _to_yyjson(EJsonReadFlags flags)
{
    return static_cast<yyjson_read_flag>(static_cast<uint32_t>(flags));
}
inline static EJsonReadFlags _from_yyjson_read_flag(yyjson_read_flag yy_flags)
{
    return static_cast<EJsonReadFlags>(static_cast<uint32_t>(yy_flags));
}

// write flags
inline static yyjson_write_flag _to_yyjson(EJsonWriteFlags flags)
{
    return static_cast<yyjson_write_flag>(static_cast<uint32_t>(flags));
}
inline static EJsonWriteFlags _from_yyjson_write_flag(yyjson_write_flag yy_flags)
{
    return static_cast<EJsonWriteFlags>(static_cast<uint32_t>(yy_flags));
}

// make error result
inline static JsonReadResult _make_error_result(
    const yyjson_read_err& err,
    const char* str,
    size_t len
)
{
    // solve error location
    size_t line = 0, col = 0, chr = 0;
    if (str)
    {
        yyjson_locate_pos(
            str, len, err.pos,
            &line, &col, &chr
        );
    }

    return {
        StringView{ reinterpret_cast<const skr_char8*>(err.msg) },
        line,
        col
    };
}
inline static JsonWriteResult _make_error_result(
    const yyjson_write_err& err
)
{
    return { StringView{ reinterpret_cast<const skr_char8*>(err.msg) } };
}
} // namespace skr

// json iterator array
namespace skr
{
static_assert(sizeof(JsonIterArray) >= sizeof(yyjson_arr_iter), "JsonIterArray size mismatch");
static_assert(alignof(JsonIterArray) >= alignof(yyjson_arr_iter), "JsonIterArray alignment mismatch");

// has next value
bool JsonIterArray::has_next()
{
    auto yy_iter = _to_yyjson(this);
    return yyjson_arr_iter_has_next(yy_iter);
}

// return next value, if not has next, return nullptr
JsonValue* JsonIterArray::move_next()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_val = yyjson_arr_iter_next(yy_iter);
    return _from_yyjson(yy_val);
}

} // namespace skr

// json iterator object
namespace skr
{
static_assert(sizeof(JsonIterObject) >= sizeof(yyjson_obj_iter), "JsonIterObject size mismatch");
static_assert(alignof(JsonIterObject) >= alignof(yyjson_obj_iter), "JsonIterObject alignment mismatch");

// has next key-value pair
bool JsonIterObject::has_next()
{
    auto yy_iter = _to_yyjson(this);
    return yyjson_obj_iter_has_next(yy_iter);
}

// return next key, if not has next, return nullptr
// use value_of to get it's value
JsonValue* JsonIterObject::move_next()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_key = yyjson_obj_iter_next(yy_iter);
    return _from_yyjson(yy_key);
}

// return next found value of key, if not found, return nullptr
JsonValue* JsonIterObject::move_next_key(StringView key)
{
    auto yy_iter = _to_yyjson(this);
    auto yy_key_str = reinterpret_cast<const char*>(key.data());
    auto yy_key_len = static_cast<size_t>(key.size());
    auto yy_val = yyjson_obj_iter_getn(yy_iter, yy_key_str, yy_key_len);
    return _from_yyjson(yy_val);
}

// get bounded value of key
JsonValue* JsonIterObject::value_of(JsonValue* key)
{
    auto yy_key = _to_yyjson(key);
    auto yy_val = yyjson_obj_iter_get_val(yy_key);
    return _from_yyjson(yy_val);
}
} // namespace skr

// json iterator array mut
namespace skr
{
static_assert(sizeof(JsonIterArrayMut) >= sizeof(yyjson_mut_arr_iter), "JsonIterArrayMut size mismatch");
static_assert(alignof(JsonIterArrayMut) >= alignof(yyjson_mut_arr_iter), "JsonIterArrayMut alignment mismatch");

// has next value
bool JsonIterArrayMut::has_next()
{
    auto yy_iter = _to_yyjson(this);
    return yyjson_mut_arr_iter_has_next(yy_iter);
}

// return next value, if not has next, return nullptr
JsonValueMut* JsonIterArrayMut::move_next()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_val = yyjson_mut_arr_iter_next(yy_iter);
    return _from_yyjson(yy_val);
}

// remove and return current value
JsonValueMut* JsonIterArrayMut::remove_current()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_val = yyjson_mut_arr_iter_remove(yy_iter);
    return _from_yyjson(yy_val);
}
} // namespace skr

// json iterator object mut
namespace skr
{
static_assert(sizeof(JsonIterObjectMut) >= sizeof(yyjson_mut_obj_iter), "JsonIterObjectMut size mismatch");
static_assert(alignof(JsonIterObjectMut) >= alignof(yyjson_mut_obj_iter), "JsonIterObjectMut alignment mismatch");

// has next key-value pair
bool JsonIterObjectMut::has_next()
{
    auto yy_iter = _to_yyjson(this);
    return yyjson_mut_obj_iter_has_next(yy_iter);
}

// return next key, if not has next, return nullptr
// use value_of to get it's value
JsonValueMut* JsonIterObjectMut::move_next()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_key = yyjson_mut_obj_iter_next(yy_iter);
    return _from_yyjson(yy_key);
}

// return next found value of key, if not found, return nullptr
JsonValueMut* JsonIterObjectMut::move_next_key(StringView key)
{
    auto yy_iter = _to_yyjson(this);
    auto yy_key_str = reinterpret_cast<const char*>(key.data());
    auto yy_key_len = static_cast<size_t>(key.size());
    auto yy_val = yyjson_mut_obj_iter_getn(yy_iter, yy_key_str, yy_key_len);
    return _from_yyjson(yy_val);
}

// get bounded value of key
JsonValueMut* JsonIterObjectMut::value_of(JsonValueMut* key)
{
    auto yy_key = _to_yyjson(key);
    auto yy_val = yyjson_mut_obj_iter_get_val(yy_key);
    return _from_yyjson(yy_val);
}

// remove and return current key-value pair's value
JsonValueMut* JsonIterObjectMut::remove_current()
{
    auto yy_iter = _to_yyjson(this);
    auto yy_val = yyjson_mut_obj_iter_remove(yy_iter);
    return _from_yyjson(yy_val);
}
} // namespace skr

// json document
namespace skr
{

// read from buffer, null for failed
JsonReadResult JsonDocument::ReadBuffer(const void* buffer, uint64_t len, EJsonReadFlags flags)
{
    // read
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_buffer = reinterpret_cast<char*>(const_cast<void*>(buffer));
    auto yy_len = static_cast<size_t>(len);
    yyjson_read_err out_err;
    auto yy_doc = yyjson_read_opts(
        yy_buffer,
        yy_len,
        yy_flags,
        &yy_alc,
        &out_err
    );

    // handle error
    if (!yy_doc) [[unlikely]]
    {
        return _make_error_result(
            out_err,
            yy_buffer,
            yy_len
        );
    }
    else
    {
        return _from_yyjson(yy_doc);
    }
}

// read from file, null for failed
JsonReadResult JsonDocument::ReadFile(const Path& path, EJsonReadFlags flags)
{
    // read
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_path = reinterpret_cast<const char*>(path.c_str());
    yyjson_read_err out_err;
    auto yy_doc = yyjson_read_file(
        yy_path,
        yy_flags,
        &yy_alc,
        &out_err
    );

    // handle error
    if (!yy_doc) [[unlikely]]
    {
        return _make_error_result(
            out_err,
            nullptr,
            0
        );
    }
    else
    {
        return _from_yyjson(yy_doc);
    }
}

// destroy a document
void JsonDocument::Destroy(JsonDocument* doc)
{
    yyjson_doc_free(_to_yyjson(doc));
}

// getter
JsonValue* JsonDocument::root()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_root = yyjson_doc_get_root(yy_doc);
    return _from_yyjson(yy_root);
}
uint64_t JsonDocument::read_size()
{
    return (uint64_t)yyjson_doc_get_read_size(_to_yyjson(this));
}
uint64_t JsonDocument::value_count()
{
    return (uint64_t)yyjson_doc_get_val_count(_to_yyjson(this));
}

// to mut doc
JsonDocumentMut* JsonDocument::to_mut()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_mut_doc = yyjson_doc_mut_copy(yy_doc, nullptr);
    return _from_yyjson(yy_mut_doc);
}

// write
JsonWriteResult JsonDocument::write_to(JsonWriteCallback callback, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_write_opts(
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        callback(out_data, (uint64_t)out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonDocument::write_to_string(String& str, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_write_opts(
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        str.add_unsafe((uint64_t)out_size);
        ::std::memcpy(str.data_w(), out_data, out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonDocument::write_to_file(const Path& path, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_path = reinterpret_cast<const char*>(path.c_str());
    yyjson_write_err out_error;
    bool success = yyjson_write_file(
        yy_path,
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_error
    );

    if (!success) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        return {};
    }
}
} // namespace skr

// json document mut
namespace skr
{
// create a mutable document
JsonDocumentMut* JsonDocumentMut::Create()
{
    auto yy_alc = _make_yyjson_alc();
    auto yy_mut_doc = yyjson_mut_doc_new(&yy_alc);
    return _from_yyjson(yy_mut_doc);
}
SP<JsonDocumentMut> JsonDocumentMut::CreateSP()
{
    return { JsonDocumentMut::Create() };
}
UPtr<JsonDocumentMut> JsonDocumentMut::CreateUPtr()
{
    return { JsonDocumentMut::Create() };
}

// destroy a mutable document
void JsonDocumentMut::Destroy(JsonDocumentMut* doc)
{
    yyjson_mut_doc_free(_to_yyjson(doc));
}

// root value
JsonValueMut* JsonDocumentMut::root()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_root = yyjson_mut_doc_get_root(yy_doc);
    return _from_yyjson(yy_root);
}
void JsonDocumentMut::set_root(JsonValueMut* value)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = _to_yyjson(value);
    yyjson_mut_doc_set_root(yy_doc, yy_value);
}

// create value
JsonValueMut* JsonDocumentMut::create_raw(StringView raw)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_raw = reinterpret_cast<const char*>(raw.data());
    auto yy_len = static_cast<size_t>(raw.size());
    auto yy_value = yyjson_mut_rawncpy(yy_doc, yy_raw, yy_len);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_string(StringView str)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_str = reinterpret_cast<const char*>(str.data());
    auto yy_len = static_cast<size_t>(str.size());
    auto yy_value = yyjson_mut_strncpy(yy_doc, yy_str, yy_len);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_raw_non_copy(StringView raw)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_raw = reinterpret_cast<const char*>(raw.data());
    auto yy_len = static_cast<size_t>(raw.size());
    auto yy_value = yyjson_mut_rawn(yy_doc, yy_raw, yy_len);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_string_non_copy(StringView str)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_str = reinterpret_cast<const char*>(str.data());
    auto yy_len = static_cast<size_t>(str.size());
    auto yy_value = yyjson_mut_strn(yy_doc, yy_str, yy_len);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_null()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_null(yy_doc);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_bool(bool b)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_bool(yy_doc, b);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_int(int64_t i)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_int(yy_doc, i);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_uint(uint64_t u)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_uint(yy_doc, u);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_real(double f)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_real(yy_doc, f);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_object()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_obj(yy_doc);
    return _from_yyjson(yy_value);
}
JsonValueMut* JsonDocumentMut::create_array()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_value = yyjson_mut_arr(yy_doc);
    return _from_yyjson(yy_value);
}

// copy a value from another doc
JsonValueMut* JsonDocumentMut::copy_value(JsonValue* other)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_other = _to_yyjson(other);
    auto yy_copied = yyjson_val_mut_copy(yy_doc, yy_other);
    return _from_yyjson(yy_copied);
}
JsonValueMut* JsonDocumentMut::copy_value(JsonValueMut* other)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_other = _to_yyjson(other);
    auto yy_copied = yyjson_mut_val_mut_copy(yy_doc, yy_other);
    return _from_yyjson(yy_copied);
}

// copy document
JsonDocumentMut* JsonDocumentMut::copy()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_copied = yyjson_mut_doc_mut_copy(yy_doc, nullptr);
    return _from_yyjson(yy_copied);
}
JsonDocument* JsonDocumentMut::copy_readonly()
{
    auto yy_doc = _to_yyjson(this);
    auto yy_copied = yyjson_mut_doc_imut_copy(yy_doc, nullptr);
    return _from_yyjson(yy_copied);
}

// pooling optimize
void JsonDocumentMut::set_pool_size_str(uint64_t size)
{
    auto yy_doc = _to_yyjson(this);
    yyjson_mut_doc_set_str_pool_size(yy_doc, (size_t)size);
}
void JsonDocumentMut::set_pool_size_val(uint64_t val_count)
{
    auto yy_doc = _to_yyjson(this);
    yyjson_mut_doc_set_val_pool_size(yy_doc, (size_t)val_count);
}

// write
JsonWriteResult JsonDocumentMut::write_to(JsonWriteCallback callback, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_mut_write_opts(
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        callback(out_data, (uint64_t)out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonDocumentMut::write_to_string(String& str, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_mut_write_opts(
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        str.add_unsafe((uint64_t)out_size);
        ::std::memcpy(str.data_w(), out_data, out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonDocumentMut::write_to_file(const Path& path, EJsonWriteFlags flags)
{
    auto yy_doc = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_path = reinterpret_cast<const char*>(path.c_str());
    yyjson_write_err out_error;
    bool success = yyjson_mut_write_file(
        yy_path,
        yy_doc,
        yy_flags,
        &yy_alc,
        &out_error
    );

    if (!success) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        return {};
    }
}
} // namespace skr

// json value
namespace skr
{
// get type
EJsonValueType JsonValue::type()
{
    auto yy_val = _to_yyjson(this);
    auto yy_type = unsafe_yyjson_get_type(yy_val);
    auto yy_subtype = unsafe_yyjson_get_subtype(yy_val);

    switch (yy_type)
    {
    case YYJSON_TYPE_RAW:
        return EJsonValueType::Raw;
    case YYJSON_TYPE_NULL:
        return EJsonValueType::Null;
    case YYJSON_TYPE_BOOL:
        return EJsonValueType::Bool;
    case YYJSON_TYPE_STR:
        return EJsonValueType::String;
    case YYJSON_TYPE_ARR:
        return EJsonValueType::Array;
    case YYJSON_TYPE_OBJ:
        return EJsonValueType::Object;
    case YYJSON_TYPE_NUM: {
        switch (yy_subtype)
        {
        case YYJSON_SUBTYPE_SINT:
            return EJsonValueType::Int;
        case YYJSON_SUBTYPE_UINT:
            return EJsonValueType::UInt;
        case YYJSON_SUBTYPE_REAL:
            return EJsonValueType::Real;
        default:
            SKR_UNREACHABLE_CODE();
            return EJsonValueType::Raw;
        }
    }
    default:
        SKR_UNREACHABLE_CODE();
        return EJsonValueType::Raw;
    }
}
bool JsonValue::is_raw()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_raw(yy_val);
}
bool JsonValue::is_null()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_null(yy_val);
}
bool JsonValue::is_bool()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_bool(yy_val);
}
bool JsonValue::is_int()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_sint(yy_val);
}
bool JsonValue::is_uint()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_uint(yy_val);
}
bool JsonValue::is_real()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_real(yy_val);
}
bool JsonValue::is_number()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_num(yy_val);
}
bool JsonValue::is_string()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_str(yy_val);
}
bool JsonValue::is_object()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_obj(yy_val);
}
bool JsonValue::is_array()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_arr(yy_val);
}
bool JsonValue::is_container()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_ctn(yy_val);
}

// get value
String JsonValue::get_raw()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_get_len(yy_val);
    auto yy_raw = yyjson_get_raw(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_raw),
        (uint64_t)yy_len
    };
}
StringView JsonValue::get_raw_view()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_get_len(yy_val);
    auto yy_raw = yyjson_get_raw(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_raw),
        (uint64_t)yy_len
    };
}
String JsonValue::get_string()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_get_len(yy_val);
    auto yy_str = yyjson_get_str(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_str),
        (uint64_t)yy_len
    };
}
StringView JsonValue::get_string_view()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_get_len(yy_val);
    auto yy_str = yyjson_get_str(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_str),
        (uint64_t)yy_len
    };
}
bool JsonValue::get_bool()
{
    SKR_ASSERT(is_bool());
    auto yy_val = _to_yyjson(this);
    return yyjson_get_bool(yy_val);
}
int64_t JsonValue::get_int()
{
    SKR_ASSERT(is_int());
    auto yy_val = _to_yyjson(this);
    return yyjson_get_sint(yy_val);
}
uint64_t JsonValue::get_uint()
{
    SKR_ASSERT(is_uint());
    auto yy_val = _to_yyjson(this);
    return yyjson_get_uint(yy_val);
}
double JsonValue::get_real()
{
    SKR_ASSERT(is_real());
    auto yy_val = _to_yyjson(this);
    return yyjson_get_real(yy_val);
}

// get length
uint64_t JsonValue::get_raw_length()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_get_len(yy_val);
}
uint64_t JsonValue::get_string_length()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_get_len(yy_val);
}
uint64_t JsonValue::array_length()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_arr_size(yy_val);
}
uint64_t JsonValue::object_length()
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_obj_size(yy_val);
}

// array api
void JsonValue::array_foreach(ArrayForeachCallback callback)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);

    size_t idx, max;
    yyjson_val* val;
    yyjson_arr_foreach(yy_val, idx, max, val)
    {
        bool continue_loop = callback(
            _from_yyjson(val),
            idx
        );
        if (!continue_loop) [[unlikely]]
            break;
    }
}
JsonValue* JsonValue::array_at(uint64_t index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_index = (size_t)index;
    auto yy_elem = yyjson_arr_get(yy_val, yy_index);
    return _from_yyjson(yy_elem);
}
JsonIterArray JsonValue::array_iter()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    JsonIterArray iter;
    auto yy_iter = _to_yyjson(&iter);
    yyjson_arr_iter_init(yy_val, yy_iter);
    return iter;
}

// object api
void JsonValue::object_foreach(ObjectForeachCallback callback)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);

    size_t idx, max;
    yyjson_val *key, *val;
    yyjson_obj_foreach(yy_val, idx, max, key, val)
    {
        // key must be string
        auto yy_key_str = yyjson_get_str(key);
        auto yy_key_len = yyjson_get_len(key);
        StringView key{ reinterpret_cast<const skr_char8*>(yy_key_str), (uint64_t)yy_key_len };

        bool continue_loop = callback(
            key,
            _from_yyjson(val)
        );
        if (!continue_loop) [[unlikely]]
            break;
    }
}
JsonValue* JsonValue::object_get(StringView key)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = reinterpret_cast<const char*>(key.data());
    auto yy_len = static_cast<size_t>(key.size());
    auto yy_elem = yyjson_obj_getn(yy_val, yy_key, yy_len);
    return _from_yyjson(yy_elem);
}
JsonIterObject JsonValue::object_iter()
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    JsonIterObject iter;
    auto yy_iter = _to_yyjson(&iter);
    yyjson_obj_iter_init(yy_val, yy_iter);
    return iter;
}

// write
JsonWriteResult JsonValue::write_to(JsonWriteCallback callback, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_val_write_opts(
        yy_val,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        callback(out_data, (uint64_t)out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonValue::write_to_string(String& str, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_val_write_opts(
        yy_val,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        str.add_unsafe((uint64_t)out_size);
        ::std::memcpy(str.data_w(), out_data, out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonValue::write_to_file(const Path& path, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_path = reinterpret_cast<const char*>(path.c_str());
    yyjson_write_err out_error;
    bool success = yyjson_val_write_file(
        yy_path,
        yy_val,
        yy_flags,
        &yy_alc,
        &out_error
    );

    if (!success) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        return {};
    }
}
} // namespace skr

// json value mut
namespace skr
{
// get type
EJsonValueType JsonValueMut::type()
{
    auto yy_val = _to_yyjson(this);
    auto yy_type = unsafe_yyjson_get_type(yy_val);
    auto yy_subtype = unsafe_yyjson_get_subtype(yy_val);

    switch (yy_type)
    {
    case YYJSON_TYPE_RAW:
        return EJsonValueType::Raw;
    case YYJSON_TYPE_NULL:
        return EJsonValueType::Null;
    case YYJSON_TYPE_BOOL:
        return EJsonValueType::Bool;
    case YYJSON_TYPE_STR:
        return EJsonValueType::String;
    case YYJSON_TYPE_ARR:
        return EJsonValueType::Array;
    case YYJSON_TYPE_OBJ:
        return EJsonValueType::Object;
    case YYJSON_TYPE_NUM: {
        switch (yy_subtype)
        {
        case YYJSON_SUBTYPE_SINT:
            return EJsonValueType::Int;
        case YYJSON_SUBTYPE_UINT:
            return EJsonValueType::UInt;
        case YYJSON_SUBTYPE_REAL:
            return EJsonValueType::Real;
        default:
            SKR_UNREACHABLE_CODE();
            return EJsonValueType::Raw;
        }
    }
    default:
        SKR_UNREACHABLE_CODE();
        return EJsonValueType::Raw;
    }
}
bool JsonValueMut::is_raw()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_raw(yy_val);
}
bool JsonValueMut::is_null()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_null(yy_val);
}
bool JsonValueMut::is_bool()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_bool(yy_val);
}
bool JsonValueMut::is_int()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_sint(yy_val);
}
bool JsonValueMut::is_uint()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_uint(yy_val);
}
bool JsonValueMut::is_real()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_real(yy_val);
}
bool JsonValueMut::is_number()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_num(yy_val);
}
bool JsonValueMut::is_string()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_str(yy_val);
}
bool JsonValueMut::is_object()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_obj(yy_val);
}
bool JsonValueMut::is_array()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_arr(yy_val);
}
bool JsonValueMut::is_container()
{
    auto yy_val = _to_yyjson(this);
    return unsafe_yyjson_is_ctn(yy_val);
}

// get value
String JsonValueMut::get_raw()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_mut_get_len(yy_val);
    auto yy_raw = yyjson_mut_get_raw(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_raw),
        (uint64_t)yy_len
    };
}
StringView JsonValueMut::get_raw_view()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_mut_get_len(yy_val);
    auto yy_raw = yyjson_mut_get_raw(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_raw),
        (uint64_t)yy_len
    };
}
String JsonValueMut::get_string()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_mut_get_len(yy_val);
    auto yy_str = yyjson_mut_get_str(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_str),
        (uint64_t)yy_len
    };
}
StringView JsonValueMut::get_string_view()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    auto yy_len = yyjson_mut_get_len(yy_val);
    auto yy_str = yyjson_mut_get_str(yy_val);
    return {
        reinterpret_cast<const skr_char8*>(yy_str),
        (uint64_t)yy_len
    };
}
bool JsonValueMut::get_bool()
{
    SKR_ASSERT(is_bool());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_get_bool(yy_val);
}
int64_t JsonValueMut::get_int()
{
    SKR_ASSERT(is_int());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_get_sint(yy_val);
}
uint64_t JsonValueMut::get_uint()
{
    SKR_ASSERT(is_uint());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_get_uint(yy_val);
}
double JsonValueMut::get_real()
{
    SKR_ASSERT(is_real());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_get_real(yy_val);
}

// get length
uint64_t JsonValueMut::get_raw_length()
{
    SKR_ASSERT(is_raw());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_mut_get_len(yy_val);
}
uint64_t JsonValueMut::get_string_length()
{
    SKR_ASSERT(is_string());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_mut_get_len(yy_val);
}
uint64_t JsonValueMut::array_length()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_mut_arr_size(yy_val);
}
uint64_t JsonValueMut::object_length()
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    return (uint64_t)yyjson_mut_obj_size(yy_val);
}

// array api
void JsonValueMut::array_foreach(ArrayForeachCallback callback)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);

    size_t idx, max;
    yyjson_mut_val* val;
    yyjson_mut_arr_foreach(yy_val, idx, max, val)
    {
        bool continue_loop = callback(
            _from_yyjson(val),
            idx
        );
        if (!continue_loop) [[unlikely]]
            break;
    }
}
JsonValueMut* JsonValueMut::array_at(uint64_t index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_index = (size_t)index;
    auto yy_elem = yyjson_mut_arr_get(yy_val, yy_index);
    return _from_yyjson(yy_elem);
}
JsonIterArrayMut JsonValueMut::array_iter()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    JsonIterArrayMut iter;
    auto yy_iter = _to_yyjson(&iter);
    yyjson_mut_arr_iter_init(yy_val, yy_iter);
    return iter;
}

// object api
void JsonValueMut::object_foreach(ObjectForeachCallback callback)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);

    size_t idx, max;
    yyjson_mut_val *key, *val;
    yyjson_mut_obj_foreach(yy_val, idx, max, key, val)
    {
        // key must be string
        auto yy_key_str = yyjson_mut_get_str(key);
        auto yy_key_len = yyjson_mut_get_len(key);
        StringView key{ reinterpret_cast<const skr_char8*>(yy_key_str), (uint64_t)yy_key_len };

        bool continue_loop = callback(
            key,
            _from_yyjson(val)
        );
        if (!continue_loop) [[unlikely]]
            break;
    }
}
JsonValueMut* JsonValueMut::object_get(StringView key)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = reinterpret_cast<const char*>(key.data());
    auto yy_len = static_cast<size_t>(key.size());
    auto yy_elem = yyjson_mut_obj_getn(yy_val, yy_key, yy_len);
    return _from_yyjson(yy_elem);
}
JsonIterObjectMut JsonValueMut::object_iter()
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    JsonIterObjectMut iter;
    auto yy_iter = _to_yyjson(&iter);
    yyjson_mut_obj_iter_init(yy_val, yy_iter);
    return iter;
}

// write
JsonWriteResult JsonValueMut::write_to(JsonWriteCallback callback, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_mut_val_write_opts(
        yy_val,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        callback(out_data, (uint64_t)out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonValueMut::write_to_string(String& str, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    size_t out_size;
    yyjson_write_err out_error;
    char* out_data = yyjson_mut_val_write_opts(
        yy_val,
        yy_flags,
        &yy_alc,
        &out_size,
        &out_error
    );

    if (!out_data) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        str.add_unsafe((uint64_t)out_size);
        ::std::memcpy(str.data_w(), out_data, out_size);
        skr_yyjson_free(nullptr, out_data);
        return {};
    }
}
JsonWriteResult JsonValueMut::write_to_file(const Path& path, EJsonWriteFlags flags)
{
    auto yy_val = _to_yyjson(this);
    auto yy_flags = _to_yyjson(flags);
    auto yy_alc = _make_yyjson_alc();
    auto yy_path = reinterpret_cast<const char*>(path.c_str());
    yyjson_write_err out_error;
    bool success = yyjson_mut_val_write_file(
        yy_path,
        yy_val,
        yy_flags,
        &yy_alc,
        &out_error
    );

    if (!success) [[unlikely]]
    {
        return _make_error_result(out_error);
    }
    else
    {
        return {};
    }
}

// copy to readonly document
JsonDocument* JsonValueMut::copy_readonly()
{
    auto yy_val = _to_yyjson(this);
    auto yy_doc = yyjson_mut_val_imut_copy(yy_val, nullptr);
    return _from_yyjson(yy_doc);
}

// array modify api
bool JsonValueMut::arrray_insert(JsonValueMut* value, uint64_t index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_value = _to_yyjson(value);
    auto yy_index = (size_t)index;
    return yyjson_mut_arr_insert(yy_val, yy_value, yy_index);
}
JsonValueMut* JsonValueMut::arrray_replace(JsonValueMut* value, uint64_t index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_value = _to_yyjson(value);
    auto yy_index = (size_t)index;
    auto yy_old = yyjson_mut_arr_replace(yy_val, yy_index, yy_value);
    return _from_yyjson(yy_old);
}
bool JsonValueMut::arrray_push_back(JsonValueMut* value)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_value = _to_yyjson(value);
    return yyjson_mut_arr_append(yy_val, yy_value);
}
bool JsonValueMut::arrray_push_front(JsonValueMut* value)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_value = _to_yyjson(value);
    return yyjson_mut_arr_prepend(yy_val, yy_value);
}
JsonValueMut* JsonValueMut::arrray_pop_back()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_old = yyjson_mut_arr_remove_last(yy_val);
    return _from_yyjson(yy_old);
}
JsonValueMut* JsonValueMut::arrray_pop_front()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_old = yyjson_mut_arr_remove_first(yy_val);
    return _from_yyjson(yy_old);
}
JsonValueMut* JsonValueMut::arrray_remove_at(uint64_t index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_index = (size_t)index;
    auto yy_old = yyjson_mut_arr_remove(yy_val, yy_index);
    return _from_yyjson(yy_old);
}
bool JsonValueMut::arrray_remove_range(uint64_t index, uint64_t count)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_index = (size_t)index;
    auto yy_count = (size_t)count;
    return yyjson_mut_arr_remove_range(yy_val, yy_index, yy_count);
}
bool JsonValueMut::arrray_clear()
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_arr_clear(yy_val);
}
bool JsonValueMut::arrray_rotate(uint64_t mid_index)
{
    SKR_ASSERT(is_array());
    auto yy_val = _to_yyjson(this);
    auto yy_mid_index = (size_t)mid_index;
    return yyjson_mut_arr_rotate(yy_val, yy_mid_index);
}

// object modify api
bool JsonValueMut::object_add(JsonValueMut* key, JsonValueMut* value)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = _to_yyjson(key);
    auto yy_value = _to_yyjson(value);
    return yyjson_mut_obj_add(yy_val, yy_key, yy_value);
}
bool JsonValueMut::object_set(JsonValueMut* key, JsonValueMut* value)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = _to_yyjson(key);
    auto yy_value = _to_yyjson(value);
    return yyjson_mut_obj_put(yy_val, yy_key, yy_value);
}
bool JsonValueMut::object_replace(JsonValueMut* key, JsonValueMut* value)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = _to_yyjson(key);
    auto yy_value = _to_yyjson(value);
    return yyjson_mut_obj_replace(yy_val, yy_key, yy_value);
}
bool JsonValueMut::object_remove_all(StringView key)
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    auto yy_key = reinterpret_cast<const char*>(key.data());
    auto yy_len = static_cast<size_t>(key.size());
    return yyjson_mut_obj_remove_keyn(yy_val, yy_key, yy_len);
}
bool JsonValueMut::object_rename_key(JsonDocumentMut* doc, StringView old_key, StringView new_key)
{
    SKR_ASSERT(is_object());
    auto yy_doc = _to_yyjson(doc);
    auto yy_val = _to_yyjson(this);
    auto yy_old_key = reinterpret_cast<const char*>(old_key.data());
    auto yy_old_len = static_cast<size_t>(old_key.size());
    auto yy_new_key = reinterpret_cast<const char*>(new_key.data());
    auto yy_new_len = static_cast<size_t>(new_key.size());
    return yyjson_mut_obj_rename_keyn(yy_doc, yy_val, yy_old_key, yy_old_len, yy_new_key, yy_new_len);
}
bool JsonValueMut::object_clear()
{
    SKR_ASSERT(is_object());
    auto yy_val = _to_yyjson(this);
    return yyjson_mut_obj_clear(yy_val);
}
} // namespace skr