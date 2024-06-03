#pragma once
#include "SkrJson/common.h"

#if defined(__cplusplus)
#include "SkrContainers/vector.hpp"

struct SKR_STATIC_API _SJsonReader {
    using CharType = SJsonCharType;
    using SizeType = SJsonSizeType;
    using DocumentType = SJsonDocument;
    using ValueType = SJsonValue;

    _SJsonReader(skr::StringView json);
    ~_SJsonReader();

    bool StartObject(skr::StringView key);
    bool EndObject();

    bool StartArray(skr::StringView key, SizeType& count);
    bool EndArray();

    bool ReadBool(skr::StringView key, bool& value);
    bool ReadInt32(skr::StringView key, int32_t& value);
    bool ReadInt64(skr::StringView key, int64_t& value);
    bool ReadUInt32(skr::StringView key, uint32_t& value);
    bool ReadUInt64(skr::StringView key, uint64_t& value);
    bool ReadFloat(skr::StringView key, float& value);
    bool ReadDouble(skr::StringView key, double& value);
    bool ReadString(skr::StringView key, skr::String& value);

    inline bool ReadInt(skr::StringView key, int& value) { return ReadInt32(key, value); }
    inline bool ReadUInt(skr::StringView key, unsigned int& value) { return ReadUInt32(key, value); }

#pragma region Helpers

    template <JsonPrimitiveReadableType Type>
    bool ReadArray(Type* values, SizeType count)
    {
        if (!_stack.empty() && _stack.back()._type == Level::kArray)
        {
            for (SizeType i = 0; i < count; i++)
            {
                if constexpr (std::is_same_v<Type, bool>)
                    ReadBool(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, int32_t>)
                    ReadInt32(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, int64_t>)
                    ReadInt64(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, uint32_t>)
                    ReadUInt32(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, uint64_t>)
                    ReadUInt64(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, float>)
                    ReadFloat(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, double>)
                    ReadDouble(u8"", values[i]);
                else if constexpr (std::is_same_v<Type, skr::String>)
                    ReadString(u8"", values[i]);
            }
            return true;
        }
        return false;
    }

    template <JsonPrimitiveReadableType Type>
    bool ReadValue(skr::StringView key, Type& value)
    {
        if constexpr (std::is_same_v<Type, bool>)
            return ReadBool(key, value);
        else if constexpr (std::is_same_v<Type, int8_t>)
        {
            int32_t v;
            bool success = ReadInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, int16_t>)
        {
            int32_t v;
            bool success = ReadInt32(key, v);
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
            bool success = ReadUInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, uint16_t>)
        {
            uint32_t v;
            bool success = ReadUInt32(key, v);
            value = v;
            return success;
        }
        else if constexpr (std::is_same_v<Type, uint32_t>)
        {
            uint32_t v;
            bool success = ReadUInt32(key, v);
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
            return false;
    }

#pragma endregion

    struct Level {
        ValueType* _value = nullptr;
        enum {
            kObject,
            kArray
        } _type = kObject;
        uint32_t _index = 0;
    };
protected:
    friend struct _SJsonReaderHelper;
    skr::Vector<Level> _stack;
    DocumentType* _document = nullptr;
};

struct SKR_STATIC_API SJsonReader : public _SJsonReader {
    SJsonReader(skr::StringView json);

    // TODO: REMOVE?
    bool Key(skr::StringView key);
    bool Bool(bool& value);
    bool Int32(int32_t& value);
    bool Int64(int64_t& value);
    bool UInt32(uint32_t& value);
    bool UInt64(uint64_t& value);
    bool Float(float& value);
    bool Double(double& value);
    bool String(skr::String& value);

    bool StartArray(SizeType& count);
    bool StartObject();

    inline bool Int(int value) { return Int32(value); }
    inline bool UInt(unsigned int value) { return UInt32(value); }

protected:
    skr::String _currentKey;
};

#endif
