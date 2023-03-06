#pragma once
#include "text_server/config.h"
#include "text_server/sort_array.h"

#include <containers/vector.hpp>
#include <EASTL/set.h>
#include <EASTL/map.h>
#include <EASTL/list.h>
#include <containers/hashmap.hpp>
#include <EASTL/shared_ptr.h>

namespace godot{
template<class K>
struct Hasher : public eastl::hash<K> {};

template<class T>
class Span : public skr::span<T>
{

};

#ifdef DEBUG_ENABLED
#define SORT_ARRAY_VALIDATE_ENABLED true
#else
#define SORT_ARRAY_VALIDATE_ENABLED false
#endif


template<class T>
class Vector : public skr::vector<T>
{
public:
	Vector() = default;
	Vector(const Vector<T>& other) = default;
	Vector(const Span<T>& other) : skr::vector<T>(other) {}
	Vector& operator=(const Vector<T>& other) = default;
	inline Vector& operator=(const Span<T>& other)
	{
		this->clear();
		skr::vector<T>::insert(this->end(), other.begin(), other.end());
		return *this;
	}

	template <class Comparator, bool Validate = SORT_ARRAY_VALIDATE_ENABLED, class... Args>
	void sort_custom(Args &&...args) {
		int len = this->size();
		if (len == 0) {
			return;
		}

		T *data = ptrw();
		SortArray<T, Comparator, Validate> sorter{ args... };
		sorter.sort(data, len);
	}

	bool has(const T& key) const
	{
		return eastl::find(this->begin(), this->end(), key) != this->end();
	}
    void remove(size_t inPos)
    {
        this->erase(this->begin() + inPos);
    }
    void remove_at(size_t inPos)
    {
        this->erase(this->begin() + inPos);
    }
	void remove(const T& key)
	{
		this->erase(std::remove(this->begin(), this->end(), key), this->end());
	}
    void reverse()
    {
        std::reverse(this->begin(), this->end());
    }
	bool is_empty() const
	{
		return this->size() == 0;
	}
	T* ptrw()
	{
		return this->data();
	}
	const T* ptr() const
	{
		return this->data();
	}
	void insert(size_t pos, const T& v)
	{
		skr::vector<T>::insert(this->begin() + pos, v);
	}
	void append(const T& v)
	{
		skr::vector<T>::emplace_back(v);
	}
	void append_array(const Vector<T>& other)
	{
		skr::vector<T>::insert(this->end(), other.begin(), other.end());
	}
};


template <typename T>
class TypedArray : public Vector<T>
{
public:
	TypedArray() = default;
	TypedArray(const TypedArray<T> &other) = default;
	TypedArray(const Vector<T> &other)
	{
		Vector<T>::operator=(other);
	}

	inline TypedArray& operator=(const Vector<T>& other)
	{
		Vector<T>::operator=(other);
		return *this;
	}
};

template<class T>
class Set : public eastl::set<T>
{
public:
	bool has(const T& key) const
	{
		return this->find(key) != this->end();
	}
};

template<class K, class T>
class Map : public eastl::map<K, T>
{
public:
	bool is_empty() const
	{
		return this->size() == 0;
	}

	bool has(const K& key) const
	{
		return this->find(key) != this->end();
	}
};

template<class K, class T>
class HashMap : public skr::flat_hash_map<K, T, godot::Hasher<K>>
{
public:
	bool has(const K& key) const
	{
		return this->find(key) != this->end();
	}
	const T& operator[](const K& key) const
	{
		return this->at(key);
	}
	T& operator[](const K& key) 
	{
		if (has(key)) return this->at(key);
		return this->insert({ key, T() }).first->second;
	}
	bool is_empty() const 
	{
		return this->size() == 0;
	}
};

template<class T>
using List = eastl::list<T>;

using Variant = void*;

template<class T>
struct Ref : public eastl::shared_ptr<T>
{
	bool is_null() const { return !this->get(); }
};

template <class T>
class BitField {
	int64_t value = 0;

public:
	_FORCE_INLINE_ void set_flag(T p_flag) { value |= (int64_t)p_flag; }
	_FORCE_INLINE_ bool has_flag(T p_flag) const { return value & (int64_t)p_flag; }
	_FORCE_INLINE_ bool is_empty() const { return value == 0; }
	_FORCE_INLINE_ void clear_flag(T p_flag) { value &= ~(int64_t)p_flag; }
	_FORCE_INLINE_ void clear() { value = 0; }
	_FORCE_INLINE_ BitField() = default;
	_FORCE_INLINE_ BitField(int64_t p_value) { value = p_value; }
	_FORCE_INLINE_ BitField(T p_value) { value = (int64_t)p_value; }
	_FORCE_INLINE_ operator int64_t() const { return value; }
	_FORCE_INLINE_ operator Variant() const { return value; }
};

using PackedByteArray = Vector<uint8_t>;
using PackedInt32Array = Vector<int32_t>;

} // namespace godot