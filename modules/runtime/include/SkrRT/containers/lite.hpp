#pragma once
/* #ifdef CONTAINER_LITE_IMPL
    #include "SkrRT/containers/vector.hpp"
    #include "SkrRT/containers/string.hpp"
    #include "SkrRT/containers/hashmap.hpp"
    #include <new> // placement new operator
#endif */
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
        value = other.value;
        has_value = other.has_value;
        return *this;
    }
    LiteOptional& operator=(LiteOptional&& other)
    {
        value = std::move(other.value);
        has_value = other.has_value;
        return *this;
    }
    LiteOptional& operator=(const T& value)
    {
        this->value = value;
        has_value = true;
        return *this;
    }
    LiteOptional& operator=(T&& value)
    {
        this->value = std::move(value);
        has_value = true;
        return *this;
    }
    operator bool() const { return has_value; }
    T& operator*() { return value; }
    const T& operator*() const { return value; }
    const T& get() const { return value; }
    T& get() { return value; }

protected:
    T value;
    bool has_value = false;
};

template <typename T>
struct LiteSpan {
    inline constexpr uint64_t size() const SKR_NOEXCEPT { return size_; }
    inline SKR_CONSTEXPR T* data() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T& operator[](uint64_t index) const SKR_NOEXCEPT { return data_[index]; }
    inline SKR_CONSTEXPR T* begin() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T* end() const SKR_NOEXCEPT { return data_ + size_; }
    inline SKR_CONSTEXPR bool empty() const SKR_NOEXCEPT { return size_ == 0; }
    T* data_ = nullptr;
    uint64_t size_ = 0;
};

template <uint32_t _size, uint32_t _align>
struct AlignedStorage {
    alignas(_align) uint8_t storage[_size];
};

/* #ifndef CONTAINER_LITE_IMPL
template <typename T>
struct VectorStorage : public AlignedStorage<24, 8> {
    VectorStorage();
    ~VectorStorage();
    VectorStorage(const VectorStorage&);
    VectorStorage(VectorStorage&&);
    VectorStorage& operator=(const VectorStorage&);
    VectorStorage& operator=(VectorStorage&&);
};
#else
template <typename T>
using VectorStorage = skr::vector<T>;
#endif

#ifndef CONTAINER_LITE_IMPL
struct TextStorage : public AlignedStorage<16, 1> {
    TextStorage(const char8_t*);

    TextStorage();
    ~TextStorage();
    TextStorage(const TextStorage&);
    TextStorage(TextStorage&&);
    TextStorage& operator=(const TextStorage&);
    TextStorage& operator=(TextStorage&&);
};
#else
using TextStorage = skr::string;
#endif

#ifndef CONTAINER_LITE_IMPL
template <typename K, typename V>
struct HashMapStorage : public AlignedStorage<48, 8> {

    HashMapStorage();
    ~HashMapStorage();
    HashMapStorage(const HashMapStorage&);
    HashMapStorage(HashMapStorage&&);
    HashMapStorage& operator=(const HashMapStorage&);
    HashMapStorage& operator=(HashMapStorage&&);
};
#else
template <typename K, typename V>
using HashMapStorage = skr::flat_hash_map<K, V>;
#endif */

} // namespace lite
} // namespace skr