// OpenString - unicode related functions
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once
#include "definitions.h"
#include "basic_types.h"
#include <array>

OPEN_STRING_NS_BEGIN

namespace unicode
{
	/**
	 * @param c start of a utf8 sequence
	 * @return length of utf8 sequence, return 0 if this is not start of a utf8 sequence
	 */
	[[nodiscard]] constexpr i32 parse_utf8_length(const ochar8_t c) noexcept
	{
		if (c == 0)
			return 1;
		constexpr ochar8_t mask = 0b10000000_as_char;
		i32 size = 0;
		u8 v = c;
		while (v & mask)
		{
			++size;
			v <<= 1;
		}
		// 1 byte when start with 0
		// 0 byte when start with 10
		// 2 bytes when start with 110
		// n bytes when start with 1..(n)..0
		return size > 1 ? size : 1 - size;
	}

	/**
	 * @param length length of a utf8 sequence
	 * @return mask of utf8 code unit
	 */
	[[nodiscard]] constexpr ochar8_t get_utf8_mask(const i32 length) noexcept
	{
		constexpr ochar8_t masks[] =
		{
			0b00111111_as_char,
			0b01111111_as_char,
			0b00011111_as_char,
			0b00001111_as_char,
			0b00000111_as_char,
		};
		return masks[length];
	}

	/**
	 * @param utf8 input utf-8 code unit sequence
	 * @param length length of utf-8 code unit sequence
	 * @return codepoint of input utf8 code unit sequence
	 */
	[[nodiscard]] constexpr char32_t utf8_to_utf32(ochar8_t const* const utf8, const i32 length) noexcept
	{
		if (!utf8) 
			return 0;
		const ochar8_t c0 = utf8[0];
		const ochar8_t lead_mask = get_utf8_mask(length);
		char32_t utf32 = c0 & lead_mask;
		constexpr ochar8_t following_mask = get_utf8_mask(0);
		for (i32 i = 1; i < length; ++i)
		{
			constexpr i32 bits_following = 6;
			const ochar8_t ci = utf8[i];
			utf32 <<= bits_following;
			utf32 |= ci & following_mask;
		}
		return utf32;
	}

	[[nodiscard]] constexpr i32 parse_utf8_length(const char32_t utf32) noexcept
	{
		if (utf32 < 0x80)
			return 1;
		if (utf32 < 0x800)
			return 2;
		if (utf32 < 0x10000)
			return 3;
		return 4;
	}

	/**
	 * @param utf32 input utf-32 code unit
	 * @return decoded utf-8 code unit sequence from utf-32 code unit
	 */
	[[nodiscard]] constexpr std::array<ochar8_t, 4> utf32_to_utf8(const char32_t utf32) noexcept
	{
		if (utf32 < 0x80) return
			{ static_cast<ochar8_t>(utf32),
				0, 0, 0 };
		if (utf32 < 0x800) return
			{ static_cast<ochar8_t>((utf32 >> 6) | 0xc0), 
				static_cast<ochar8_t>((utf32 & 0x3f) | 0x80),
				0, 0 };
		if (utf32 < 0x10000) return
			{ static_cast<ochar8_t>((utf32 >> 12) | 0xe0),
				static_cast<ochar8_t>(((utf32 >> 6) & 0x3f) | 0x80),
				static_cast<ochar8_t>((utf32 & 0x3f) | 0x80),
				0 };
		return
			{ static_cast<ochar8_t>((utf32 >> 18) | 0xf0),
				static_cast<ochar8_t>(((utf32 >> 12) & 0x3f) | 0x80),
				static_cast<ochar8_t>(((utf32 >> 6) & 0x3f) | 0x80),
				static_cast<ochar8_t>((utf32 & 0x3f) | 0x80) };
	}
}

struct codepoint
{
	static constexpr i32 sequence_length = 4;
	std::array<ochar8_t, sequence_length> sequence;

// code-region-start: constructors
	
	explicit constexpr codepoint(const ochar8_t v0 = 0, const ochar8_t v1 = 0, const ochar8_t v2 = 0, const ochar8_t v3 = 0) noexcept
		: sequence({ v0, v1, v2, v3 })
	{ }

	explicit constexpr codepoint(const ochar8_t* const v, const i32 length) noexcept
		: sequence()
	{
		for(i32 i = 0; i < length; ++i)
			sequence[i] = v[i];
	}

	explicit constexpr codepoint(const ochar8_t* v) noexcept
		: codepoint(v, unicode::parse_utf8_length(v[0]))
	{ }

	explicit constexpr codepoint(const char32_t cp) noexcept
		: sequence(unicode::utf32_to_utf8(cp))
	{ }

// code-region-end: constructors

// code-region-start: iterators

	struct const_iterator
	{
		constexpr const_iterator() noexcept = default;
		
		explicit constexpr const_iterator(const ochar8_t* v) noexcept
			: value(v)
		{ }

		[[nodiscard]] constexpr const ochar8_t& operator*() const noexcept
		{
			return *this->value;
		}
		
		[[nodiscard]] constexpr const ochar8_t* raw() const noexcept
		{
			return this->value;
		}

		[[nodiscard]] constexpr std::ptrdiff_t operator-(const const_iterator& rhs) const noexcept
		{
			return this->value - rhs.value;
		}
		
		constexpr const_iterator& operator+=(const std::ptrdiff_t diff) noexcept
		{
			this->value += diff;
			return *this;
		}

		constexpr const_iterator& operator-=(const std::ptrdiff_t diff) noexcept
		{
			return this->operator+=(-diff);
		}

		[[nodiscard]] constexpr const_iterator operator+(const std::ptrdiff_t diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp += diff;
			return tmp;
		}

		[[nodiscard]] constexpr const_iterator operator-(const std::ptrdiff_t diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp -= diff;
			return tmp;
		}

	    constexpr const_iterator& operator++() noexcept
		{
			++this->value;
			return *this;
		}

	    constexpr const_iterator operator++(int) noexcept
		{
			const const_iterator tmp = *this;
	        ++*this;
	        return tmp;
	    }

		constexpr const_iterator& operator--() noexcept
		{
			--this->value;
			return *this;
	    }

		constexpr const_iterator operator--(int) noexcept 
		{
			const const_iterator tmp = *this;
			--*this;
			return tmp;
		}

	    [[nodiscard]] constexpr bool operator==(const const_iterator& rhs) const noexcept
		{
			return this->value == rhs.value;
	    }

	    [[nodiscard]] constexpr bool operator!=(const const_iterator& rhs) const noexcept
		{
	        return !(*this == rhs);
	    }

	    [[nodiscard]] constexpr bool operator<(const const_iterator& rhs) const noexcept
		{
	        return this->value < rhs.value;
	    }
	 
	    [[nodiscard]] constexpr bool operator>(const const_iterator& rhs) const noexcept
		{
	        return rhs < *this;
	    }
	 
	    [[nodiscard]] constexpr bool operator<=(const const_iterator& rhs) const noexcept
		{
	        return rhs >= *this;
	    }
	 
	    [[nodiscard]] constexpr bool operator>=(const const_iterator& rhs) const noexcept
		{
	        return !(*this < rhs);
	   }
		
		const ochar8_t* value = nullptr;
	};
	
	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator{ this->sequence.data() };
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{ this->sequence.data() + this->size() };
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return this->begin();
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return this->end();
	}

// code-region-end: iterators

	[[nodiscard]] constexpr bool operator==(const codepoint& rhs) const noexcept
	{
		const i32 size_l = this->size();
		const i32 size_r = rhs.size();
		if(size_l != size_r)
			return false;
		for(i32 i = 0; i < size_l; ++i)
			if(this->sequence[i] != rhs.sequence[i])
				return false;
		return true;
	}

	[[nodiscard]] constexpr bool operator==(const ochar8_t* rhs) const noexcept
	{
		for(i32 i = 0; i < this->size(); ++i)
			if(this->sequence[i] != rhs[i])
				return false;
		return true;
	}

	[[nodiscard]] constexpr bool operator==(const char32_t rhs) const noexcept
	{
		return this->get_codepoint() == rhs;
	}

	[[nodiscard]] constexpr bool operator!=(const codepoint& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	[[nodiscard]] constexpr const ochar8_t* raw() const noexcept
	{
		return this->sequence.data();
	}

	[[nodiscard]] constexpr i32 size() const noexcept
	{
		for(i32 i = 0; i < sequence_length; ++i)
			if(this->sequence[i] == 0)
				return i;
		return sequence_length;
	}

	[[nodiscard]] constexpr char32_t get_codepoint() const noexcept
	{
		return unicode::utf8_to_utf32(this->sequence.data(), this->size());
	}

	[[nodiscard]] explicit constexpr operator char32_t() const noexcept
	{
		return this->get_codepoint();
	}
};

[[nodiscard]] constexpr bool operator==(const ochar8_t* lhs, const codepoint& rhs) noexcept
{
	return rhs == lhs;
}

OPEN_STRING_NS_END

inline namespace literal
{
	[[nodiscard]] constexpr OPEN_STRING_NS::codepoint operator""_cp(const ochar8_t c) noexcept
	{
		return OPEN_STRING_NS::codepoint{ c };
	}

	[[nodiscard]] constexpr OPEN_STRING_NS::codepoint operator""_cp(const char32_t c) noexcept
	{
		return OPEN_STRING_NS::codepoint{ c };
	}
}