#include "SkrBase/misc/defer.hpp"
#include "SkrArchive/json/reader.h"
#include "cstr_builder.hpp"
#include "SkrCore/log.h"

#define SKR_RET_JSON_READ_ERROR_IF(cond, what) { if (!(cond)) { return what; } }

namespace skr::json {

struct _ReaderHelper
{
    using CharType = _Reader::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveReadableType T>
    static skr::json::ReadResult ReadValue(_Reader* r, skr::StringView key, T& value) 
    {
        using Level = _Reader::Level;
        using Type = std::decay_t<T>;
        SKR_RET_JSON_READ_ERROR_IF(!r->_stack.empty(), EReadError::NoOpenScope);

        auto type = r->_stack.back()._type;
        auto parent = (yyjson_val*)r->_stack.back()._value;
        CStringBuilder keyBuilder((yyjson_doc*)r->_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_val* found = nullptr;
        if (type == Level::kObject)
        {
            SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), skr::json::EReadError::EmptyObjectFieldKey);
            found = yyjson_obj_get((yyjson_val*)parent, ckey);
            SKR_RET_JSON_READ_ERROR_IF(found, skr::json::EReadError::KeyNotFound);
        }
        else if (type == Level::kArray)
        {
            SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), skr::json::EReadError::ArrayElementWithKey);
            found = yyjson_arr_get((yyjson_val*)parent, r->_stack.back()._index++);
            SKR_RET_JSON_READ_ERROR_IF(found, skr::json::EReadError::KeyNotFound);
        }

        IS_TYPE(_Reader::ValueType*)
            value = (_Reader::ValueType*)found;
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
            return skr::json::EReadError::UnknownTypeToRead;

        return {};
    }

#undef IS_TYPE
};

_Reader::_Reader(skr::StringView json)
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

_Reader::~_Reader()
{
    if (_document)
        yyjson_doc_free((yyjson_doc*)_document);
}

ReadResult _Reader::StartObject(skr::StringView key)
{
    if (_stack.empty())
    {
        SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), EReadError::RootObjectWithKey);
        auto obj = yyjson_doc_get_root((yyjson_doc*)_document);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, EReadError::ScopeTypeMismatch);
        _stack.emplace((ValueType*)obj, Level::kObject);
    }
    else
    {
        auto parent = (yyjson_val*)_stack.back()._value;
        auto parent_type = yyjson_get_type(parent);
        if (parent_type == YYJSON_TYPE_ARR)
        {
            SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), EReadError::ArrayElementWithKey);
            auto obj = yyjson_arr_get(parent, _stack.back()._index++);
            SKR_RET_JSON_READ_ERROR_IF(obj, EReadError::KeyNotFound);
            SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, EReadError::ScopeTypeMismatch);
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else if (parent_type == YYJSON_TYPE_OBJ)
        {
            SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), EReadError::EmptyObjectFieldKey);
            auto obj = yyjson_obj_get(parent, (const char*)key.raw().data());
            SKR_RET_JSON_READ_ERROR_IF(obj, EReadError::KeyNotFound);
            SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, EReadError::ScopeTypeMismatch);
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else
            SKR_RET_JSON_READ_ERROR_IF(false, EReadError::UnknownError);
    }
    return {};
}

ReadResult _Reader::EndObject()
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), EReadError::NoOpenScope);
    SKR_RET_JSON_READ_ERROR_IF(_stack.back()._type == Level::kObject, EReadError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

ReadResult _Reader::StartArray(skr::StringView key, SizeType& count)
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), EReadError::NoOpenScope);
    auto parent = (yyjson_val*)_stack.back()._value;
    auto parent_type = yyjson_get_type(parent);
    if (parent_type == YYJSON_TYPE_ARR)
    {
        SKR_RET_JSON_READ_ERROR_IF(key.is_empty(), EReadError::ArrayElementWithKey);
        auto arr = yyjson_arr_get(parent, _stack.back()._index++);
        SKR_RET_JSON_READ_ERROR_IF(arr, EReadError::KeyNotFound);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(arr) == YYJSON_TYPE_ARR, EReadError::ScopeTypeMismatch);
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    else if (parent_type == YYJSON_TYPE_OBJ)
    {
        SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), EReadError::EmptyObjectFieldKey);
        auto arr = yyjson_obj_get(parent, (const char*)key.raw().data());
        SKR_RET_JSON_READ_ERROR_IF(arr, EReadError::KeyNotFound);
        SKR_RET_JSON_READ_ERROR_IF(yyjson_get_type(arr) == YYJSON_TYPE_ARR, EReadError::ScopeTypeMismatch);
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    else
        SKR_RET_JSON_READ_ERROR_IF(false, EReadError::UnknownError);
    return {};
}

ReadResult _Reader::EndArray()
{
    SKR_RET_JSON_READ_ERROR_IF(!_stack.empty(), EReadError::NoOpenScope);
    SKR_RET_JSON_READ_ERROR_IF(_stack.back()._type == Level::kArray, EReadError::ScopeTypeMismatch);
    _stack.pop_back();
    return {};
}

ReadResult _Reader::ReadBool(skr::StringView key, bool& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadInt32(skr::StringView key, int32_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadInt64(skr::StringView key, int64_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadUInt32(skr::StringView key, uint32_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadUInt64(skr::StringView key, uint64_t& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadFloat(skr::StringView key, float& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadDouble(skr::StringView key, double& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

ReadResult _Reader::ReadString(skr::StringView key, skr::String& value)
{
    return _ReaderHelper::ReadValue(this, key, value);
}

Reader::Reader(skr::StringView json)
    : _Reader(json)
{

}

ReadResult Reader::Key(skr::StringView key)
{
    SKR_RET_JSON_READ_ERROR_IF(_currentKey.is_empty(), EReadError::PresetKeyNotConsumedYet);
    SKR_RET_JSON_READ_ERROR_IF(!key.is_empty(), EReadError::PresetKeyIsEmpty);
    _currentKey = key;
    return {};
}

ReadResult Reader::Bool(bool& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadBool(_currentKey.view(), value);
}

ReadResult Reader::Int32(int32_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadInt32(_currentKey.view(), value);
}

ReadResult Reader::Int64(int64_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadInt64(_currentKey.view(), value);
}

ReadResult Reader::UInt32(uint32_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadUInt32(_currentKey.view(), value);
}

ReadResult Reader::UInt64(uint64_t& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadUInt64(_currentKey.view(), value);
}

ReadResult Reader::Float(float& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadFloat(_currentKey.view(), value);
}

ReadResult Reader::Double(double& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadDouble(_currentKey.view(), value);
}

ReadResult Reader::String(skr::String& value)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return ReadString(_currentKey.view(), value);
}

ReadResult Reader::StartArray(SizeType& count)
{
    SKR_DEFER( { _currentKey.empty(); } );
    return _Reader::StartArray(_currentKey.view(), count);
}

ReadResult Reader::StartObject()
{
    SKR_DEFER( { _currentKey.empty(); } );
    return _Reader::StartObject(_currentKey.view());
}

}

#undef SKR_RET_JSON_READ_ERROR_IF