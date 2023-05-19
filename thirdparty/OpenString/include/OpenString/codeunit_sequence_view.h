#pragma once

#include <cstddef>
#include <tuple>
#include "common/basic_types.h"
#include "common/definitions.h"
#include "common/index_interval.h"
#include "common/unicode.h"

OPEN_STRING_NS_BEGIN

namespace details
{
	[[nodiscard]] constexpr i32 get_sequence_size(const ochar8_t* str) noexcept
	{
		if(!str)
			return 0;
		i32 count = 0;
		while(str[count] != 0)
			++count;
		return count;
	}

	[[nodiscard]] constexpr u32 hash_crc32(const ochar8_t* data, const u32 length) noexcept
	{
		// CRC32 Table
		constexpr u32 crc_table[256] = 
		{
			0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
			0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
			0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
			0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
			0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
			0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
			0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
			0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
			0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
			0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
			0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
			0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
			0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
			0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
			0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
			0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
			0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
			0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
			0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
			0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
			0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
			0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
			0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
			0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
			0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
			0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
			0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
			0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
			0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
			0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
			0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
			0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
			0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
			0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
			0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
			0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
			0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
			0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
			0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
			0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
			0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
			0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
			0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
		};
		
		u32 ans = 0xFFFFFFFF;
		for (u32 i = 0; i < length; ++i)
		{
			ans = ((ans >> 8) ^ crc_table[(ans ^ data[i]) & 0x000000FF]);
		}
		return ans ^ 0xFFFFFFFF;
	}
}

class codeunit_sequence_view
{
public:

// code-region-start: constructors

	constexpr codeunit_sequence_view() noexcept = default;
	constexpr codeunit_sequence_view(const codeunit_sequence_view&) noexcept = default;
	constexpr codeunit_sequence_view(codeunit_sequence_view&&) noexcept = default;
	constexpr codeunit_sequence_view& operator=(const codeunit_sequence_view& other) noexcept = default;
	constexpr codeunit_sequence_view& operator=(codeunit_sequence_view&& other) noexcept = default;
	~codeunit_sequence_view() noexcept = default;

	constexpr codeunit_sequence_view(const ochar8_t* data, const i32 count) noexcept
		: size_(count)
		, data_(data)
	{ }

	constexpr codeunit_sequence_view(const ochar8_t* data, const size_t count) noexcept
		: size_(static_cast<i32>(count))
		, data_(data)
	{ }

	constexpr codeunit_sequence_view(const ochar8_t* data, const ochar8_t* last) noexcept
		: size_(static_cast<i32>(last - data))
		, data_(data)
	{ }

	explicit constexpr codeunit_sequence_view(const ochar8_t* str) noexcept
		: codeunit_sequence_view(str, details::get_sequence_size(str))
	{ }

	explicit constexpr codeunit_sequence_view(const ochar8_t& c) noexcept
		: codeunit_sequence_view(&c, 1)
	{ }

	explicit constexpr codeunit_sequence_view(const codepoint& cp) noexcept
		: codeunit_sequence_view(cp.raw(), cp.size())
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
		return const_iterator{ this->data_ };
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{ this->data_ + this->size_ };
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
	
	[[nodiscard]] constexpr bool operator==(const codeunit_sequence_view& rhs) const noexcept
	{
		if (this->size_ != rhs.size_) 
			return false;
		for(i32 i = 0; i < this->size_; ++i)
			if(this->data_[i] != rhs.data_[i])
				return false;
		return true;
	}
	
	[[nodiscard]] constexpr bool operator==(const ochar8_t* rhs) const noexcept
	{
		return this->operator==(codeunit_sequence_view(rhs));
	}

	[[nodiscard]] constexpr bool operator!=(const codeunit_sequence_view& rhs) const noexcept
	{
		return !this->operator==(rhs);
	}
	
	[[nodiscard]] constexpr bool operator!=(const ochar8_t* rhs) const noexcept
	{
		return !this->operator==(rhs);
	}

	[[nodiscard]] constexpr i32 size() const noexcept
	{
		return this->size_;
	}

	[[nodiscard]] const ochar_t* c_str() const noexcept
	{
		return (const ochar_t*)this->data_;
	}

	[[nodiscard]] constexpr const ochar8_t* u8_str() const noexcept
	{
		return this->data_;
	}

	[[nodiscard]] constexpr const ochar8_t* last() const noexcept
	{
		return this->data_ + this->size_;
	}

	/// @return Is this an empty string
	[[nodiscard]] constexpr bool is_empty() const noexcept
	{
		return this->size_ == 0;
	}

	[[nodiscard]] constexpr codeunit_sequence_view subview(const index_interval& range) const noexcept
	{
		const i32 self_size = this->size();
		const index_interval selection = range.select(self_size);
		if(selection.is_empty())
			return { };
		const i32 size = selection.size();
		const ochar8_t* first_data = this->data_ + selection.get_inclusive_min();
		return { first_data, size };
	}
	
	[[nodiscard]] constexpr const ochar8_t& read_at(const i32 index) const noexcept
	{
		return this->data_[index + (index >= 0 ? 0 : this->size_)];
	}

	[[nodiscard]] constexpr const ochar8_t& operator[](const i32 index) const noexcept
	{
		return this->read_at(index);
	}

	[[nodiscard]] constexpr codeunit_sequence_view operator[](const index_interval& range) const noexcept
	{
		return this->subview(range);
	}

	/**
	 * \brief this is learned from FBString in folly
	 * see https://github.com/facebook/folly/blob/main/folly/FBString.h
	 * which is a Boyer-Moore-like trick
	 * \param pattern string which to search in this string
	 * \param range search range
	 * \return index where searched, return index_invalid if not found
	 */
	[[nodiscard]] constexpr i32 index_of(const codeunit_sequence_view& pattern, const index_interval& range = index_interval::all()) const noexcept
	{
		if(pattern.is_empty())
			return index_invalid;
		const i32 self_size = this->size();
		const index_interval selection = range.select(self_size);
		const codeunit_sequence_view view = this->subview(selection);
		if(view.size() < pattern.size())
			return index_invalid;

		const ochar8_t pattern_last = pattern.data_[pattern.size_ - 1];
		i32 skip = 1;
		while(pattern.size_ > skip && pattern.data_[pattern.size_ - 1 - skip] != pattern_last)
			++skip;

		i32 i = 0;
		const i32 endpoint = view.size_ - pattern.size_ + 1;
		while(i < endpoint)
		{
			while(true)
			{
				if(view.data_[i + pattern.size_ - 1] == pattern_last)
					break;
				++i;
				if(i == endpoint)
					return index_invalid;
			}
			if(view.subview({'[', i, i + pattern.size(),')' }) == pattern)
				return i + selection.get_inclusive_min();
			i += skip;
		}
		return index_invalid;
	}

	[[nodiscard]] constexpr i32 index_of(const ochar8_t codeunit, const index_interval& range = index_interval::all()) const noexcept
	{
		if(codeunit == 0)
			return index_invalid;
		const i32 self_size = this->size();
		const index_interval selection = range.select(self_size);
		const codeunit_sequence_view view = this->subview(selection);
		for(i32 i = 0; i < view.size_; ++i)
			if(view.data_[i] == codeunit)
				return i + selection.get_inclusive_min();
		return index_invalid;
	}
	
	[[nodiscard]] constexpr i32 last_index_of(const codeunit_sequence_view& pattern, const index_interval& range = index_interval::all()) const noexcept
	{
		const i32 self_size = this->size();
		const index_interval selection = range.select(self_size);
		const codeunit_sequence_view view = this->subview(selection);
		if(pattern.is_empty())
			return index_invalid;
		if(view.size_ < pattern.size_)
			return index_invalid;

		const ochar8_t pattern_first = pattern.data_[0];
		i32 skip = 1;
		while(pattern.size_ > skip && pattern.data_[skip] != pattern_first)
			++skip;

		i32 i = view.size_ - pattern.size_;
		constexpr i32 endpoint = 0;
		while(i >= endpoint)
		{
			while(true)
			{
				if(view.data_[i] == pattern_first)
					break;
				if(i == endpoint)
					return index_invalid;
				--i;
			}
			if(view.subview({'[', i, i + pattern.size(),')' }) == pattern)
				return i + selection.get_inclusive_min();
			i -= skip;
		}
		return index_invalid;
	}

	[[nodiscard]] constexpr i32 index_any_of(const codeunit_sequence_view& units, const index_interval& range = index_interval::all()) const noexcept
	{
		const index_interval selection = range.select(this->size());
		for(const i32 i : selection)
		{
			const ochar8_t& unit = this->data_[i];
			if(units.contains(unit))
				return i;
		}
		return index_invalid;
	}

	[[nodiscard]] constexpr bool contains(const codeunit_sequence_view& pattern) const noexcept
	{
		return this->index_of(pattern) != index_invalid;
	}

	[[nodiscard]] constexpr bool contains(const ochar8_t codeunit) const noexcept
	{
		return this->index_of(codeunit) != index_invalid;
	}

	[[nodiscard]] constexpr std::tuple<codeunit_sequence_view, codeunit_sequence_view> split(const codeunit_sequence_view& splitter) const noexcept
	{
		const i32 index = this->index_of(splitter);
		if(index == index_invalid)
			return { *this, { } };
		const codeunit_sequence_view first = this->subview({ '[', 0, index, ')' });
		const codeunit_sequence_view second = this->subview({ '[', index + splitter.size(), '~' });
		return { first, second };
	}

	[[nodiscard]] constexpr i32 count(const codeunit_sequence_view& pattern) const noexcept
	{
		codeunit_sequence_view view = *this;
		i32 count = 0;
		while(true)
		{
			const i32 index = view.index_of(pattern);
			if(index == index_invalid)
				break;
			++count;
			view = view.subview({ '[', index + pattern.size(), '~' });
		}
		return count;
	}

	[[nodiscard]] constexpr bool starts_with(const codeunit_sequence_view& prefix) const noexcept
	{
		if(prefix.is_empty())
			return true;
		const i32 start_size = prefix.size_;
		return this->subview({ '~', start_size , ')' }) == prefix;
	}

	[[nodiscard]] constexpr bool ends_with(const codeunit_sequence_view& suffix) const noexcept
	{
		if(suffix.is_empty())
			return true;
		const i32 end_size = suffix.size_;
		return this->subview({ '[', -end_size, -1, ']' }) == suffix;
	}

	[[nodiscard]] constexpr codeunit_sequence_view remove_prefix(const codeunit_sequence_view& prefix) const noexcept
	{
		return this->starts_with(prefix) ? this->subview({ '[', prefix.size_, '~' }) : *this;
	}

	[[nodiscard]] constexpr codeunit_sequence_view remove_suffix(const codeunit_sequence_view& suffix) const noexcept
	{
		return this->ends_with(suffix) ? this->subview({ '[', 0, -suffix.size_, ')' }) : *this;
	}

	[[nodiscard]] constexpr codeunit_sequence_view trim_start(const codeunit_sequence_view& units = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept
	{
		if(this->is_empty())
			return { };
		for(i32 i = 0; i < this->size_; ++i)
			if(const ochar8_t codeunit = this->data_[i]; !units.contains(codeunit))
				return this->subview({ '[', i, '~' });
		return { };
	}

	[[nodiscard]] constexpr codeunit_sequence_view trim_end(const codeunit_sequence_view& units = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept
	{
		if(this->is_empty())
			return { };

		for(i32 i = this->size_ - 1; i >= 0; --i)
			if(const ochar8_t codeunit = this->data_[i]; !units.contains(codeunit))
				return this->subview({ '[', 0, i, ']' });
		return { };
	}

	[[nodiscard]] constexpr codeunit_sequence_view trim(const codeunit_sequence_view& units = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept
	{
		return this->trim_start(units).trim_end(units);
	}

	[[nodiscard]] constexpr u32 get_hash() const noexcept
	{
		return details::hash_crc32(this->data_, this->size_);
	}

private:

	i32 size_ = 0;
	const ochar8_t* data_ = nullptr;

};

OPEN_STRING_NS_END

inline namespace literal
{
	[[nodiscard]] constexpr OPEN_STRING_NS::codeunit_sequence_view operator""_cuqv(const ochar8_t* str, const size_t len) noexcept
	{
		return { str, len };
	}
}
