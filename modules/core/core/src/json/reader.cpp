#include "SkrJson/reader.h"
#include "cstr_builder.hpp"

#define SKR_ASSERT_RET_FALSE(cond, what) { if (!(cond)) { SKR_ASSERT(false && what); return false; } }

struct _SJsonReaderHelper
{
    using CharType = _SJsonReader::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveReadableType T>
    static bool ReadValue(_SJsonReader* r, skr::StringView key, T& value) 
    {
        using Level = _SJsonReader::Level;
        using Type = std::decay_t<T>;
        SKR_ASSERT(!r->_stack.empty() && "ReadValue() called without StartObject() or StartArray()");

        auto type = r->_stack.back()._type;
        auto parent = (yyjson_val*)r->_stack.back()._value;
        CStringBuilder keyBuilder((yyjson_doc*)r->_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_val* found = nullptr;
        if (type == Level::kObject)
        {
            SKR_ASSERT(!key.is_empty() && "ReadValue() must be called with key in object");
            found = yyjson_obj_get((yyjson_val*)parent, ckey);
            SKR_ASSERT_RET_FALSE(found, "Value not found");
        }
        else if (type == Level::kArray)
        {
            SKR_ASSERT(key.is_empty() && "ReadValue() must be called without key in array");
            found = yyjson_arr_get((yyjson_val*)parent, r->_stack.back()._index++);
            SKR_ASSERT_RET_FALSE(found, "Value not found");
        }

        IS_TYPE(_SJsonReader::ValueType*)
            value = (_SJsonReader::ValueType*)found;
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
            SKR_ASSERT_RET_FALSE(false, "ReadValue failed with unknown type");

        return true;
    }

#undef IS_TYPE
};

_SJsonReader::_SJsonReader(skr::StringView json)
{
    _document = (DocumentType*)yyjson_read((const char*)json.raw().data(), json.size(), 0);
}

_SJsonReader::~_SJsonReader()
{
    if (_document)
        yyjson_doc_free((yyjson_doc*)_document);
}

bool _SJsonReader::StartObject(skr::StringView key)
{
    if (_stack.empty())
    {
        SKR_ASSERT_RET_FALSE(key.is_empty(), "Root object should not have a key");
        auto obj = yyjson_doc_get_root((yyjson_doc*)_document);
        SKR_ASSERT_RET_FALSE(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, "Root object should be an object");
        _stack.emplace((ValueType*)obj, Level::kObject);
    }
    else
    {
        auto parent = (yyjson_val*)_stack.back()._value;
        auto parent_type = yyjson_get_type(parent);
        if (parent_type == YYJSON_TYPE_ARR)
        {
            SKR_ASSERT_RET_FALSE(key.is_empty(), "Key should be empty when StartObject within array");
            auto obj = yyjson_arr_get(parent, _stack.back()._index++);
            SKR_ASSERT_RET_FALSE(obj, "Object not found");
            SKR_ASSERT_RET_FALSE(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, "Object should be an object");
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else if (parent_type == YYJSON_TYPE_OBJ)
        {
            SKR_ASSERT_RET_FALSE(!key.is_empty(), "Key should not be empty when StartObject within object");
            auto obj = yyjson_obj_get(parent, (const char*)key.raw().data());
            SKR_ASSERT_RET_FALSE(obj, "Object not found");
            SKR_ASSERT_RET_FALSE(yyjson_get_type(obj) == YYJSON_TYPE_OBJ, "Object should be an object");
            _stack.emplace((ValueType*)obj, Level::kObject);
        }
        else
            SKR_ASSERT_RET_FALSE(false, "Can't start object within primitive types!");
    }
    return true;
}

bool _SJsonReader::EndObject()
{
    SKR_ASSERT_RET_FALSE(!_stack.empty(), "No object to end");
    SKR_ASSERT_RET_FALSE(_stack.back()._type == Level::kObject, "Not in an started object");
    _stack.pop_back();
    return true;
}

bool _SJsonReader::StartArray(skr::StringView key, SizeType& count)
{
    SKR_ASSERT_RET_FALSE(!_stack.empty(), "No object/array to start array");
    auto parent = (yyjson_val*)_stack.back()._value;
    if (_stack.back()._type == Level::kObject)
    {
        SKR_ASSERT_RET_FALSE(!key.is_empty(), "Key should not be empty when StartArray within object");
        auto arr = yyjson_obj_get(parent, (const char*)key.raw().data());
        SKR_ASSERT_RET_FALSE(arr, "Array not found");
        SKR_ASSERT_RET_FALSE(yyjson_get_type(arr) == YYJSON_TYPE_ARR, "Array should be an array");
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    else if (_stack.back()._type == Level::kArray)
    {
        auto arr = yyjson_arr_get(parent, _stack.back()._index++);
        SKR_ASSERT_RET_FALSE(arr, "Array not found");
        SKR_ASSERT_RET_FALSE(yyjson_get_type(arr) == YYJSON_TYPE_ARR, "Array should be an array");
        count = yyjson_arr_size(arr);
        _stack.emplace((ValueType*)arr, Level::kArray);
    }
    return true;
}

bool _SJsonReader::EndArray()
{
    SKR_ASSERT_RET_FALSE(!_stack.empty(), "No array to end");
    SKR_ASSERT_RET_FALSE(_stack.back()._type == Level::kArray, "Not in an started array");
    _stack.pop_back();
    return true;
}

bool _SJsonReader::ReadBool(skr::StringView key, bool& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadInt32(skr::StringView key, int32_t& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadInt64(skr::StringView key, int64_t& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadUInt32(skr::StringView key, uint32_t& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadUInt64(skr::StringView key, uint64_t& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadFloat(skr::StringView key, float& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadDouble(skr::StringView key, double& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

bool _SJsonReader::ReadString(skr::StringView key, skr::String& value)
{
    return _SJsonReaderHelper::ReadValue(this, key, value);
}

SJsonReader::SJsonReader(skr::StringView json)
    : _SJsonReader(json)
{

}

bool SJsonReader::Key(skr::StringView key)
{
    SKR_ASSERT_RET_FALSE(_currentKey.is_empty(), "Last key is not consumed yet!");
    SKR_ASSERT_RET_FALSE(!key.is_empty(), "key must not be empty!");
    _currentKey = key;
    return true;
}

bool SJsonReader::Bool(bool& value)
{
    bool r = ReadBool(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::Int32(int32_t& value)
{
    bool r = ReadInt32(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::Int64(int64_t& value)
{
    bool r = ReadInt64(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::UInt32(uint32_t& value)
{
    bool r = ReadUInt32(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::UInt64(uint64_t& value)
{
    bool r = ReadUInt64(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::Float(float& value)
{
    bool r = ReadFloat(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::Double(double& value)
{
    bool r = ReadDouble(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::String(skr::String& value)
{
    bool r = ReadString(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonReader::StartArray(SizeType& count)
{
    bool r = _SJsonReader::StartArray(_currentKey.view(), count);
    _currentKey.empty();
    return r;
}

bool SJsonReader::StartObject()
{
    bool r = _SJsonReader::StartObject(_currentKey.view());
    _currentKey.empty();
    return r;
}

#undef SKR_ASSERT_RET_FALSE