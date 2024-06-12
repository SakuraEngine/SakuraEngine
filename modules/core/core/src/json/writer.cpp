#include "SkrBase/misc/defer.hpp"
#include "SkrJson/writer.h"
#include "cstr_builder.hpp"

#define SKR_RET_JSON_WRITE_ERROR_IF(cond, what) { if (!(cond)) { return what; } }
#define SKR_RET_WRITE_RESULT_WITH_BOOL(cond) { if (!(cond)) { return EWriteError::UnknownError; } return {}; }

namespace skr::json {

struct _WriterHelper
{
    using CharType = _Writer::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveWritableType T>
    static skr::json::WriteResult StartArray(_Writer* w, skr::StringView key, const T* values, _Writer::SizeType count)
    {
        using Level = _Writer::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_WRITE_ERROR_IF(!w->_stack.empty(), EWriteError::NoOpenScope)

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
            SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), EWriteError::ArrayElementWithKey);
            yyjson_mut_arr_add_val((yyjson_mut_val*)w->_stack.back()._value, arr);
        }
        else if (w->_stack.back()._type == Level::kObject)
        {
            SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), EWriteError::EmptyObjectFieldKey);
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();
            yyjson_mut_obj_add_val((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)w->_stack.back()._value, ckey, arr);
        }

        w->_stack.emplace((_Writer::ValueType*)arr, Level::kArray);
        return {};
    }

    template <JsonPrimitiveWritableType T>
    static WriteResult WriteValue(_Writer* w, skr::StringView key, const T& value)
    {
        using Level = _Writer::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_WRITE_ERROR_IF(!w->_stack.empty(), EWriteError::NoOpenScope);

        auto type = w->_stack.back()._type;
        auto object = (yyjson_mut_val*)w->_stack.back()._value;
        if (type == Level::kObject)
        {
            SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), EWriteError::EmptyObjectFieldKey);
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();
            bool success = false;

            IS_TYPE(_Writer::ValueType*)
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

            SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), EWriteError::ArrayElementWithKey);
            IS_TYPE(_Writer::ValueType*)
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
        return EWriteError::UnknownError;
    }

#undef IS_TYPE
};

_Writer::_Writer(size_t levelDepth)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    _document = (SJsonMutableDocument*)doc;
    _stack.reserve(levelDepth);
}

_Writer::~_Writer()
{
    yyjson_mut_doc_free((yyjson_mut_doc*)_document);
}

WriteResult _Writer::StartObject(skr::StringView key)
{
    yyjson_mut_val *obj = yyjson_mut_obj((yyjson_mut_doc*)_document);
    if (_stack.empty())
    {
        SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), EWriteError::RootObjectWithKey);
        yyjson_mut_doc_set_root((yyjson_mut_doc*)_document, obj);
    }
    else if (_stack.back()._type == Level::kArray)
    {
        SKR_RET_JSON_WRITE_ERROR_IF(key.is_empty(), EWriteError::ArrayElementWithKey);
        yyjson_mut_arr_add_val((yyjson_mut_val*)_stack.back()._value, obj);
    }
    else if (_stack.back()._type == Level::kObject)
    {
        SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), EWriteError::EmptyObjectFieldKey);
        CStringBuilder keyBuilder((yyjson_mut_doc*)_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_mut_obj_add_val((yyjson_mut_doc*)_document, (yyjson_mut_val*)_stack.back()._value, ckey, obj);
    }
    _stack.emplace((ValueType*)obj, Level::kObject);
    return {};
}

WriteResult _Writer::EndObject()
{
    SKR_RET_JSON_WRITE_ERROR_IF(!_stack.empty(), EWriteError::NoOpenScope);
    SKR_RET_JSON_WRITE_ERROR_IF(_stack.back()._type == Level::kObject, EWriteError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

WriteResult _Writer::StartArray(skr::StringView key, const float* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::StartArray(skr::StringView key, const double* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::StartArray(skr::StringView key)
{
    const char* dummy = nullptr;
    return _WriterHelper::StartArray(this, key, dummy, 0);
}

WriteResult _Writer::StartArray(skr::StringView key, const int32_t* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::StartArray(skr::StringView key, const int64_t* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::StartArray(skr::StringView key, const uint32_t* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::StartArray(skr::StringView key, const uint64_t* values, _Writer::SizeType count)
{
    return _WriterHelper::StartArray(this, key, values, count);
}

WriteResult _Writer::EndArray()
{
    SKR_RET_JSON_WRITE_ERROR_IF(!_stack.empty(), EWriteError::NoOpenScope);
    SKR_RET_JSON_WRITE_ERROR_IF(_stack.back()._type == Level::kArray, EWriteError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

/*
WriteResult _Writer::WriteValue(skr::StringView key, ValueType* value)
{
    return _WriterHelper::WriteValue(this, key, value);
}
*/

WriteResult _Writer::WriteBool(skr::StringView key, bool value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteInt32(skr::StringView key, int32_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteInt64(skr::StringView key, int64_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteUInt32(skr::StringView key, uint32_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteUInt64(skr::StringView key, uint64_t value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteFloat(skr::StringView key, float value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteDouble(skr::StringView key, double value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteString(skr::StringView key, skr::StringView value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

WriteResult _Writer::WriteString(skr::StringView key, const skr::String& value)
{
    return _WriterHelper::WriteValue(this, key, value);
}

skr::String _Writer::Write()
{
    yyjson_mut_doc* doc = (yyjson_mut_doc*)_document;
    auto str = yyjson_mut_write(doc, 0, NULL);
    auto result = skr::String((const char8_t*)str);
    ::free(str);
    return result;
}

Writer::Writer(size_t levelDepth)
    : _Writer(levelDepth)
{

}

WriteResult Writer::Key(skr::StringView key)
{
    SKR_RET_JSON_WRITE_ERROR_IF(_currentKey.is_empty(), EWriteError::PresetKeyNotConsumedYet);
    SKR_RET_JSON_WRITE_ERROR_IF(!key.is_empty(), EWriteError::PresetKeyIsEmpty);
    _currentKey = key;
    return {};
}

WriteResult Writer::Bool(bool value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteBool(_currentKey.view(), value);
}

WriteResult Writer::Int32(int32_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteInt32(_currentKey.view(), value);
}

WriteResult Writer::Int64(int64_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteInt64(_currentKey.view(), value);
}

WriteResult Writer::UInt32(uint32_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteUInt32(_currentKey.view(), value);
}

WriteResult Writer::UInt64(uint64_t value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteUInt64(_currentKey.view(), value);
}

WriteResult Writer::Float(float value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteFloat(_currentKey.view(), value);
}

WriteResult Writer::Double(double value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteDouble(_currentKey.view(), value);
}

WriteResult Writer::String(skr::StringView value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteString(_currentKey.view(), value);
}

WriteResult Writer::String(const skr::String& value)
{
    SKR_DEFER({ _currentKey.empty(); });
    return WriteString(_currentKey.view(), value);
}

WriteResult Writer::StartArray()
{
    SKR_DEFER({ _currentKey.empty(); });
    return _Writer::StartArray(_currentKey.view());
}

WriteResult Writer::StartObject()
{
    SKR_DEFER({ _currentKey.empty(); });
    return _Writer::StartObject(_currentKey.view());
}

}

#undef SKR_RET_WRITE_RESULT_WITH_BOOL
#undef SKR_RET_JSON_WRITE_ERROR_IF