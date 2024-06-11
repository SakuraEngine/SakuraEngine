#pragma once
#include "SkrJson/common.h"

#if defined(__cplusplus)
#include "SkrContainers/vector.hpp"

struct SKR_STATIC_API _SJsonWriter {
    using CharType = SJsonCharType;
    using SizeType = SJsonSizeType;
    using DocumentType = SJsonMutableDocument;
    using ValueType = SJsonMutableValue;

    _SJsonWriter(size_t levelDepth);
    ~_SJsonWriter();

    bool StartObject(skr::StringView key);
    bool EndObject();

    bool StartArray(skr::StringView key);
    bool StartArray(skr::StringView key, const float* values, SizeType count);
    bool StartArray(skr::StringView key, const double* values, SizeType count);
    bool StartArray(skr::StringView key, const int32_t* values, SizeType count);
    bool StartArray(skr::StringView key, const int64_t* values, SizeType count);
    bool StartArray(skr::StringView key, const uint32_t* values, SizeType count);
    bool StartArray(skr::StringView key, const uint64_t* values, SizeType count);
    bool EndArray();

    // bool WriteValue(skr::StringView key, ValueType* value);
    bool WriteBool(skr::StringView key, bool value);
    bool WriteInt32(skr::StringView key, int32_t value);
    bool WriteInt64(skr::StringView key, int64_t value);
    bool WriteUInt32(skr::StringView key, uint32_t value);
    bool WriteUInt64(skr::StringView key, uint64_t value);
    bool WriteFloat(skr::StringView key, float value);
    bool WriteDouble(skr::StringView key, double value);
    bool WriteString(skr::StringView key, skr::StringView value);
    bool WriteString(skr::StringView key, const skr::String& value);

    inline bool WriteInt(skr::StringView key, int value) { return WriteInt32(key, value); }
    inline bool WriteUInt(skr::StringView key, unsigned int value) { return WriteUInt32(key, value); }

#pragma region Helpers

    template <JsonPrimitiveWritableType Type>
    bool WriteValue(skr::StringView key, const Type& value) 
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
            return false;
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
    friend struct _SJsonWriterHelper;
    skr::Vector<Level> _stack;
    DocumentType* _document = nullptr;
};

struct SKR_STATIC_API SJsonWriter : public _SJsonWriter {
    SJsonWriter(size_t levelDepth);

    // TODO: REMOVE?
    bool Key(skr::StringView key);
    bool Bool(bool value);
    bool Int32(int32_t value);
    bool Int64(int64_t value);
    bool UInt32(uint32_t value);
    bool UInt64(uint64_t value);
    bool Float(float value);
    bool Double(double value);
    bool String(skr::StringView value);
    bool String(const skr::String& value);
    bool StartArray();
    bool StartObject();

    inline bool Int(int value) { return Int32(value); }
    inline bool UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

#else

typedef struct _SJsonWriter _SJsonWriter;
typedef struct SJsonWriter SJsonWriter;

#endif