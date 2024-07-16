#include "SkrBase/misc/defer.hpp"
#include "SkrArchive/json/writer.h"
#include "cstr_builder.hpp"

#define SKR_RET_JSON_WRITE_ERROR_IF(cond, what) { if (!(cond)) { return what; } }
#define SKR_RET_WRITE_RESULT_WITH_BOOL(cond) { if (!(cond)) { return JsonErrorCode::UnknownError; } return {}; }

namespace skr::archive {

struct _WriterHelper
{
    using CharType = _JsonWriter::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveWritableType T>
    static JsonWriteResult StartArray(_JsonWriter* w, skr::StringView key, const T* values, _JsonWriter::SizeType count)
    {
        using Level = _JsonWriter::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_WRITE_ERROR_IF(!w->_stack.empty(), JsonErrorCode::NoOpenScope)

        yyjson_mut_val* arr = nullptr;
        if (count != 0)
        {
            IS_TYPE(uint32_t)
                arr = yyjson_mut_arr_with_uint32((yyjson_mut_doc*)w->_document, values, count);
            IS_TYPE(uint64_t)
                arr = yyjson_mut_arr_with_uint64((yyjson_mut_doc*)w->_document, values, count);
            
            IS_TYPE(int32_t)
                arr = yyjson_mut_arr_with_sint32((yyjson_mut_doc*)w->_document, values, count);
            IS_TYPE(int64_t)
                arr = yyjson_mut_arr_with_sint64((yyjson_mut_doc*)w->_document, values, count);
            
            IS_TYPE(float)
                arr = yyjson_mut_arr_with_float((yyjson_mut_doc*)w->_document, values, count);
            IS_TYPE(double)
                arr = yyjson_mut_arr_with_real((yyjson_mut_doc*)w->_document, values, count);
        } 
        else {
            arr = yyjson_mut_arr((yyjson_mut_doc*)w->_document);
        } 

        if (w->_stack.back()._type == Level::kArray)
        {
            SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), JsonErrorCode::ArrayElementWithKey);
            yyjson_mut_arr_add_val((yyjson_mut_val*)w->_stack.back()._value, arr);
        }
        else if (w->_stack.back()._type == Level::kObject)
        {
            SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), JsonErrorCode::EmptyObjectFieldKey);
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();
            yyjson_mut_obj_add_val((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)w->_stack.back()._value, ckey, arr);
        }

        w->_stack.emplace((_JsonWriter::ValueType*)arr, Level::kArray);
        return {};
    }

    template <JsonPrimitiveWritableType T>
    static JsonWriteResult WriteValue(_JsonWriter* w, skr::StringView key, const T& value)
    {
        using Level = _JsonWriter::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_WRITE_ERROR_IF(!w->_stack.empty(), JsonErrorCode::NoOpenScope);

        auto type = w->_stack.back()._type;
        auto object = (yyjson_mut_val*)w->_stack.back()._value;
        if (type == Level::kObject)
        {
            SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), JsonErrorCode::EmptyObjectFieldKey);
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();
            bool success = false;

            IS_TYPE(_JsonWriter::ValueType*)
                success = yyjson_mut_obj_add_val((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, (yyjson_mut_val*)value);
            IS_TYPE(bool)
                success = yyjson_mut_obj_add_bool((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(int32_t)
                success = yyjson_mut_obj_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(int64_t)
                success = yyjson_mut_obj_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(uint32_t)
                success = yyjson_mut_obj_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(uint64_t)
                success = yyjson_mut_obj_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(float)
                success = yyjson_mut_obj_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(double)
                success = yyjson_mut_obj_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(skr::String)
                success = yyjson_mut_obj_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value.c_str(), value.raw().size());
            IS_TYPE(skr::StringView)
                success = yyjson_mut_obj_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, (const char*)value.raw().data(), value.raw().size());
            
            SKR_RET_WRITE_RESULT_WITH_BOOL(success);
        }
        else if (type == Level::kArray)
        {
            bool success = false;

            SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), JsonErrorCode::ArrayElementWithKey);
            IS_TYPE(_JsonWriter::ValueType*)
                success = yyjson_mut_arr_add_val((yyjson_mut_val*)object, (yyjson_mut_val*)value);
            IS_TYPE(bool)
                success = yyjson_mut_arr_add_bool((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(int32_t)
                success = yyjson_mut_arr_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(int64_t)
                success = yyjson_mut_arr_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(uint32_t)
                success = yyjson_mut_arr_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(uint64_t)
                success = yyjson_mut_arr_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(float)
                success = yyjson_mut_arr_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(double)
                success = yyjson_mut_arr_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(skr::String)
                success = yyjson_mut_arr_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value.c_str(), value.raw().size());
            IS_TYPE(skr::StringView)
                success = yyjson_mut_arr_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, (const char*)value.raw().data(), value.raw().size());
            
            SKR_RET_WRITE_RESULT_WITH_BOOL(success);
        }
        return JsonErrorCode::UnknownError;
    }

#undef IS_TYPE
};

_JsonWriter::_JsonWriter(size_t levelDepth)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    _document = (SJsonMutableDocument*)doc;
    _stack.reserve(levelDepth);
}

_JsonWriter::~_JsonWriter()
{
    yyjson_mut_doc_free((yyjson_mut_doc*)_document);
}

JsonWriteResult _JsonWriter::StartObject(skr::StringView key)
{
    yyjson_mut_val *obj = yyjson_mut_obj((yyjson_mut_doc*)_document);
    if (_stack.empty())
    {
        SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), JsonErrorCode::RootObjectWithKey);
        yyjson_mut_doc_set_root((yyjson_mut_doc*)_document, obj);
    }
    else if (_stack.back()._type == Level::kArray)
    {
        SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), JsonErrorCode::ArrayElementWithKey);
        yyjson_mut_arr_add_val((yyjson_mut_val*)_stack.back()._value, obj);
    }
    else if (_stack.back()._type == Level::kObject)
    {
        SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), JsonErrorCode::EmptyObjectFieldKey);
        CStringBuilder keyBuilder((yyjson_mut_doc*)_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_mut_obj_add_val((yyjson_mut_doc*)_document, (yyjson_mut_val*)_stack.back()._value, ckey, obj);
    }
    _stack.emplace((ValueType*)obj, Level::kObject);
    return {};
}

JsonWriteResult _JsonWriter::EndObject()
{
    SKR_RET_JSON_WRITE_ERROR_IF(!_stack.empty(), JsonErrorCode::NoOpenScope);
    SKR_RET_JSON_WRITE_ERROR_IF(_stack.back()._type == Level::kObject, JsonErrorCode::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const float* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const double* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key)
{
    const char* dummy = nullptr;
    return _WriterHelper::StartArray(this, key, dummy, 0);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const int32_t* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const int64_t* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const uint32_t* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::StartArray(skr::StringView key, const uint64_t* values, _JsonWriter::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

JsonWriteResult _JsonWriter::EndArray()
{
    SKR_RET_JSON_WRITE_ERROR_IF(!_stack.empty(), JsonErrorCode::NoOpenScope);
    SKR_RET_JSON_WRITE_ERROR_IF(_stack.back()._type == Level::kArray, JsonErrorCode::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

/*
JsonWriteResult _JsonWriter::WriteValue(skr::StringView key, ValueType* value)
{
    return _WriterHelper::WriteValue(this, key, value);
}
*/

JsonWriteResult _JsonWriter::WriteBool(skr::StringView key, bool value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteInt32(skr::StringView key, int32_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteInt64(skr::StringView key, int64_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteUInt32(skr::StringView key, uint32_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteUInt64(skr::StringView key, uint64_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteFloat(skr::StringView key, float value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteDouble(skr::StringView key, double value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteString(skr::StringView key, skr::StringView value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

JsonWriteResult _JsonWriter::WriteString(skr::StringView key, const skr::String& value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

skr::String _JsonWriter::Write()
{
    yyjson_mut_doc* doc = (yyjson_mut_doc*)_document;
    auto str = yyjson_mut_write(doc, 0, NULL);
    auto result = skr::String((const char8_t*)str);
    ::free(str);
    return result;
}

JsonWriter::JsonWriter(size_t levelDepth)
    : _JsonWriter(levelDepth)
{

}

JsonWriteResult JsonWriter::Key(skr::StringView key)
{
    SKR_RET_JSON_WRITE_ERROR_IF(_currentKey.is_empty(), JsonErrorCode::PresetKeyNotConsumedYet);
    SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), JsonErrorCode::PresetKeyIsEmpty);
    _currentKey = key;
    return {};
}

JsonWriteResult JsonWriter::Bool(bool value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteBool(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::Int32(int32_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteInt32(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::Int64(int64_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteInt64(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::UInt32(uint32_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteUInt32(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::UInt64(uint64_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteUInt64(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::Float(float value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteFloat(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::Double(double value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteDouble(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::String(skr::StringView value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteString(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::String(const skr::String& value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteString(_currentKey.view(), value);
}

JsonWriteResult JsonWriter::StartArray()
{
    SKR_DEFER({ _currentKey.empty(); });
    return _JsonWriter::StartArray(_currentKey.view());
}

JsonWriteResult JsonWriter::StartObject()
{
    SKR_DEFER({ _currentKey.empty(); });
    return _JsonWriter::StartObject(_currentKey.view());
}

}

#undef SKR_RET_WRITE_RESULT_WITH_BOOL
#undef SKR_RET_JSON_WRITE_ERROR_IF