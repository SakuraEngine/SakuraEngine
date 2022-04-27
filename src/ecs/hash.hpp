#pragma once

namespace dual
{
#ifdef __EMSCRIPTEN__
	constexpr size_t _FNV_offset_basis = 2166136261U;
	constexpr size_t _FNV_prime = 16777619U;
#else
	constexpr size_t _FNV_offset_basis = sizeof(size_t) == sizeof(uint32_t) ? 2166136261U : 14695981039346656037ULL;
	constexpr size_t _FNV_prime = sizeof(uint32_t) ? 16777619U : 1099511628211ULL;
#endif
    inline size_t hash_append(size_t val, size_t data)
    {
        val ^= data;
        val *= _FNV_prime;
        return val;
    }

    inline size_t hash_append(size_t val, const unsigned char* const data, const size_t length) noexcept
	{ // accumulate range [data, data + length) into partial FNV-1a uuid val
		for (size_t i = 0; i < length; ++i) {
			val ^= static_cast<size_t>(data[i]);
			val *= _FNV_prime;
		}

		return val;
	}

	template <class T>
	inline size_t hash_bytes(const T* const data, const size_t length, const size_t basis = _FNV_offset_basis) noexcept
	{ 
        // bitwise hashes the representation of an array
		return hash_append(
			basis, reinterpret_cast<const unsigned char*>(data), length * sizeof(T));
	}
}