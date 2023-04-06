#pragma once
#ifdef CONTAINER_LITE_IMPL
#include <containers/vector.hpp>
#include <containers/text.hpp>
#include <containers/hashmap.hpp>
#include <new> // placement new operator
#endif
#include "utils/types.h"

namespace skr {
namespace lite {
template<typename T>
struct LiteOptional
{
    LiteOptional() = default;
    LiteOptional(const T& value) : value(value), has_value(true) {}
    LiteOptional(T&& value) : value(std::move(value)), has_value(true) {}
    LiteOptional(const LiteOptional& other) : value(other.value), has_value(other.has_value) {}
    LiteOptional(LiteOptional&& other) : value(std::move(other.value)), has_value(other.has_value) {}
    LiteOptional& operator=(const LiteOptional& other) { value = other.value; has_value = other.has_value; return *this; }
    LiteOptional& operator=(LiteOptional&& other) { value = std::move(other.value); has_value = other.has_value; return *this; }
    LiteOptional& operator=(const T& value) { this->value = value; has_value = true; return *this; }
    LiteOptional& operator=(T&& value) { this->value = std::move(value); has_value = true; return *this; }
    operator bool() const { return has_value; }
    T& operator*() { return value; }
    const T& operator*() const { return value; }
    const T& get() const { return value; }
    T& get() { return value; }
protected:
    T value;
    bool has_value = false;
};

template<typename T>
struct LiteSpan
{
    inline constexpr uint64_t size() const SKR_NOEXCEPT { return size_; }
    inline SKR_CONSTEXPR T* data() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T& operator[](uint64_t index) const SKR_NOEXCEPT { return data_[index]; }
    inline SKR_CONSTEXPR T* begin() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T* end() const SKR_NOEXCEPT { return data_ + size_; }
    inline SKR_CONSTEXPR bool empty() const SKR_NOEXCEPT { return size_ == 0; }
    T* data_ = nullptr;
    uint64_t size_ = 0;
};

template<uint32_t _size, uint32_t _align>
struct AlignedStorage
{
    alignas(_align) uint8_t storage[_size];
};

using VectorStorageBase = AlignedStorage<24, 8>;
template<typename T> 
struct VectorStorage : public VectorStorageBase
{
#ifdef CONTAINER_LITE_IMPL
    using type = skr::vector<T>;
    inline type& get() 
    {
        return *std::launder(reinterpret_cast<type*>(this));
    }
    inline const type& get() const
    {
        return *std::launder(reinterpret_cast<const type*>(this));
    }
    inline VectorStorage() { ctor(); }
    inline ~VectorStorage() { dtor(); }
private:
    void ctor() { new (&get()) type(); }
    void dtor() { get().~type(); }
    static_assert(sizeof(VectorStorageBase) == sizeof(type), "Vector storage size mismatch!");
    static_assert(alignof(VectorStorageBase) == alignof(type), "Vector storage alignment mismatch!"); 
#endif
};

using TextStorageBase = AlignedStorage<16, 1>;
struct TextStorage : public TextStorageBase
{
    RUNTIME_API TextStorage() SKR_NOEXCEPT;
    RUNTIME_API TextStorage(const char8_t* str) SKR_NOEXCEPT;
    RUNTIME_API ~TextStorage() SKR_NOEXCEPT;
#ifdef CONTAINER_LITE_IMPL
    using type = skr::text::text;
    inline type& get() 
    {
        return *std::launder(reinterpret_cast<type*>(this));
    }
    inline const type& get() const
    {
        return *std::launder(reinterpret_cast<const type*>(this));
    }
private:
    inline void ctor(const char8_t* str = nullptr) { new (&get()) type(str); }
    inline void dtor() { get().~type(); }
    static_assert(sizeof(TextStorageBase) == sizeof(type), "Vector storage size mismatch!");
    static_assert(alignof(TextStorageBase) == alignof(type), "Vector storage alignment mismatch!"); 
#endif
};

using HashMapStorageBase = AlignedStorage<48, 8>;
template<typename K, typename V> 
struct HashMapStorage : public HashMapStorageBase
{
#ifdef CONTAINER_LITE_IMPL
    using type = skr::flat_hash_map<K, V>;
    inline type& get() 
    {
        return *std::launder(reinterpret_cast<type*>(this));
    }
    inline const type& get() const
    {
        return *std::launder(reinterpret_cast<const type*>(this));
    }
    inline HashMapStorage() { ctor(); }
    inline ~HashMapStorage() { dtor(); }
private:
    void ctor() { new (&get()) type(); }
    void dtor() { get().~type(); }
    static_assert(sizeof(HashMapStorageBase) == sizeof(type), "HashMap storage size mismatch!");
    static_assert(alignof(HashMapStorageBase) == alignof(type), "HashMap storage alignment mismatch!"); 
#endif
};

} }