#pragma once
#include "SkrJson/common.h"

#if defined(__cplusplus)
#include "SkrContainers/vector.hpp"

namespace skr::json {

using EWriteError = ErrorCode;
using WriteResult = JsonResult;

struct SKR_STATIC_API _Writer {
    using CharType = SJsonCharType;
    using SizeType = SJsonSizeType;
    using DocumentType = SJsonMutableDocument;
    using ValueType = SJsonMutableValue;

    _Writer(size_t levelDepth);
    ~_Writer();

    WriteResult StartObject(skr::StringView key);
    WriteResult EndObject();

    WriteResult StartArray(skr::StringView key);
    WriteResult StartArray(skr::StringView key, const float* values, SizeType count);
    WriteResult StartArray(skr::StringView key, const double* values, SizeType count);
    WriteResult StartArray(skr::StringView key, const int32_t* values, SizeType count);
    WriteResult StartArray(skr::StringView key, const int64_t* values, SizeType count);
    WriteResult StartArray(skr::StringView key, const uint32_t* values, SizeType count);
    WriteResult StartArray(skr::StringView key, const uint64_t* values, SizeType count);
    WriteResult EndArray();

    // bool WriteValue(skr::StringView key, ValueType* value);
    WriteResult WriteBool(skr::StringView key, bool value);
    WriteResult WriteInt32(skr::StringView key, int32_t value);
    WriteResult WriteInt64(skr::StringView key, int64_t value);
    WriteResult WriteUInt32(skr::StringView key, uint32_t value);
    WriteResult WriteUInt64(skr::StringView key, uint64_t value);
    WriteResult WriteFloat(skr::StringView key, float value);
    WriteResult WriteDouble(skr::StringView key, double value);
    WriteResult WriteString(skr::StringView key, skr::StringView value);
    WriteResult WriteString(skr::StringView key, const skr::String& value);

    inline WriteResult WriteInt(skr::StringView key, int value) { return WriteInt32(key, value); }
    inline WriteResult WriteUInt(skr::StringView key, unsigned int value) { return WriteUInt32(key, value); }

#pragma region Helpers

    template <JsonPrimitiveWritableType Type>
    WriteResult WriteValue(skr::StringView key, const Type& value) 
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
            return EWriteError::UnknownError;
    }

#pragma endregion

    skr::String Write();

    struct Level {
        ValueType* _value = nullptr;
        enum EType {
            kObject,
            kArray
        } _type = kObject;

        Level(ValueType* _value, EType _type) SKR_NOEXCEPT
            : _value(_value), _type(_type) 
        {

        }
    };
protected:
    friend struct _WriterHelper;
    skr::Vector<Level> _stack;
    DocumentType* _document = nullptr;
};

struct SKR_STATIC_API Writer : public _Writer {
    Writer(size_t levelDepth);

    // TODO: REMOVE?
    WriteResult Key(skr::StringView key);
    WriteResult Bool(bool value);
    WriteResult Int32(int32_t value);
    WriteResult Int64(int64_t value);
    WriteResult UInt32(uint32_t value);
    WriteResult UInt64(uint64_t value);
    WriteResult Float(float value);
    WriteResult Double(double value);
    WriteResult String(skr::StringView value);
    WriteResult String(const skr::String& value);
    WriteResult StartArray();
    WriteResult StartObject();

    inline WriteResult Int(int value) { return Int32(value); }
    inline WriteResult UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

}

#endif