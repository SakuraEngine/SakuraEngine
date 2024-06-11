#include "SkrJson/writer.h"
#include "cstr_builder.hpp"

#define SKR_ASSERT_RET_FALSE(cond, what) { if (!(cond)) { SKR_ASSERT(false && what); return false; } }

struct _SJsonWriterHelper
{
    using CharType = _SJsonWriter::CharType;

#define IS_TYPE(T) if constexpr (std::is_same_v<Type, T>)

    template <JsonPrimitiveWritableType T>
    static bool StartArray(_SJsonWriter* w, skr::StringView key, const T* values, _SJsonWriter::SizeType count)
    {
        using Level = _SJsonWriter::Level;
        using Type = std::decay_t<T>;
        SKR_ASSERT_RET_FALSE(!w->_stack.empty(), "Root object should not be an array")

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
            SKR_ASSERT_RET_FALSE(key.is_empty(), "StartArray() must not be called with key in array");
            yyjson_mut_arr_add_val((yyjson_mut_val*)w->_stack.back()._value, arr);
        }
        else if (w->_stack.back()._type == Level::kObject)
        {
            SKR_ASSERT_RET_FALSE(!key.is_empty(), "StartArray() must be called with key in object");
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();
            yyjson_mut_obj_add_val((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)w->_stack.back()._value, ckey, arr);
        }

        w->_stack.emplace((_SJsonWriter::ValueType*)arr, Level::kArray);
        return true;
    }

    template <JsonPrimitiveWritableType T>
    static bool WriteValue(_SJsonWriter* w, skr::StringView key, const T& value)
    {
        using Level = _SJsonWriter::Level;
        using Type = std::decay_t<T>;
        SKR_ASSERT(!w->_stack.empty() && "WriteValue() called without StartObject() or StartArray()");

        auto type = w->_stack.back()._type;
        auto object = (yyjson_mut_val*)w->_stack.back()._value;
        if (type == Level::kObject)
        {
            SKR_ASSERT(!key.is_empty() && "WriteValue() must be called with key in object");
            CStringBuilder keyBuilder((yyjson_mut_doc*)w->_document, key);
            const char* ckey = keyBuilder.c_str();

            IS_TYPE(_SJsonWriter::ValueType*)
                return yyjson_mut_obj_add_val((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, (yyjson_mut_val*)value);
            IS_TYPE(bool)
                return yyjson_mut_obj_add_bool((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(int32_t)
                return yyjson_mut_obj_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(int64_t)
                return yyjson_mut_obj_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(uint32_t)
                return yyjson_mut_obj_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(uint64_t)
                return yyjson_mut_obj_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(float)
                return yyjson_mut_obj_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(double)
                return yyjson_mut_obj_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value);
            IS_TYPE(skr::String)
                return yyjson_mut_obj_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, value.c_str(), value.raw().size());
            IS_TYPE(skr::StringView)
                return yyjson_mut_obj_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, ckey, (const char*)value.raw().data(), value.raw().size());
        }
        else if (type == Level::kArray)
        {
            SKR_ASSERT(key.is_empty() && "WriteValue() must not be called with key in array");
            IS_TYPE(_SJsonWriter::ValueType*)
                return yyjson_mut_arr_add_val((yyjson_mut_val*)object, (yyjson_mut_val*)value);
            IS_TYPE(bool)
                return yyjson_mut_arr_add_bool((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(int32_t)
                return yyjson_mut_arr_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(int64_t)
                return yyjson_mut_arr_add_sint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(uint32_t)
                return yyjson_mut_arr_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(uint64_t)
                return yyjson_mut_arr_add_uint((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(float)
                return yyjson_mut_arr_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(double)
                return yyjson_mut_arr_add_real((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value);
            IS_TYPE(skr::String)
                return yyjson_mut_arr_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, value.c_str(), value.raw().size());
            IS_TYPE(skr::StringView)
                return yyjson_mut_arr_add_strncpy((yyjson_mut_doc*)w->_document, (yyjson_mut_val*)object, (const char*)value.raw().data(), value.raw().size());
        }
        return false;
    }

#undef IS_TYPE
};

_SJsonWriter::_SJsonWriter(size_t levelDepth)
{
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
    _document = (SJsonMutableDocument*)doc;
    _stack.reserve(levelDepth);
}

_SJsonWriter::~_SJsonWriter()
{
    yyjson_mut_doc_free((yyjson_mut_doc*)_document);
}

bool _SJsonWriter::StartObject(skr::StringView key)
{
    yyjson_mut_val *obj = yyjson_mut_obj((yyjson_mut_doc*)_document);
    if (_stack.empty())
    {
        SKR_ASSERT_RET_FALSE(key.is_empty(), "Root object should not have a key");
        yyjson_mut_doc_set_root((yyjson_mut_doc*)_document, obj);
    }
    else if (_stack.back()._type == Level::kArray)
    {
        SKR_ASSERT_RET_FALSE(key.is_empty(), "StartObject() must be called without key in array");
        yyjson_mut_arr_add_val((yyjson_mut_val*)_stack.back()._value, obj);
    }
    else if (_stack.back()._type == Level::kObject)
    {
        SKR_ASSERT_RET_FALSE(!key.is_empty(), "StartObject() must be called with key in object");
        CStringBuilder keyBuilder((yyjson_mut_doc*)_document, key);
        const char* ckey = keyBuilder.c_str();
        yyjson_mut_obj_add_val((yyjson_mut_doc*)_document, (yyjson_mut_val*)_stack.back()._value, ckey, obj);
    }
    _stack.emplace((ValueType*)obj, Level::kObject);
    return true;
}

bool _SJsonWriter::EndObject()
{
    if (_stack.empty())
        SKR_ASSERT_RET_FALSE(false, "EndObject() called without StartObject()")
    else if (_stack.back()._type != Level::kObject)
        SKR_ASSERT_RET_FALSE(false, "EndObject() should not called with StartArray()")

    _stack.pop_back();
    return true;
}

bool _SJsonWriter::StartArray(skr::StringView key, const float* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::StartArray(skr::StringView key, const double* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::StartArray(skr::StringView key)
{
    const char* dummy = nullptr;
    return _SJsonWriterHelper::StartArray(this, key, dummy, 0);
}

bool _SJsonWriter::StartArray(skr::StringView key, const int32_t* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::StartArray(skr::StringView key, const int64_t* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::StartArray(skr::StringView key, const uint32_t* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::StartArray(skr::StringView key, const uint64_t* values, _SJsonWriter::SizeType count)
{
    return _SJsonWriterHelper::StartArray(this, key, values, count);
}

bool _SJsonWriter::EndArray()
{
    if (_stack.empty())
        SKR_ASSERT_RET_FALSE(false, "EndArray() called without StartArray()")
    else if (_stack.back()._type != Level::kArray)
        SKR_ASSERT_RET_FALSE(false, "EndArray() should not called with StartObject()")

    _stack.pop_back();
    return true;
}

/*
bool _SJsonWriter::WriteValue(skr::StringView key, ValueType* value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}
*/

bool _SJsonWriter::WriteBool(skr::StringView key, bool value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteInt32(skr::StringView key, int32_t value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteInt64(skr::StringView key, int64_t value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteUInt32(skr::StringView key, uint32_t value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteUInt64(skr::StringView key, uint64_t value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteFloat(skr::StringView key, float value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteDouble(skr::StringView key, double value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteString(skr::StringView key, skr::StringView value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

bool _SJsonWriter::WriteString(skr::StringView key, const skr::String& value)
{
    return _SJsonWriterHelper::WriteValue(this, key, value);
}

skr::String _SJsonWriter::Write()
{
    yyjson_mut_doc* doc = (yyjson_mut_doc*)_document;
    auto str = yyjson_mut_write(doc, 0, NULL);
    auto result = skr::String((const char8_t*)str);
    ::free(str);
    return result;
}

SJsonWriter::SJsonWriter(size_t levelDepth)
    : _SJsonWriter(levelDepth)
{

}

bool SJsonWriter::Key(skr::StringView key)
{
    SKR_ASSERT_RET_FALSE(_currentKey.is_empty(), "Last key is not consumed yet!");
    SKR_ASSERT_RET_FALSE(!key.is_empty(), "key must not be empty!");
    _currentKey = key;
    return true;
}

bool SJsonWriter::Bool(bool value)
{
    bool r = WriteBool(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::Int32(int32_t value)
{
    bool r = WriteInt32(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::Int64(int64_t value)
{
    bool r = WriteInt64(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::UInt32(uint32_t value)
{
    bool r = WriteUInt32(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::UInt64(uint64_t value)
{
    bool r = WriteUInt64(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::Float(float value)
{
    bool r = WriteFloat(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::Double(double value)
{
    bool r = WriteDouble(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::String(skr::StringView value)
{
    bool r = WriteString(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::String(const skr::String& value)
{
    bool r = WriteString(_currentKey.view(), value);
    _currentKey.empty();
    return r;
}

bool SJsonWriter::StartArray()
{
    bool r = _SJsonWriter::StartArray(_currentKey.view());
    _currentKey.empty();
    return r;
}

bool SJsonWriter::StartObject()
{
    bool r = _SJsonWriter::StartObject(_currentKey.view());
    _currentKey.empty();
    return r;
}

#undef SKR_ASSERT_RET_FALSE