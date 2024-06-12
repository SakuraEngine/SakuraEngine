#pragma once
#include "SkrArchive/json/common.h"

#if defined(__cplusplus)
#include "SkrContainers/vector.hpp"

namespace skr::json {

using EReadError = ErrorCode;
using ReadResult = JsonResult;

struct SKR_STATIC_API _Reader {
    using CharType = SJsonCharType;
    using SizeType = SJsonSizeType;
    using DocumentType = SJsonDocument;
    using ValueType = SJsonValue;

    _Reader(skr::StringView json);
    ~_Reader();

    ReadResult StartObject(skr::StringView key);
    ReadResult EndObject();

    ReadResult StartArray(skr::StringView key, SizeType& count);
    ReadResult EndArray();

    ReadResult ReadBool(skr::StringView key, bool& value);
    ReadResult ReadInt32(skr::StringView key, int32_t& value);
    ReadResult ReadInt64(skr::StringView key, int64_t& value);
    ReadResult ReadUInt32(skr::StringView key, uint32_t& value);
    ReadResult ReadUInt64(skr::StringView key, uint64_t& value);
    ReadResult ReadFloat(skr::StringView key, float& value);
    ReadResult ReadDouble(skr::StringView key, double& value);
    ReadResult ReadString(skr::StringView key, skr::String& value);

    inline ReadResult ReadInt(skr::StringView key, int& value) { return ReadInt32(key, value); }
    inline ReadResult ReadUInt(skr::StringView key, unsigned int& value) { return ReadUInt32(key, value); }

#pragma region Helpers

    template <JsonPrimitiveReadableType Type>
    ReadResult ReadArray(Type* values, SizeType count)
    {
        if (!_stack.empty() && _stack.back()._type == Level::kArray)
        {
            for (SizeType i = 0; i < count; i++)
            {
                ReadResult result = {};

                if constexpr (std::is_same_v<Type, bool>)
                    result = ReadBool(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, int32_t>)
                    result = ReadInt32(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, int64_t>)
                    result = ReadInt64(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, uint32_t>)
                    result = ReadUInt32(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, uint64_t>)
                    result = ReadUInt64(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, float>)
                    result = ReadFloat(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, double>)
                    result = ReadDouble(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, skr::String>)
                    result = ReadString(u8"", values[i]);

                if (result.has_error())
                    return result;
            }
            return {};
        }
        if (_stack.empty())
            return EReadError::NoOpenScope;
        if (_stack.back()._type != Level::kArray)
            return EReadError::ScopeTypeMismatch;
        return EReadError::UnknownError;
    }

    template <JsonPrimitiveReadableType Type>
    ReadResult ReadValue(skr::StringView key, Type& value)
    {
        if constexpr (std::is_same_v<Type, bool>)
            return ReadBool(key, value);
        else if constexpr (std::is_same_v<Type, int8_t>)
        {
            int32_t v;
            auto success = ReadInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, int16_t>)
        {
            int32_t v;
            auto success = ReadInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, int32_t>)
            return ReadInt32(key, value);
        else if constexpr (std::is_same_v<Type, int64_t>)
            return ReadInt64(key, value);

        else if constexpr (std::is_same_v<Type, uint8_t>)
        {
            uint32_t v;
            auto success = ReadUInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, uint16_t>)
        {
            uint32_t v;
            auto success = ReadUInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, uint32_t>)
        {
            uint32_t v;
            auto success = ReadUInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, uint64_t>)
            return ReadUInt64(key, value);

        else if constexpr (std::is_same_v<Type, float>)
            return ReadFloat(key, value);
        else if constexpr (std::is_same_v<Type, double>)
            return ReadDouble(key, value);
            
        else if constexpr (std::is_same_v<Type, skr::String>)
            return ReadString(key, value);

        else
            return EReadError::UnknownTypeToRead;
    }

#pragma endregion

    struct Level {
        ValueType* _value = nullptr;
        enum EType {
            kObject,
            kArray
        } _type = kObject;
        uint32_t _index = 0;

        Level(ValueType* _value, EType _type) SKR_NOEXCEPT
            : _value(_value), _type(_type) 
        {

        }
    };
protected:
    friend struct _ReaderHelper;
    skr::Vector<Level> _stack;
    DocumentType* _document = nullptr;
};

struct SKR_STATIC_API Reader : public _Reader {
    Reader(skr::StringView json);

    // TODO: REMOVE?
    ReadResult Key(skr::StringView key);
    ReadResult Bool(bool& value);
    ReadResult Int32(int32_t& value);
    ReadResult Int64(int64_t& value);
    ReadResult UInt32(uint32_t& value);
    ReadResult UInt64(uint64_t& value);
    ReadResult Float(float& value);
    ReadResult Double(double& value);
    ReadResult String(skr::String& value);

    ReadResult StartArray(SizeType& count);
    ReadResult StartObject();

    inline ReadResult Int(int value) { return Int32(value); }
    inline ReadResult UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

}

#endif
