#include "SkrBase/misc/defer.hpp"
#include "SkrArchive/json/reader.h"
#include "cstr_builder.hpp"
#include "SkrCore/log.h"

#define SKR_RET_JSON_READ_ERROR_IF(cond, what) { if (!(cond)) { return what; } }

namespace skr::archive {

struct _ReaderHelper
{
    using CharType = _JsonReader::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveReadableType T>
    static JsonReadResult ReadValue(_JsonReader* r, skr::StringView key, T& value) 
    {
        using Level = _JsonReader::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_READ_ERROR_IF(!r->_stack.empty(), JsonReadError::NoOpenScope);

        auto type = r->_stack.back()._type;
        auto parent = (yyjson_val*)r->_stack.back()._value;
        CStringBuilder keyBuilder((yyjson_doc*)r->_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_val* found = nullptr;
        if (type == Level::kObject)
        {
            SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), JsonReadError::EmptyObjectFieldKey);
            found = yyjson_obj_get((yyjson_val*)parent, ckey);
            SKR_RET_JSON_READ_ERROR_IF(found, JsonReadError::KeyNotFound);
        }
        else if (type == Level::kArray)
        {
            SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), JsonReadError::ArrayElementWithKey);
            found = yyjson_arr_get((yyjson_val*)parent, r->_stack.back()._index++);
            SKR_RET_JSON_READ_ERROR_IF(found, JsonReadError::KeyNotFound);
        }

        IS_TYPE(_JsonReader::ValueType*)
            value = (_JsonReader::ValueType*)found;
        else IS_TYPE(bool)
            value = yyjson_get_bool(found);
        else IS_TYPE(int32_t)
            value = yyjson_get_int(found);
        else IS_TYPE(int64_t)
            value = yyjson_get_sint(found);
        else IS_TYPE(uint32_t)
            value = yyjson_get_uint(found);
        else IS_TYPE(uint64_t)
            value = yyjson_get_uint(found);
        else IS_TYPE(float)
            value = yyjson_get_real(found);
        else IS_TYPE(double)
            value = yyjson_get_real(found);
        else IS_TYPE(skr::String)
            value = skr::String((const char8_t*)yyjson_get_str(found));
        else
            return JsonReadError::UnknownTypeToRead;

        return {};
    }

#undef IS_TYPE
};

_JsonReader::_JsonReader(skr::StringView json)
{
    yyjson_read_err err = {};
    _document = (DocumentType*)yyjson_read_opts(
        (char*)json.raw().data(), json.raw().size(), 
        0, nullptr, &err);
    if (_document == nullptr)
    {
        SKR_LOG_ERROR(u8"Failed to parse JSON: %s, error: %s", json.raw().data(), err.msg);
    }
}

_JsonReader::~_JsonReader()
{
    if (_document)
        yyjson_doc_free((yyjson_doc*)_document);
}

JsonReadResult _JsonReader::StartObject(skr::StringView key)
{
    if (_stack.empty())
    {
        SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), JsonReadError::RootObjectWithKey);
        auto obj = yyjson_doc_get_root((yyjson_doc*)_document);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, JsonReadError::ScopeTypeMismatch);
        _stack.emplace((ValueType*)obj, Level::kObject);
    }
    else
    {
        auto parent = (yyjson_val*)_stack.back()._value;
        auto parent_type = yyjson_get_type(parent);
        if (parent_type == YYJSON_TYPE_ARR)
        {
            SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), JsonReadError::ArrayElementWithKey);
            auto obj = yyjson_arr_get(parent, _stack.back()._index++);
            SKR_RET_JSON_READ_ERROR_IF(obj, JsonReadError::KeyNotFound);
            SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, JsonReadError::ScopeTypeMismatch);
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else if (parent_type == YYJSON_TYPE_OBJ)
        {
            SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), JsonReadError::EmptyObjectFieldKey);
            auto obj = yyjson_obj_get(parent, (const char*)key.raw().data());
            SKR_RET_JSON_READ_ERROR_IF(obj, JsonReadError::KeyNotFound);
            SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, JsonReadError::ScopeTypeMismatch);
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else
            SKR_RET_JSON_READ_ERROR_IF(false, JsonReadError::UnknownError);
    }
    return {};
}

JsonReadResult _JsonReader::EndObject()
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), JsonReadError::NoOpenScope);
    SKR_RET_JSON_READ_ERROR_IF(_stack.back()._type == Level::kObject, JsonReadError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

JsonReadResult _JsonReader::StartArray(skr::StringView key, SizeType& count)
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), JsonReadError::NoOpenScope);
    auto parent = (yyjson_val*)_stack.back()._value;
    auto parent_type = yyjson_get_type(parent);
    if (parent_type == YYJSON_TYPE_ARR)
    {
        SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), JsonReadError::ArrayElementWithKey);
        auto arr = yyjson_arr_get(parent, _stack.back()._index++);
        SKR_RET_JSON_READ_ERROR_IF(arr, JsonReadError::KeyNotFound);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(arr) == YYJSON_TYPE_ARR, JsonReadError::ScopeTypeMismatch);
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    else if (parent_type == YYJSON_TYPE_OBJ)
    {
        SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), JsonReadError::EmptyObjectFieldKey);
        auto arr = yyjson_obj_get(parent, (const char*)key.raw().data());
        SKR_RET_JSON_READ_ERROR_IF(arr, JsonReadError::KeyNotFound);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(arr) == YYJSON_TYPE_ARR, JsonReadError::ScopeTypeMismatch);
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    else
        SKR_RET_JSON_READ_ERROR_IF(false, JsonReadError::UnknownError);
    return {};
}

JsonReadResult _JsonReader::EndArray()
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), JsonReadError::NoOpenScope);
    SKR_RET_JSON_READ_ERROR_IF(_stack.back()._type == Level::kArray, JsonReadError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

bool _JsonReader::HasKey(skr::StringView key)
{
    auto type = _stack.back()._type;
    auto parent = (yyjson_val*)_stack.back()._value;
    CStringBuilder keyBuilder((yyjson_doc*)_document, key);
    const char* ckey = keyBuilder.c_str();
    if (type == Level::kObject)
    {
        return yyjson_obj_get((yyjson_val*)parent, ckey);
    }
    return false;
}

JsonReadResult _JsonReader::ReadBool(skr::StringView key, bool& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadInt32(skr::StringView key, int32_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadInt64(skr::StringView key, int64_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadUInt32(skr::StringView key, uint32_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadUInt64(skr::StringView key, uint64_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadFloat(skr::StringView key, float& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadDouble(skr::StringView key, double& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReadResult _JsonReader::ReadString(skr::StringView key, skr::String& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

JsonReader::JsonReader(skr::StringView json)
    : _JsonReader(json)
{

}

JsonReadResult JsonReader::Key(skr::StringView key)
{
    SKR_RET_JSON_READ_ERROR_IF(_currentKey.is_empty(), JsonReadError::PresetKeyNotConsumedYet);
    SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), JsonReadError::PresetKeyIsEmpty);
    if (!_JsonReader::HasKey(key))
        return JsonReadError::KeyNotFound;
    _currentKey = key;
    return {};
}

JsonReadResult JsonReader::Bool(bool& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadBool(_currentKey.view(), value);
}

JsonReadResult JsonReader::Int32(int32_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadInt32(_currentKey.view(), value);
}

JsonReadResult JsonReader::Int64(int64_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadInt64(_currentKey.view(), value);
}

JsonReadResult JsonReader::UInt32(uint32_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadUInt32(_currentKey.view(), value);
}

JsonReadResult JsonReader::UInt64(uint64_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadUInt64(_currentKey.view(), value);
}

JsonReadResult JsonReader::Float(float& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadFloat(_currentKey.view(), value);
}

JsonReadResult JsonReader::Double(double& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadDouble(_currentKey.view(), value);
}

JsonReadResult JsonReader::String(skr::String& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadString(_currentKey.view(), value);
}

JsonReadResult JsonReader::StartArray(SizeType& count)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return _JsonReader::StartArray(_currentKey.view(), count);
}

JsonReadResult JsonReader::StartObject()
{
    SKR_DEFER( { _currentKey.empty(); } );
    return _JsonReader::StartObject(_currentKey.view());
}

}

#undef SKR_RET_JSON_READ_ERROR_IF