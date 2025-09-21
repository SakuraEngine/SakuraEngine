#include <SkrCore/serialize/json_archive.hpp>
#include <SkrOS/filesystem.hpp>

// impl ArWriteJson
namespace skr
{
// ctor & dtor
ArWriteJson::ArWriteJson()
{
    _is_structured = true;

    _enable_string_enum = true;
    _enable_string_guid = true;
    _enable_string_md5 = true;
    _enable_string_sha256 = true;
}
ArWriteJson::~ArWriteJson()
{
}

// structured api
void ArWriteJson::impl_key(StringView key)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    if (_has_key) [[unlikely]]
    { // key called twice
        error(u8"[ArWriteJson] cannot set key twice without using it");
        return;
    }
    if (_scope_stack.is_empty()) [[unlikely]]
    { // call when no scope
        error(u8"[ArWriteJson] cannot set key at root scope");
        return;
    }
    else if (_scope_stack.back()->is_array()) [[unlikely]]
    { // call in array scope
        error(u8"[ArWriteJson] cannot set key in array scope");
        return;
    }

    _set_key(key, false);
}
void ArWriteJson::impl_key_copy(StringView key)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    if (_has_key) [[unlikely]]
    { // key called twice
        error(u8"[ArWriteJson] cannot set key twice without using it");
        return;
    }
    if (_scope_stack.is_empty()) [[unlikely]]
    { // call when no scope
        error(u8"[ArWriteJson] cannot set key at root scope");
        return;
    }
    else if (_scope_stack.back()->is_array()) [[unlikely]]
    { // call in array scope
        error(u8"[ArWriteJson] cannot set key in array scope");
        return;
    }

    _set_key(key, true);
}
void ArWriteJson::impl_begin_array()
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_FAST_CHECK(_check_key_state(), );

    // handle value call
    auto arr = _doc->create_array();
    if (_scope_stack.is_empty())
    { // as root
        _root = arr;
    }
    else
    { // add to current scope
        _add_to_scope(arr);
    }

    // push to scope stack
    _scope_stack.push_back(arr);
}
void ArWriteJson::impl_end_array()
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    if (_scope_stack.is_empty()) [[unlikely]]
    { // miss begin_array
        error(u8"[ArWriteJson] end_array() miss begin_array()");
        return;
    }
    if (!_scope_stack.back()->is_array()) [[unlikely]]
    { // bad scope
        error(u8"[ArWriteJson] end_array() not in array scope");
        return;
    }

    // pop scope
    _scope_stack.pop_back();
}
void ArWriteJson::impl_begin_object()
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_FAST_CHECK(_check_key_state(), );

    // handle value call
    auto obj = _doc->create_object();
    if (_scope_stack.is_empty())
    { // as root
        _root = obj;
    }
    else
    { // call in unmatched scope
        _add_to_scope(obj);
    }

    // push to scope stack
    _scope_stack.push_back(obj);
}
void ArWriteJson::impl_end_object()
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    if (_scope_stack.is_empty()) [[unlikely]]
    { // miss begin_object
        error(u8"[ArWriteJson] end_object() miss begin_object()");
        return;
    }
    if (!_scope_stack.back()->is_object()) [[unlikely]]
    { // call in unmatched scope
        error(u8"[ArWriteJson] end_object() not in object scope");
        return;
    }

    // pop scope
    _scope_stack.pop_back();
}

// typed data
void ArWriteJson::impl_primitive(EArchivePrimitiveType type, const void* data)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_FAST_CHECK(_check_key_state(), );
    if (_scope_stack.is_empty()) [[unlikely]]
    { // cannot write at root scope
        error(u8"[ArWriteJson] cannot write primitive at root scope");
        return;
    }

    // create value
    JsonValueMut* val;
    switch (type)
    {
    case EArchivePrimitiveType::Bool:
        val = _doc->create_bool(*(const bool*)data);
        break;
    case EArchivePrimitiveType::Int8:
        val = _doc->create_int(*(const int8_t*)data);
        break;
    case EArchivePrimitiveType::Int16:
        val = _doc->create_int(*(const int16_t*)data);
        break;
    case EArchivePrimitiveType::Int32:
        val = _doc->create_int(*(const int32_t*)data);
        break;
    case EArchivePrimitiveType::Int64:
        val = _doc->create_int(*(const int64_t*)data);
        break;
    case EArchivePrimitiveType::UInt8:
        val = _doc->create_uint(*(const uint8_t*)data);
        break;
    case EArchivePrimitiveType::UInt16:
        val = _doc->create_uint(*(const uint16_t*)data);
        break;
    case EArchivePrimitiveType::UInt32:
        val = _doc->create_uint(*(const uint32_t*)data);
        break;
    case EArchivePrimitiveType::UInt64:
        val = _doc->create_uint(*(const uint64_t*)data);
        break;
    case EArchivePrimitiveType::Float:
        val = _doc->create_real(*(const float*)data);
        break;
    case EArchivePrimitiveType::Double:
        val = _doc->create_real(*(const double*)data);
        break;
    default:
        error(u8"[ArWriteJson] does not support this primitive type");
        return;
    }

    // add to scope
    _add_to_scope(val);
}
void ArWriteJson::impl_str(StringView str)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_FAST_CHECK(_check_key_state(), );
    if (_scope_stack.is_empty()) [[unlikely]]
    { // cannot write at root scope
        error(u8"[ArWriteJson] cannot write string at root scope");
        return;
    }

    // add to scope
    _add_to_scope(_doc->create_string(str));
}

// binary data support
void ArWriteJson::impl_bytes(const void* data, uint64_t size)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    error(u8"[ArWriteJson] does not support impl_bytes");
}
void ArWriteJson::impl_bits(const void* data, uint64_t size, uint64_t offset)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    error(u8"[ArWriteJson] does not support impl_bits");
}

// factory functions, used for fast create and use
ArWriteJson ArWriteJson::Create()
{
    ArWriteJson writer;
    writer.use_doc();
    return writer;
}
ArWriteJson ArWriteJson::UseDoc(SP<JsonDocumentMut> doc, bool auto_set_root)
{
    ArWriteJson writer;
    writer.use_doc(std::move(doc), auto_set_root);
    return writer;
}

// step 1: document setup
// if doc is nullptr, a new doc will be created
// if auto_set_root is true, root will be set to doc->root()
void ArWriteJson::use_doc(SP<JsonDocumentMut> doc, bool auto_set_root)
{
    // reset document first
    reset_doc();

    // create doc if needed
    if (!doc) doc = JsonDocumentMut::CreateSP();

    _doc = std::move(doc);
    _auto_set_doc_root = auto_set_root;
}
bool ArWriteJson::has_doc() const
{
    return _doc != nullptr;
}

// step 2: get result
const SP<JsonDocumentMut>& ArWriteJson::doc() const
{
    return _doc;
}
JsonValueMut* ArWriteJson::root() const
{
    return _root;
}
JsonWriteResult ArWriteJson::write_to(JsonWriteCallback callback, EJsonWriteFlags flags)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_ASSERT(_root && "root is null, please serialize a root object first");
    return _root->write_to(callback, flags);
}
JsonWriteResult ArWriteJson::write_to_string(String& str, EJsonWriteFlags flags)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_ASSERT(_root && "root is null, please serialize a root object first");
    return _root->write_to_string(str, flags);
}
JsonWriteResult ArWriteJson::write_to_file(const Path& path, EJsonWriteFlags flags)
{
    SKR_ASSERT(_doc && "document is null, please call use_doc() first");
    SKR_ASSERT(_root && "root is null, please serialize a root object first");
    return _root->write_to_file(path, flags);
}

// step 3: reset state for next use
// reset means clear write state
// reset document means clear document, and you need to setup document again
void ArWriteJson::reset()
{
    _root = nullptr;
    _scope_stack.clear();
    _error_tracker.reset();
    _reset_key_state();
}
void ArWriteJson::reset_doc()
{
    if (has_doc())
    {
        _doc = nullptr;
        _auto_set_doc_root = false;
    }
}
void ArWriteJson::reset_all()
{
    reset();
    reset_doc();
}

// checks
bool ArWriteJson::_check_key_state()
{
    if (!_scope_stack.is_empty()) [[likely]]
    {
        auto& cur_scope = _scope_stack.back();
        if (cur_scope->is_object() && !_has_key) [[unlikely]]
        { // must set key before writing value
            error(u8"[ArWriteJson] must set key before writing value in object scope");
            return false;
        }
        //! array scope checked in impl_key
    }
    return true;
}

// key operations
void ArWriteJson::_set_key(StringView key, bool need_copy)
{
    _has_key = true;
    _key_is_literal = !need_copy;
    if (_key_is_literal)
    {
        _literal_key = key;
    }
    else
    {
        _owned_key = key;
    }
}
JsonValueMut* ArWriteJson::_get_key_value() const
{
    if (_has_key) [[likely]]
    {
        if (_key_is_literal) [[likely]]
            return _doc->create_string_non_copy(_literal_key);
        else
            return _doc->create_string(_owned_key);
    }
    else
    {
        return nullptr;
    }
}
void ArWriteJson::_reset_key_state()
{
    _has_key = false;
    _literal_key = {};
    _owned_key.clear();
}

// scope operations
bool ArWriteJson::_add_to_scope(JsonValueMut* value)
{
    auto& cur_scope = _scope_stack.back();
    SKR_DEFER({ _reset_key_state(); });

    if (cur_scope->is_array())
    { // push to array
        SKR_FAST_CHECK(cur_scope->arrray_push_back(value), false);
    }
    else if (cur_scope->is_object())
    { // set to object
        // build key
        JsonValueMut* key = _get_key_value();

        // add to object
        SKR_FAST_CHECK(cur_scope->object_add(key, value), false);
    }
    return true;
}
} // namespace skr

// impl ArReadJson
namespace skr
{
// ctor & dtor
ArReadJson::ArReadJson()
{
    _is_structured = true;

    _enable_string_enum = true;
    _enable_string_guid = true;
    _enable_string_md5 = true;
    _enable_string_sha256 = true;
}
ArReadJson::~ArReadJson()
{
}

// structured api
void ArReadJson::impl_key(StringView key)
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), );
    auto& cur_scope = _scope_stack.back();

    if (!cur_scope.value.is_object()) [[unlikely]]
    { // not in object scope
        error(u8"[ArReadJson] cannot set key in non-object scope");
        return;
    }
    if (cur_scope.pending_value) [[unlikely]]
    { // key called twice
        error(u8"[ArReadJson] cannot set key twice without consuming value");
        return;
    }

    _update_key_in_scope(key, cur_scope);
}
bool ArReadJson::impl_has_value()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), false);
    auto& cur_scope = _scope_stack.back();
    return cur_scope.pending_value != nullptr;
}
void ArReadJson::impl_begin_array()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    if (_scope_stack.is_empty())
    { // push root object
        if (!_root.is_array()) [[unlikely]]
        { // root type miss match
            error(u8"[ArReadJson] cannot push root scope that is not array");
            return;
        }

        // push to scope
        _push_scop(_root);
    }
    else
    { // comsume current value and push
        SKR_FAST_CHECK(_check_has_scope(), );
        auto cur_value = _consume_current_value();
        if (!cur_value) [[unlikely]]
        { // no valid pending value
            error(u8"[ArReadJson] cannot begin_array without valid value");
            return;
        }
        if (!cur_value.is_array()) [[unlikely]]
        { // pending value type miss match
            error(u8"[ArReadJson] cannot begin_array on non-array value");
            return;
        }

        // push to scope
        _push_scop(cur_value);
    }
}
uint64_t ArReadJson::impl_array_size()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), 0);
    auto& cur_scope = _scope_stack.back();
    if (!cur_scope.value.is_array()) [[unlikely]]
    { // not in array scope
        error(u8"[ArReadJson] call array_size() not in array scope");
        return 0;
    }
    return cur_scope.value.array_length();
}
void ArReadJson::impl_end_array()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), );
    if (!_scope_stack.back().value.is_array()) [[unlikely]]
    { // not in array scope
        error(u8"[ArReadJson] call end_array() not in array scope");
        return;
    }

    // pop scope
    _scope_stack.pop_back();
}
void ArReadJson::impl_begin_object()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    if (_scope_stack.is_empty())
    { // push root object
        if (!_root.is_object()) [[unlikely]]
        { // root type miss match
            error(u8"[ArReadJson] cannot push root scope that is not object");
            return;
        }
        _push_scop(_root);
    }
    else
    { // comsume current value and push
        SKR_FAST_CHECK(_check_has_scope(), );
        auto cur_value = _consume_current_value();
        if (!cur_value) [[unlikely]]
        { // no valid pending value
            error(u8"[ArReadJson] cannot begin_object without valid value");
            return;
        }
        if (!cur_value.is_object()) [[unlikely]]
        { // pending value type miss match
            error(u8"[ArReadJson] cannot begin_object on non-object value");
            return;
        }

        // push to scope
        _push_scop(cur_value);
    }
}
void ArReadJson::impl_end_object()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), );
    if (!_scope_stack.back().value.is_object()) [[unlikely]]
    { // not in object scope
        error(u8"[ArReadJson] call end_object() not in object scope");
        return;
    }

    // pop scope
    _scope_stack.pop_back();
}

// typed data
void ArReadJson::impl_primitive(EArchivePrimitiveType type, void* data)
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), );
    auto cur_value = _consume_current_value();
    if (!cur_value) [[unlikely]]
    { // no valid pending value
        error(u8"[ArReadJson] cannot read primitive without valid value");
        return;
    }

    bool type_mismatch = false;
    switch (type)
    {
    case EArchivePrimitiveType::Bool: {
        auto v = cur_value.try_convert_to<bool>();
        if (v.has_value())
            *(bool*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Int8: {
        auto v = cur_value.try_convert_to<int8_t>();
        if (v.has_value())
            *(int8_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Int16: {
        auto v = cur_value.try_convert_to<int16_t>();
        if (v.has_value())
            *(int16_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Int32: {
        auto v = cur_value.try_convert_to<int32_t>();
        if (v.has_value())
            *(int32_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Int64: {
        auto v = cur_value.try_convert_to<int64_t>();
        if (v.has_value())
            *(int64_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::UInt8: {
        auto v = cur_value.try_convert_to<uint8_t>();
        if (v.has_value())
            *(uint8_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::UInt16: {
        auto v = cur_value.try_convert_to<uint16_t>();
        if (v.has_value())
            *(uint16_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::UInt32: {
        auto v = cur_value.try_convert_to<uint32_t>();
        if (v.has_value())
            *(uint32_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::UInt64: {
        auto v = cur_value.try_convert_to<uint64_t>();
        if (v.has_value())
            *(uint64_t*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Float: {
        auto v = cur_value.try_convert_to<float>();
        if (v.has_value())
            *(float*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    case EArchivePrimitiveType::Double: {
        auto v = cur_value.try_convert_to<double>();
        if (v.has_value())
            *(double*)data = v.value();
        else
            type_mismatch = true;
        break;
    }
    }

    if (type_mismatch) [[unlikely]]
    {
        if (get_behaviour_when_primitive_type_mismatch() != EArchiveLogBehaviour::Silence)
        {

            StringView type_name = u8"unknown";
            switch (type)
            {
            case EArchivePrimitiveType::Bool:
                type_name = u8"bool";
                break;
            case EArchivePrimitiveType::Int8:
                type_name = u8"int8";
                break;
            case EArchivePrimitiveType::Int16:
                type_name = u8"int16";
                break;
            case EArchivePrimitiveType::Int32:
                type_name = u8"int32";
                break;
            case EArchivePrimitiveType::Int64:
                type_name = u8"int64";
                break;
            case EArchivePrimitiveType::UInt8:
                type_name = u8"uint8";
                break;
            case EArchivePrimitiveType::UInt16:
                type_name = u8"uint16";
                break;
            case EArchivePrimitiveType::UInt32:
                type_name = u8"uint32";
                break;
            case EArchivePrimitiveType::UInt64:
                type_name = u8"uint64";
                break;
            case EArchivePrimitiveType::Float:
                type_name = u8"float";
                break;
            case EArchivePrimitiveType::Double:
                type_name = u8"double";
                break;
            }
            String type_str;
            switch (cur_value.type())
            {
            case EJsonValueType::Null:
                type_str = u8"null";
                break;
            case EJsonValueType::Bool:
                type_str = cur_value.get_bool() ? u8"true" : u8"false";
                break;
            case EJsonValueType::Int:
                format_to(type_str, u8"int({})", cur_value.get_int());
                break;
            case EJsonValueType::UInt:
                format_to(type_str, u8"uint({})", cur_value.get_uint());
                break;
            case EJsonValueType::Real:
                format_to(type_str, u8"real({})", cur_value.get_real());
                break;
            case EJsonValueType::String:
                format_to(type_str, u8"str(\"{}\")", cur_value.get_string_view());
                break;
            case EJsonValueType::Raw:
                type_str = u8"<raw>";
                break;
            case EJsonValueType::Array:
                type_str = u8"<array>";
                break;
            case EJsonValueType::Object:
                type_str = u8"<object>";
                break;
            }

            log_by_behaviour(
                get_behaviour_when_primitive_type_mismatch(),
                u8"[ArReadJson] primitive type mismatch, expected type '{}', got '{}'",
                type_name,
                type_str
            );
        }
    }
}
StringView ArReadJson::impl_str_view()
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    SKR_FAST_CHECK(_check_has_scope(), {});
    auto cur_value = _consume_current_value();
    if (!cur_value) [[unlikely]]
    { // no valid pending value
        error(u8"[ArReadJson] cannot read string without valid value");
        return {};
    }
    if (!cur_value.is_string())
    { // type miss match
        error(u8"[ArReadJson] cannot read non-string value as string");
        return {};
    }

    return cur_value.get_string_view();
}

// binary data support
void ArReadJson::impl_bytes(void* data, uint64_t size)
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    error(u8"[ArReadJson] does not support impl_bytes");
}
void ArReadJson::impl_bits(void* data, uint64_t size, uint64_t offset)
{
    SKR_ASSERT(has_root() && "document is null, please call use_root() first");
    error(u8"[ArReadJson] does not support impl_bits");
}

// factory
ArReadJson ArReadJson::UseRoot(SP<JsonDocument> doc, JsonValue* root)
{
    ArReadJson reader;
    reader.use_root(std::move(doc), root);
    return reader;
}
ArReadJson ArReadJson::UseRoot(SP<JsonDocumentMut> doc, JsonValueMut* root)
{
    ArReadJson reader;
    reader.use_root(std::move(doc), root);
    return reader;
}
ArReadJson ArReadJson::UseRootUPtr(UPtr<JsonDocument> doc, JsonValue* root)
{
    ArReadJson reader;
    reader.use_root_uptr(std::move(doc), root);
    return reader;
}
ArReadJson ArReadJson::UseRootUPtr(UPtr<JsonDocumentMut> doc, JsonValueMut* root)
{
    ArReadJson reader;
    reader.use_root_uptr(std::move(doc), root);
    return reader;
}
ArReadJson ArReadJson::ReadFile(const Path& path, EJsonReadFlags flags)
{
    JsonReadResult result = JsonDocument::ReadFile(path, flags);
    if (result.is_success())
    {
        ArReadJson reader;
        reader.use_root(result.value);
        return reader;
    }
    else
    {
        ArReadJson reader;
        reader.error(
            u8"[ArReadJson] failed to read json file '{}', error: {}",
            path.string(),
            result.error_msg
        );
        return reader;
    }
}
ArReadJson ArReadJson::ReadBuffer(const void* buffer, uint64_t len, EJsonReadFlags flags)
{
    JsonReadResult result = JsonDocument::ReadBuffer(buffer, len, flags);
    if (result.is_success())
    {
        ArReadJson reader;
        reader.use_root(result.value);
        return reader;
    }
    else
    {
        ArReadJson reader;
        reader.error(
            u8"[ArReadJson] failed to read json buffer, pos ({}:{}), error: {}",
            result.error_line,
            result.error_col,
            result.error_msg
        );
        return reader;
    }
}

// step 1: setup read source
// doc used to take ownership of document
// value used to set root value, if null, root will be doc->root()
void ArReadJson::use_root(SP<JsonDocument> doc, JsonValue* root)
{
    SKR_ASSERT(doc && "doc must not be null, and has ownership of value");
    reset_root();
    _doc_sp = std::move(doc);
    _root = root ? root : _doc_sp->root();
}
void ArReadJson::use_root(SP<JsonDocumentMut> doc, JsonValueMut* root)
{
    SKR_ASSERT(doc && "doc must not be null, and has ownership of value");
    reset_root();
    _doc_mut_sp = std::move(doc);
    _root = root ? root : _doc_mut_sp->root();
}
void ArReadJson::use_root_uptr(UPtr<JsonDocument> doc, JsonValue* root)
{
    SKR_ASSERT(doc && "doc must not be null, and has ownership of value");
    reset_root();
    _doc_sp = doc.release();
    _root = root ? root : _doc_sp->root();
}
void ArReadJson::use_root_uptr(UPtr<JsonDocumentMut> doc, JsonValueMut* root)
{
    SKR_ASSERT(doc && "doc must not be null, and has ownership of value");
    reset_root();
    _doc_mut_sp = doc.release();
    _root = root ? root : _doc_mut_sp->root();
}
JsonReadResult ArReadJson::read_file(const Path& path, EJsonReadFlags flags)
{
    auto result = JsonDocument::ReadFile(path, flags);
    if (result.is_success())
    {
        use_root_uptr(result.value);
    }
    return result;
}
JsonReadResult ArReadJson::read_buffer(const void* buffer, uint64_t len, EJsonReadFlags flags)
{
    auto result = JsonDocument::ReadBuffer(buffer, len, flags);
    if (result.is_success())
    {
        use_root_uptr(result.value);
    }
    return result;
}
JsonValueCommon ArReadJson::root() const
{
    return _root;
}
bool ArReadJson::has_root() const
{
    return _root != nullptr;
}

// step 2: reset state for next use
void ArReadJson::reset()
{
    _scope_stack.clear();
    _error_tracker.reset();
}
void ArReadJson::reset_root()
{
    if (has_root())
    {
        _doc_mut_sp = nullptr;
        _doc_sp = nullptr;

        _root = nullptr;
    }
}
void ArReadJson::reset_all()
{
    reset();
    reset_root();
}

// checks
bool ArReadJson::_check_has_scope()
{
    if (_scope_stack.is_empty()) [[unlikely]]
    {
        error(u8"[ArReadJson] requires a valid scope");
        return false;
    }
    return true;
}

// value operations
JsonValueCommon ArReadJson::_consume_current_value()
{
    auto& cur_scope = _scope_stack.back();
    if (cur_scope.value.is_array())
    { // consume current and move next
        // cache value
        auto val = cur_scope.pending_value;

        // update iter
        if (_root.is_mut())
        {
            if (cur_scope.data_arr.iter_mut.has_next())
                cur_scope.pending_value = cur_scope.data_arr.iter.move_next();
            else
                cur_scope.pending_value = nullptr;
        }
        else
        {
            if (cur_scope.data_arr.iter.has_next())
                cur_scope.pending_value = cur_scope.data_arr.iter.move_next();
            else
                cur_scope.pending_value = nullptr;
        }

        // return saved value
        return val;
    }
    else if (cur_scope.value.is_object())
    { // consume current key-value pair, and wait for next key
        auto val = cur_scope.pending_value;
        cur_scope.pending_value = nullptr;
        return val;
    }
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

// scope control
void ArReadJson::_push_scop(JsonValueCommon value)
{
    if (value.is_array())
    {
        auto ref = _scope_stack.add_unsafe();
        ref.ref().value = value;

        // setup iterator
        if (value.is_mut())
            ref.ref().data_arr.iter_mut = value.get_value_mut()->array_iter();
        else
            ref.ref().data_arr.iter = value.get_value()->array_iter();

        // array init pending value
        ref.ref().pending_value = ref.ref().data_arr.iter.has_next() ?
            ref.ref().data_arr.iter.move_next() :
            nullptr;
        return;
    }
    else if (value.is_object())
    {
        auto ref = _scope_stack.add_unsafe();
        ref.ref().value = value;

        // setup iterator
        if (value.is_mut())
            ref.ref().data_obj.iter_mut = value.get_value_mut()->object_iter();
        else
            ref.ref().data_obj.iter = value.get_value()->object_iter();

        // object init pending value should be null
        ref.ref().pending_value = nullptr;

        // init retry count
        ref.ref().data_obj.retry_count = 0;
        return;
    }
    SKR_UNREACHABLE_CODE();
}
void ArReadJson::_update_key_in_scope(StringView key, StackNode& scope)
{
    if (_root.is_mut())
    {
        // find key
        if (scope.data_obj.retry_count > _max_retry_count) [[unlikely]]
        { // use linear search
            scope.pending_value = scope.value.object_get(key);
        }
        else [[likely]]
        {
            // try find by iterator first
            scope.pending_value = scope.data_obj.iter_mut.move_next_key(key);

            // retry
            if (!scope.pending_value) [[unlikely]]
            {
                // update retry count
                scope.data_obj.retry_count++;

                // reset iterator and find again
                scope.data_obj.iter_mut = scope.value.get_value_mut()->object_iter();
                scope.pending_value = scope.data_obj.iter_mut.move_next_key(key);

                // reset iterator for next search if not found
                if (!scope.pending_value) [[unlikely]]
                {
                    scope.data_obj.iter_mut = scope.value.get_value_mut()->object_iter();
                }
            }
        }
    }
    else
    {
        // find key
        if (scope.data_obj.retry_count > _max_retry_count) [[unlikely]]
        { // use linear search
            scope.pending_value = scope.value.object_get(key);
        }
        else [[likely]]
        {
            // try find by iterator first
            scope.pending_value = scope.data_obj.iter.move_next_key(key);

            // retry
            if (!scope.pending_value) [[unlikely]]
            {
                // update retry count
                scope.data_obj.retry_count++;

                // reset iterator and find again
                scope.data_obj.iter = scope.value.get_value()->object_iter();
                scope.pending_value = scope.data_obj.iter.move_next_key(key);

                // reset iterator for next search if not found
                if (!scope.pending_value) [[unlikely]]
                {
                    scope.data_obj.iter = scope.value.get_value()->object_iter();
                }
            }
        }
    }
}
} // namespace skr

// impl ArWriteInplace
namespace skr
{
// ctor & dtor
ArWriteJsonInplace::ArWriteJsonInplace()
{
    _is_structured = true;

    _enable_string_enum = true;
    _enable_string_guid = true;
    _enable_string_md5 = true;
    _enable_string_sha256 = true;
}
ArWriteJsonInplace::~ArWriteJsonInplace()
{
}

// structured api
void ArWriteJsonInplace::impl_key(StringView key)
{
    // check set key environment
    if (_has_key) [[unlikely]]
    { // key called twice
        error(u8"[ArWriteJsonInplace] cannot set key twice without using it");
        return;
    }
    if (_scope_stack.is_empty()) [[unlikely]]
    { // call when no scope
        error(u8"[ArWriteJsonInplace] cannot set key at root scope");
        return;
    }
    else if (_scope_stack.back().is_array()) [[unlikely]]
    { // call in array scope
        error(u8"[ArWriteJsonInplace] cannot set key in array scope");
        return;
    }

    // setup key
    _set_key(key, false);
}
void ArWriteJsonInplace::impl_key_copy(StringView key)
{
    // check set key environment
    if (_has_key) [[unlikely]]
    { // key called twice
        error(u8"[ArWriteJsonInplace] cannot set key twice without using it");
        return;
    }
    if (_scope_stack.is_empty()) [[unlikely]]
    { // call when no scope
        error(u8"[ArWriteJsonInplace] cannot set key at root scope");
        return;
    }
    else if (_scope_stack.back().is_array()) [[unlikely]]
    { // call in array scope
        error(u8"[ArWriteJsonInplace] cannot set key in array scope");
        return;
    }

    // setup key
    _set_key(key, true);
}
void ArWriteJsonInplace::impl_begin_array()
{
    SKR_FAST_CHECK(_check_key_state(), );
    _push_scope(EScopeKind::Array);
}
void ArWriteJsonInplace::impl_end_array()
{
    if (_scope_stack.is_empty()) [[unlikely]]
    { // miss begin_array
        error(u8"[ArWriteJsonInplace] bad end_array() miss begin_array()");
        return;
    }
    if (!_scope_stack.back().is_array()) [[unlikely]]
    { // bad scope
        error(u8"[ArWriteJsonInplace] bad end_array() not in array scope");
        return;
    }

    _pop_scope();
}
void ArWriteJsonInplace::impl_begin_object()
{
    SKR_FAST_CHECK(_check_key_state(), );
    _push_scope(EScopeKind::Object);
}
void ArWriteJsonInplace::impl_end_object()
{
    if (_scope_stack.is_empty()) [[unlikely]]
    { // miss begin_object
        error(u8"[ArWriteJsonInplace] bad end_object() miss begin_object()");
        return;
    }
    if (!_scope_stack.back().is_object()) [[unlikely]]
    { // bad scope
        error(u8"[ArWriteJsonInplace] bad end_object() not in object scope");
        return;
    }

    _pop_scope();
}

// typed data
void ArWriteJsonInplace::impl_primitive(EArchivePrimitiveType type, const void* data)
{
    SKR_FAST_CHECK(_check_key_state(), );
    if (_scope_stack.is_empty()) [[unlikely]]
    { // cannot write at root scope
        error(u8"[ArWriteJsonInplace] cannot write primitive at root scope");
        return;
    }

    _prepare_for_write_value();
    switch (type)
    {
    case EArchivePrimitiveType::Bool:
        result.append(*(const bool*)data ? u8"true" : u8"false");
        break;
    case EArchivePrimitiveType::Int8:
        skr::container::Formatter<int8_t>::format(
            result, *(const int8_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::Int16:
        skr::container::Formatter<int16_t>::format(
            result, *(const int16_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::Int32:
        skr::container::Formatter<int32_t>::format(
            result, *(const int32_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::Int64:
        skr::container::Formatter<int64_t>::format(
            result, *(const int64_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::UInt8:
        skr::container::Formatter<uint8_t>::format(
            result, *(const uint8_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::UInt16:
        skr::container::Formatter<uint16_t>::format(
            result, *(const uint16_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::UInt32:
        skr::container::Formatter<uint32_t>::format(
            result, *(const uint32_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::UInt64:
        skr::container::Formatter<uint64_t>::format(
            result, *(const uint64_t*)data, {}
        );
        break;
    case EArchivePrimitiveType::Float:
        skr::container::Formatter<float>::format(
            result, *(const float*)data, {}
        );
        break;
    case EArchivePrimitiveType::Double:
        skr::container::Formatter<double>::format(
            result, *(const double*)data, {}
        );
        break;
    default:
        error(u8"[ArWriteJsonInplace] does not support this primitive type");
        return;
    }
}
void ArWriteJsonInplace::impl_str(StringView str)
{
    SKR_FAST_CHECK(_check_key_state(), );
    if (_scope_stack.is_empty()) [[unlikely]]
    {
        error(u8"[ArWriteJsonInplace] cannot write string at root scope");
        return;
    }

    _prepare_for_write_value();
    result.append(u8"\"");
    result.append(str);
    result.append(u8"\"");
}

// binary data support
void ArWriteJsonInplace::impl_bytes(const void* data, uint64_t size)
{
    error(u8"ArWriteJsonInplace does not support impl_bytes");
}
void ArWriteJsonInplace::impl_bits(const void* data, uint64_t size, uint64_t offset)
{
    error(u8"ArWriteJsonInplace does not support impl_bits");
}

// write help
bool ArWriteJsonInplace::write_to_file(const Path& path)
{
    return fs::File::write_all_text(path, result);
}

// reset state for next use
void ArWriteJsonInplace::reset()
{
    _scope_stack.clear();
    _reset_key_state();
    _error_tracker.reset();
}
void ArWriteJsonInplace::reset_string()
{
    result.clear();
}
void ArWriteJsonInplace::reset_all()
{
    reset();
    reset_string();
}

// checks
bool ArWriteJsonInplace::_check_key_state()
{
    if (!_scope_stack.is_empty()) [[likely]]
    {
        auto& cur_scope = _scope_stack.back();
        if (cur_scope.is_object() && !_has_key) [[unlikely]]
        {
            error(u8"ArWriteJsonInplace must set key before writing value in object scope");
            return false;
        }
        // array scope checked in impl_key
    }
    return true;
}

// key operations
void ArWriteJsonInplace::_set_key(StringView key, bool need_copy)
{
    _has_key = true;
    _key_is_literal = !need_copy;
    if (_key_is_literal)
    {
        _literal_key = key;
    }
    else
    {
        _owned_key = key;
    }
}
void ArWriteJsonInplace::_reset_key_state()
{
    _has_key = false;
    _literal_key = {};
    _owned_key.clear();
}
void ArWriteJsonInplace::_prepare_for_write_value()
{
    if (!_scope_stack.is_empty()) [[likely]]
    {
        auto& cur_scope = _scope_stack.back();
        SKR_DEFER({ _reset_key_state(); });

        if (cur_scope.is_array())
        {
            // write front ','
            if (cur_scope.index > 0) result.append(u8",");
            cur_scope.index++;
        }
        else if (cur_scope.is_object())
        {
            // write front ',
            if (cur_scope.index > 0) result.append(u8",");
            cur_scope.index++;

            // write key
            result.append(u8"\"");
            if (_key_is_literal)
            {
                result.append(_literal_key);
            }
            else
            {
                result.append(_owned_key);
            }
            result.append(u8"\":");
        }
    }
}

// scope operations
void ArWriteJsonInplace::_push_scope(EScopeKind kind)
{
    _prepare_for_write_value();

    // append scope mark
    if (kind == EScopeKind::Array)
        result.append(u8"[");
    else
        result.append(u8"{");

    _scope_stack.push_back({ kind, 0 });
}
void ArWriteJsonInplace::_pop_scope()
{
    auto& cur_scope = _scope_stack.back();

    // append scope mark
    if (cur_scope.is_array())
    {
        result.append(u8"]");
    }
    else if (cur_scope.is_object())
    {
        result.append(u8"}");
    }

    _scope_stack.pop_back();
}
} // namespace skr