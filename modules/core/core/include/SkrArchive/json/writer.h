#pragma once
#include "SkrArchive/json/common.h"

#if defined(__cplusplus)
    #include "SkrContainersDef/vector.hpp"

namespace skr::archive
{

using JsonWriteError  = JsonErrorCode;
using JsonWriteResult = JsonResult;

struct SKR_STATIC_API _JsonWriter {
    using CharType     = char8_t;
    using SizeType     = size_t;
    using DocumentType = SJsonMutableDocument;
    using ValueType    = SJsonMutableValue;

    _JsonWriter(size_t levelDepth);
    ~_JsonWriter();

    JsonWriteResult StartObject(skr::StringView key);
    JsonWriteResult EndObject();

    JsonWriteResult StartArray(skr::StringView key);
    JsonWriteResult StartArray(skr::StringView key, const float* values, SizeType count);
    JsonWriteResult StartArray(skr::StringView key, const double* values, SizeType count);
    JsonWriteResult StartArray(skr::StringView key, const int32_t* values, SizeType count);
    JsonWriteResult StartArray(skr::StringView key, const int64_t* values, SizeType count);
    JsonWriteResult StartArray(skr::StringView key, const uint32_t* values, SizeType count);
    JsonWriteResult StartArray(skr::StringView key, const uint64_t* values, SizeType count);
    JsonWriteResult EndArray();

    // bool WriteValue(skr::StringView key, ValueType* value);
    JsonWriteResult WriteBool(skr::StringView key, bool value);
    JsonWriteResult WriteInt32(skr::StringView key, int32_t value);
    JsonWriteResult WriteInt64(skr::StringView key, int64_t value);
    JsonWriteResult WriteUInt32(skr::StringView key, uint32_t value);
    JsonWriteResult WriteUInt64(skr::StringView key, uint64_t value);
    JsonWriteResult WriteFloat(skr::StringView key, float value);
    JsonWriteResult WriteDouble(skr::StringView key, double value);
    JsonWriteResult WriteString(skr::StringView key, skr::StringView value);
    JsonWriteResult WriteString(skr::StringView key, const skr::String& value);

    inline JsonWriteResult WriteInt(skr::StringView key, int value) { return WriteInt32(key, value); }
    inline JsonWriteResult WriteUInt(skr::StringView key, unsigned int value) { return WriteUInt32(key, value); }

    #pragma region Helpers

    template <JsonPrimitiveWritableType Type>
    JsonWriteResult WriteValue(skr::StringView key, const Type& value)
    {
        if constexpr (std::is_same_v<Type, bool>)
            return WriteBool(key, value);
        else if constexpr (std::is_same_v<Type, int8_t>)
            return WriteInt32(key, value);
        else if constexpr (std::is_same_v<Type, int16_t>)
            return WriteInt32(key, value);
        else if constexpr (std::is_same_v<Type, int32_t>)
            return WriteInt32(key, value);
        else if constexpr (std::is_same_v<Type, int64_t>)
            return WriteInt64(key, value);

        else if constexpr (std::is_same_v<Type, uint8_t>)
            return WriteUInt32(key, value);
        else if constexpr (std::is_same_v<Type, uint16_t>)
            return WriteUInt32(key, value);
        else if constexpr (std::is_same_v<Type, uint32_t>)
            return WriteUInt32(key, value);
        else if constexpr (std::is_same_v<Type, uint64_t>)
            return WriteUInt64(key, value);

        else if constexpr (std::is_same_v<Type, float>)
            return WriteFloat(key, value);
        else if constexpr (std::is_same_v<Type, double>)
            return WriteDouble(key, value);

        else if constexpr (std::is_same_v<Type, CharType*>)
            return WriteString(key, value);
        else if constexpr (std::is_same_v<Type, skr::String>)
            return WriteString(key, value);

        else
            return JsonErrorCode::UnknownError;
    }

    #pragma endregion

    skr::String Write();

    struct Level {
        ValueType* _value = nullptr;
        enum EType
        {
            kObject,
            kArray
        } _type = kObject;

        Level(ValueType* _value, EType _type) SKR_NOEXCEPT
            : _value(_value),
              _type(_type)
        {
        }
    };

protected:
    friend struct _WriterHelper;
    skr::Vector<Level> _stack;
    DocumentType*      _document = nullptr;
};

struct SKR_STATIC_API JsonWriter : public _JsonWriter {
    JsonWriter(size_t levelDepth);

    // TODO: REMOVE?
    JsonWriteResult Key(skr::StringView key);
    JsonWriteResult Bool(bool value);
    JsonWriteResult Int32(int32_t value);
    JsonWriteResult Int64(int64_t value);
    JsonWriteResult UInt32(uint32_t value);
    JsonWriteResult UInt64(uint64_t value);
    JsonWriteResult Float(float value);
    JsonWriteResult Double(double value);
    JsonWriteResult String(skr::StringView value);
    JsonWriteResult String(const skr::String& value);
    JsonWriteResult StartArray();
    JsonWriteResult StartObject();

    inline JsonWriteResult Int(int value) { return Int32(value); }
    inline JsonWriteResult UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

} // namespace skr::archive

#endif