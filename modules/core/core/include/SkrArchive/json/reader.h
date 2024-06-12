#pragma once
#include "SkrArchive/json/common.h"

#if defined(__cplusplus)
#include "SkrContainers/vector.hpp"

namespace skr::archive {

using JsonReadError = JsonErrorCode;
using JsonReadResult = JsonResult;

struct SKR_STATIC_API _JsonReader {
    using CharType = char8_t;
    using SizeType = size_t;
    using DocumentType = SJsonDocument;
    using ValueType = SJsonValue;

    _JsonReader(skr::StringView json);
    ~_JsonReader();

    JsonReadResult StartObject(skr::StringView key);
    JsonReadResult EndObject();

    JsonReadResult StartArray(skr::StringView key, SizeType& count);
    JsonReadResult EndArray();

    JsonReadResult ReadBool(skr::StringView key, bool& value);
    JsonReadResult ReadInt32(skr::StringView key, int32_t& value);
    JsonReadResult ReadInt64(skr::StringView key, int64_t& value);
    JsonReadResult ReadUInt32(skr::StringView key, uint32_t& value);
    JsonReadResult ReadUInt64(skr::StringView key, uint64_t& value);
    JsonReadResult ReadFloat(skr::StringView key, float& value);
    JsonReadResult ReadDouble(skr::StringView key, double& value);
    JsonReadResult ReadString(skr::StringView key, skr::String& value);

    inline JsonReadResult ReadInt(skr::StringView key, int& value) { return ReadInt32(key, value); }
    inline JsonReadResult ReadUInt(skr::StringView key, unsigned int& value) { return ReadUInt32(key, value); }

#pragma region Helpers

    template <JsonPrimitiveReadableType Type>
    JsonReadResult ReadArray(Type* values, SizeType count)
    {
        if (!_stack.empty() && _stack.back()._type == Level::kArray)
        {
            for (SizeType i = 0; i < count; i++)
            {
                JsonReadResult result = {};

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
            return JsonReadError::NoOpenScope;
        if (_stack.back()._type != Level::kArray)
            return JsonReadError::ScopeTypeMismatch;
        return JsonReadError::UnknownError;
    }

    template <JsonPrimitiveReadableType Type>
    JsonReadResult ReadValue(skr::StringView key, Type& value)
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
            return JsonReadError::UnknownTypeToRead;
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

struct SKR_STATIC_API JsonReader : public _JsonReader {
    JsonReader(skr::StringView json);

    // TODO: REMOVE?
    JsonReadResult Key(skr::StringView key);
    JsonReadResult Bool(bool& value);
    JsonReadResult Int32(int32_t& value);
    JsonReadResult Int64(int64_t& value);
    JsonReadResult UInt32(uint32_t& value);
    JsonReadResult UInt64(uint64_t& value);
    JsonReadResult Float(float& value);
    JsonReadResult Double(double& value);
    JsonReadResult String(skr::String& value);

    JsonReadResult StartArray(SizeType& count);
    JsonReadResult StartObject();

    inline JsonReadResult Int(int value) { return Int32(value); }
    inline JsonReadResult UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

}

#endif
