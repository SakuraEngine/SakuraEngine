#pragma once
#include "platform/Configure.h"
#include <cstdlib>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <unordered_map>
#include <cinttypes>
#include <memory>
namespace godot{
#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS_ENABLED
#endif
#define _ALWAYS_INLINE_ FORCEINLINE
#define _FORCE_INLINE_ FORCEINLINE
// Turn argument to string constant:
// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html#Stringizing
#ifndef _STR
#define _STR(m_x) #m_x
#define _MKSTR(m_x) _STR(m_x)
#endif
#if defined(__GNUC__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif
// Generic ABS function, for math uses please use Math::abs.
#ifndef ABS
#define ABS(m_v) (((m_v) < 0) ? (-(m_v)) : (m_v))
#endif

#ifndef SGN
#define SGN(m_v) (((m_v) == 0) ? (0.0) : (((m_v) < 0) ? (-1.0) : (+1.0)))
#endif

#ifndef MIN
#define MIN(m_a, m_b) (((m_a) < (m_b)) ? (m_a) : (m_b))
#endif

#ifndef MAX
#define MAX(m_a, m_b) (((m_a) > (m_b)) ? (m_a) : (m_b))
#endif

#ifndef CLAMP
#define CLAMP(m_a, m_min, m_max) (((m_a) < (m_min)) ? (m_min) : (((m_a) > (m_max)) ? m_max : m_a))
#endif

// Swap 16, 32 and 64 bits value for endianness.
#if defined(__GNUC__)
#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)
#define BSWAP64(x) __builtin_bswap64(x)
#else
static inline uint16_t BSWAP16(uint16_t x) {
	return (x >> 8) | (x << 8);
}

static inline uint32_t BSWAP32(uint32_t x) {
	return ((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24));
}

static inline uint64_t BSWAP64(uint64_t x) {
	x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
	x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
	x = (x & 0x00FF00FF00FF00FF) << 8 | (x & 0xFF00FF00FF00FF00) >> 8;
	return x;
}
#endif

using real_t = float;

template<class T>
class Vector : public std::vector<T>
{
public:
	bool has(const T& key) const
	{
		return std::find(this->begin(), this->end(), key) != this->end();
	}
    void remove(size_t inPos)
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
		std::vector<T>::insert(this->begin() + pos, v);
	}
	void append_array(const Vector<T>& other)
	{
		std::vector<T>::insert(this->end(), other.begin(), other.end());
	}
};

template<class T>
class Set : public std::set<T>
{
public:
	bool has(const T& key) const
	{
		return this->find(key) != this->end();
	}
};

template<class K, class T>
class Map : public std::map<K, T>
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
class HashMap : public std::unordered_map<K, T>
{
	public:
	bool has(const K& key) const
	{
		return this->find(key) != this->end();
	}
};

template<class T>
using List = std::list<T>;

using Variant = void*;

template<class T>
using Ref = std::shared_ptr<T>;

using PackedByteArray = Vector<uint8_t>;
using PackedInt32Array = Vector<int32_t>;

enum InlineAlignment {
	// Image alignment points.
	INLINE_ALIGNMENT_TOP_TO = 0b0000,
	INLINE_ALIGNMENT_CENTER_TO = 0b0001,
	INLINE_ALIGNMENT_BOTTOM_TO = 0b0010,
	INLINE_ALIGNMENT_IMAGE_MASK = 0b0011,

	// Text alignment points.
	INLINE_ALIGNMENT_TO_TOP = 0b0000,
	INLINE_ALIGNMENT_TO_CENTER = 0b0100,
	INLINE_ALIGNMENT_TO_BASELINE = 0b1000,
	INLINE_ALIGNMENT_TO_BOTTOM = 0b1100,
	INLINE_ALIGNMENT_TEXT_MASK = 0b1100,

	// Presets.
	INLINE_ALIGNMENT_TOP = INLINE_ALIGNMENT_TOP_TO | INLINE_ALIGNMENT_TO_TOP,
	INLINE_ALIGNMENT_CENTER = INLINE_ALIGNMENT_CENTER_TO | INLINE_ALIGNMENT_TO_CENTER,
	INLINE_ALIGNMENT_BOTTOM = INLINE_ALIGNMENT_BOTTOM_TO | INLINE_ALIGNMENT_TO_BOTTOM
};


enum HAlign {
	HALIGN_LEFT,
	HALIGN_CENTER,
	HALIGN_RIGHT,
	HALIGN_FILL,
};

/** Error List. Please never compare an error against FAILED
 * Either do result != OK , or !result. This way, Error fail
 * values can be more detailed in the future.
 *
 * This is a generic error list, mainly for organizing a language of returning errors.
 *
 * Errors:
 * - Are added to the Error enum in core/error/error_list.h
 * - Have a description added to error_names in core/error/error_list.cpp
 * - Are bound with BIND_CORE_ENUM_CONSTANT() in core/core_constants.cpp
 */

enum Error {
	OK, // (0)
	FAILED, ///< Generic fail error
	ERR_UNAVAILABLE, ///< What is requested is unsupported/unavailable
	ERR_UNCONFIGURED, ///< The object being used hasn't been properly set up yet
	ERR_UNAUTHORIZED, ///< Missing credentials for requested resource
	ERR_PARAMETER_RANGE_ERROR, ///< Parameter given out of range (5)
	ERR_OUT_OF_MEMORY, ///< Out of memory
	ERR_FILE_NOT_FOUND,
	ERR_FILE_BAD_DRIVE,
	ERR_FILE_BAD_PATH,
	ERR_FILE_NO_PERMISSION, // (10)
	ERR_FILE_ALREADY_IN_USE,
	ERR_FILE_CANT_OPEN,
	ERR_FILE_CANT_WRITE,
	ERR_FILE_CANT_READ,
	ERR_FILE_UNRECOGNIZED, // (15)
	ERR_FILE_CORRUPT,
	ERR_FILE_MISSING_DEPENDENCIES,
	ERR_FILE_EOF,
	ERR_CANT_OPEN, ///< Can't open a resource/socket/file
	ERR_CANT_CREATE, // (20)
	ERR_QUERY_FAILED,
	ERR_ALREADY_IN_USE,
	ERR_LOCKED, ///< resource is locked
	ERR_TIMEOUT,
	ERR_CANT_CONNECT, // (25)
	ERR_CANT_RESOLVE,
	ERR_CONNECTION_ERROR,
	ERR_CANT_ACQUIRE_RESOURCE,
	ERR_CANT_FORK,
	ERR_INVALID_DATA, ///< Data passed is invalid (30)
	ERR_INVALID_PARAMETER, ///< Parameter passed is invalid
	ERR_ALREADY_EXISTS, ///< When adding, item already exists
	ERR_DOES_NOT_EXIST, ///< When retrieving/erasing, if item does not exist
	ERR_DATABASE_CANT_READ, ///< database is full
	ERR_DATABASE_CANT_WRITE, ///< database is full (35)
	ERR_COMPILATION_FAILED,
	ERR_METHOD_NOT_FOUND,
	ERR_LINK_FAILED,
	ERR_SCRIPT_FAILED,
	ERR_CYCLIC_LINK, // (40)
	ERR_INVALID_DECLARATION,
	ERR_DUPLICATE_SYMBOL,
	ERR_PARSE_ERROR,
	ERR_BUSY,
	ERR_SKIP, // (45)
	ERR_HELP, ///< user requested help!!
	ERR_BUG, ///< a bug in the software certainly happened, due to a double check failing or unexpected behavior.
	ERR_PRINTER_ON_FIRE, /// the parallel port printer is engulfed in flames
	ERR_MAX, // Not being returned, value represents the number of errors
};

template <class T>
void memdelete(T *p_class)
{
	delete p_class;
}

void memfree(void* p_ptr);

void* memalloc(size_t size);

void* memrealloc(void* p_ptr, size_t size);

#define memnew_placement(ptr, T) new(ptr) T
#define memnew(T) new T

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

// Generic swap template.
#ifndef SWAP
#define SWAP(m_x, m_y) __swap_tmpl((m_x), (m_y))
template <class T>
inline void __swap_tmpl(T &x, T &y) {
	T aux = x;
	x = y;
	y = aux;
}
#endif // SWAP

// TODO
using Image = void*;

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    almost_equal(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x-y) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * ulp
        // unless the result is subnormal
        || std::fabs(x-y) < std::numeric_limits<T>::min();
}

#define UNIT_EPSILON 0.001

// Function to find the next power of 2 to an integer.
inline unsigned int next_power_of_2(unsigned int x) {
	if (x == 0) {
		return 0;
	}

	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;

	return ++x;
}

class TextServer* get_text_server();

#define TS get_text_server()

#define MODULE_FREETYPE_ENABLED
}