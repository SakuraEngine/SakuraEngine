#pragma once
#include "SkrRT/misc/types.h"

namespace skr
{
namespace lite
{
template <typename T>
struct LiteOptional {
    LiteOptional() = default;
    LiteOptional(const T& value)
        : value(value)
        , has_value(true)
    {
    }
    LiteOptional(T&& value)
        : value(std::move(value))
        , has_value(true)
    {
    }
    LiteOptional(const LiteOptional& other)
        : value(other.value)
        , has_value(other.has_value)
    {
    }
    LiteOptional(LiteOptional&& other)
        : value(std::move(other.value))
        , has_value(other.has_value)
    {
    }
    LiteOptional& operator=(const LiteOptional& other)
    {
        value     = other.value;
        has_value = other.has_value;
        return *this;
    }
    LiteOptional& operator=(LiteOptional&& other)
    {
        value     = std::move(other.value);
        has_value = other.has_value;
        return *this;
    }
    LiteOptional& operator=(const T& value)
    {
        this->value = value;
        has_value   = true;
        return *this;
    }
    LiteOptional& operator=(T&& value)
    {
        this->value = std::move(value);
        has_value   = true;
        return *this;
    }
    operator bool() const { return has_value; }
    T&       operator*() { return value; }
    const T& operator*() const { return value; }
    const T& get() const { return value; }
    T&       get() { return value; }

protected:
    T    value;
    bool has_value = false;
};

} // namespace lite
} // namespace skr