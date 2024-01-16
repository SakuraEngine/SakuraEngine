
#pragma once
#include <cstddef>
#include <vector>
#include "OpenString/unicode.h"
#include "OpenString/common/definitions.h"
#include "OpenString/common/basic_types.h"
#include "OpenString/common/constants.h"
#include "OpenString/common/functions.h"

namespace ostr
{
	class OPEN_STRING_API codeunit_sequence_view
	{
	public:

		// code-region-start: constructors

		constexpr codeunit_sequence_view() noexcept;
		constexpr codeunit_sequence_view(const codeunit_sequence_view&) noexcept;
		constexpr codeunit_sequence_view(codeunit_sequence_view&&) noexcept;
		constexpr codeunit_sequence_view& operator=(const codeunit_sequence_view& other) noexcept;
		constexpr codeunit_sequence_view& operator=(codeunit_sequence_view&& other) noexcept;
		~codeunit_sequence_view() noexcept = default;

		constexpr codeunit_sequence_view(const ochar8_t* data, const u64 count) noexcept;
		constexpr codeunit_sequence_view(const ochar8_t* data, const ochar8_t* last) noexcept;
		explicit constexpr codeunit_sequence_view(const ochar8_t* str) noexcept;
		explicit constexpr codeunit_sequence_view(const ochar8_t& c) noexcept;
		explicit constexpr codeunit_sequence_view(const codepoint& cp) noexcept;

		// code-region-end: constructors

		// code-region-start: iterators

		struct const_iterator
		{
			constexpr const_iterator() noexcept;
			explicit constexpr const_iterator(const ochar8_t* v) noexcept;

			[[nodiscard]] constexpr const ochar8_t* data() const noexcept;
			[[nodiscard]] constexpr const ochar8_t& operator*() const noexcept;

			[[nodiscard]] constexpr i64 operator-(const const_iterator& rhs) const noexcept;

			constexpr const_iterator& operator+=(const i64 diff) noexcept;
			constexpr const_iterator& operator-=(const i64 diff) noexcept;
			constexpr const_iterator& operator+=(const u64 diff) noexcept;
			constexpr const_iterator& operator-=(const u64 diff) noexcept;

			[[nodiscard]] constexpr const_iterator operator+(const i64 diff) const noexcept;
			[[nodiscard]] constexpr const_iterator operator-(const i64 diff) const noexcept;
			[[nodiscard]] constexpr const_iterator operator+(const u64 diff) const noexcept;
			[[nodiscard]] constexpr const_iterator operator-(const u64 diff) const noexcept;

			constexpr const_iterator& operator++() noexcept;
			constexpr const_iterator operator++(int) noexcept;
			constexpr const_iterator& operator--() noexcept;
			constexpr const_iterator operator--(int) noexcept;

			[[nodiscard]] constexpr bool operator==(const const_iterator& rhs) const noexcept;
			[[nodiscard]] constexpr bool operator!=(const const_iterator& rhs) const noexcept;
			[[nodiscard]] constexpr bool operator<(const const_iterator& rhs) const noexcept;
			[[nodiscard]] constexpr bool operator>(const const_iterator& rhs) const noexcept;
			[[nodiscard]] constexpr bool operator<=(const const_iterator& rhs) const noexcept;
			[[nodiscard]] constexpr bool operator>=(const const_iterator& rhs) const noexcept;

			const ochar8_t* value = nullptr;
		};
	
		[[nodiscard]] constexpr const_iterator begin() const noexcept;
		[[nodiscard]] constexpr const_iterator end() const noexcept;
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept;
		[[nodiscard]] constexpr const_iterator cend() const noexcept;

		// code-region-end: iterators
	
		[[nodiscard]] constexpr bool operator==(const codeunit_sequence_view& rhs) const noexcept;
		[[nodiscard]] constexpr bool operator==(const ochar8_t* rhs) const noexcept;
		[[nodiscard]] constexpr bool operator!=(const codeunit_sequence_view& rhs) const noexcept;
		[[nodiscard]] constexpr bool operator!=(const ochar8_t* rhs) const noexcept;

		[[nodiscard]] constexpr u64 size() const noexcept;

		// This is not allowed for getting rid of misuse
		[[nodiscard]] const ochar_t* c_str() const noexcept = delete;
		[[nodiscard]] const ochar8_t* u8_str() const noexcept = delete;

		// Warning: Please make sure that you know what you are doing
		// this is not guaranteed to be null-terminated
		[[nodiscard]] const ochar8_t* data() const noexcept;

		/// @return Is this an empty string
		[[nodiscard]] constexpr bool is_empty() const noexcept;

		[[nodiscard]] constexpr codeunit_sequence_view subview(const u64 from, const u64 size = SIZE_MAX) const noexcept;

		[[nodiscard]] constexpr const ochar8_t& read_at(const u64 index) const noexcept;
		[[nodiscard]] constexpr const ochar8_t& read_from_last(const u64 index) const noexcept;

		// [[nodiscard]] constexpr codeunit_sequence_view operator[](const index_interval& range) const noexcept
		// {
		// 	return this->subview(range);
		// }

		/**
		 * \brief this is learned from FBString in folly
		 * see https://github.com/facebook/folly/blob/main/folly/FBString.h
		 * which is a Boyer-Moore-like trick
		 * \param pattern string which to search in this string
		 * \param from search range start index
		 * \param size search range size
		 * \return index where searched, return global_constant::INDEX_INVALID_INDEX if not found
		 */
		[[nodiscard]] constexpr u64 index_of(const codeunit_sequence_view& pattern, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept;
		[[nodiscard]] constexpr u64 index_of(const ochar8_t codeunit, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept;
		[[nodiscard]] constexpr u64 last_index_of(const codeunit_sequence_view& pattern, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept;
		[[nodiscard]] constexpr u64 index_of_any(const codeunit_sequence_view& units, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept;
		[[nodiscard]] constexpr u64 last_index_of_any(const codeunit_sequence_view& units, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept;

		[[nodiscard]] constexpr bool contains(const codeunit_sequence_view& pattern) const noexcept;
		[[nodiscard]] constexpr bool contains(const ochar8_t codeunit) const noexcept;

		[[nodiscard]] constexpr std::array<codeunit_sequence_view, 2> split(const codeunit_sequence_view& splitter) const noexcept;
		u32 split(const codeunit_sequence_view& splitter, std::vector<codeunit_sequence_view>& pieces, bool cull_empty = true) const noexcept;

		[[nodiscard]] constexpr u64 count(const codeunit_sequence_view& pattern, u64 from = 0, u64 size = global_constant::SIZE_INVALID) const noexcept;

		[[nodiscard]] constexpr bool starts_with(const codeunit_sequence_view& prefix) const noexcept;
		[[nodiscard]] constexpr bool ends_with(const codeunit_sequence_view& suffix) const noexcept;

		[[nodiscard]] constexpr codeunit_sequence_view remove_prefix(const codeunit_sequence_view& prefix) const noexcept;
		[[nodiscard]] constexpr codeunit_sequence_view remove_suffix(const codeunit_sequence_view& suffix) const noexcept;

		[[nodiscard]] constexpr codeunit_sequence_view trim_start(const codeunit_sequence_view& units = codeunit_sequence_view(u8" \t")) const noexcept;
		[[nodiscard]] constexpr codeunit_sequence_view trim_end(const codeunit_sequence_view& units = codeunit_sequence_view(u8" \t")) const noexcept;
		[[nodiscard]] constexpr codeunit_sequence_view trim(const codeunit_sequence_view& units = codeunit_sequence_view(u8" \t")) const noexcept;

	private:

		u64 size_{ 0 };
		const ochar8_t* data_{ nullptr };

	};
	
	namespace details
	{
		[[nodiscard]] constexpr u64 get_sequence_length(const ochar8_t* str) noexcept
		{
			if(!str)
				return 0;
			u64 count = 0;
			while(str[count] != 0)
				++count;
			return count;
		}
	}

	constexpr codeunit_sequence_view::codeunit_sequence_view() noexcept = default;
	constexpr codeunit_sequence_view::codeunit_sequence_view(const codeunit_sequence_view&) noexcept = default;
	constexpr codeunit_sequence_view::codeunit_sequence_view(codeunit_sequence_view&&) noexcept = default;
	constexpr codeunit_sequence_view& codeunit_sequence_view::operator=(const codeunit_sequence_view& other) noexcept = default;
	constexpr codeunit_sequence_view& codeunit_sequence_view::operator=(codeunit_sequence_view&& other) noexcept = default;
	
	constexpr codeunit_sequence_view::codeunit_sequence_view(const ochar8_t* data, const u64 count) noexcept
		: size_{ count }
		, data_{ data }
	{ }

	constexpr codeunit_sequence_view::codeunit_sequence_view(const ochar8_t* data, const ochar8_t* last) noexcept
		: size_{ static_cast<u64>(last - data) }
		, data_{ data }
	{ }

	constexpr codeunit_sequence_view::codeunit_sequence_view(const ochar8_t* str) noexcept
		: codeunit_sequence_view{ str, details::get_sequence_length(str) }
	{ }

	constexpr codeunit_sequence_view::codeunit_sequence_view(const ochar8_t& c) noexcept
		: codeunit_sequence_view{ &c, 1 }
	{ }

	constexpr codeunit_sequence_view::codeunit_sequence_view(const codepoint& cp) noexcept
		: codeunit_sequence_view{ cp.raw(), cp.size() }
	{ }

	constexpr codeunit_sequence_view::const_iterator::const_iterator() noexcept = default;

	constexpr codeunit_sequence_view::const_iterator::const_iterator(const ochar8_t* v) noexcept
		: value{ v }
	{ }

	constexpr const ochar8_t* codeunit_sequence_view::const_iterator::data() const noexcept
	{
		return this->value;
	}

	constexpr const ochar8_t& codeunit_sequence_view::const_iterator::operator*() const noexcept
	{
		return *this->value;
	}

	constexpr i64 codeunit_sequence_view::const_iterator::operator-(const const_iterator& rhs) const noexcept
	{
		return this->value - rhs.value;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator+=(const i64 diff) noexcept
	{
		this->value += diff;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator-=(const i64 diff) noexcept
	{
		this->value -= diff;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator+=(const u64 diff) noexcept
	{
		this->value += diff;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator-=(const u64 diff) noexcept
	{
		this->value -= diff;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator+(const i64 diff) const noexcept
	{
		const_iterator tmp = *this;
		tmp += diff;
		return tmp;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator-(const i64 diff) const noexcept
	{
		const_iterator tmp = *this;
		tmp -= diff;
		return tmp;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator+(const u64 diff) const noexcept
	{
		const_iterator tmp = *this;
		tmp += diff;
		return tmp;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator-(const u64 diff) const noexcept
	{
		const_iterator tmp = *this;
		tmp -= diff;
		return tmp;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator++() noexcept
	{
		++this->value;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator++(int) noexcept
	{
		const const_iterator tmp = *this;
		++*this;
		return tmp;
	}

	constexpr codeunit_sequence_view::const_iterator& codeunit_sequence_view::const_iterator::operator--() noexcept
	{
		--this->value;
		return *this;
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::const_iterator::operator--(int) noexcept
	{
		const const_iterator tmp = *this;
		--*this;
		return tmp;
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator==(const const_iterator& rhs) const noexcept
	{
		return this->value == rhs.value;
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator!=(const const_iterator& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator<(const const_iterator& rhs) const noexcept
	{
		return this->value < rhs.value;
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator>(const const_iterator& rhs) const noexcept
	{
		return rhs < *this;
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator<=(const const_iterator& rhs) const noexcept
	{
		return rhs >= *this;
	}

	constexpr bool codeunit_sequence_view::const_iterator::operator>=(const const_iterator& rhs) const noexcept
	{
		return !(*this < rhs);
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::begin() const noexcept
	{
		return const_iterator{ this->data_ };
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::end() const noexcept
	{
		return const_iterator{ this->data_ + this->size_ };
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::cbegin() const noexcept
	{
		return this->begin();
	}

	constexpr codeunit_sequence_view::const_iterator codeunit_sequence_view::cend() const noexcept
	{
		return this->end();
	}

	constexpr bool codeunit_sequence_view::operator==(const codeunit_sequence_view& rhs) const noexcept
	{
		if (this->size() != rhs.size()) 
			return false;
		for(u64 i = 0; i < this->size(); ++i)
			if(this->read_at(i) != rhs.read_at(i))
				return false;
		return true;
	}

	constexpr bool codeunit_sequence_view::operator==(const ochar8_t* rhs) const noexcept
	{
		return this->operator==(codeunit_sequence_view(rhs));
	}

	constexpr bool codeunit_sequence_view::operator!=(const codeunit_sequence_view& rhs) const noexcept
	{
		return !this->operator==(rhs);
	}

	constexpr bool codeunit_sequence_view::operator!=(const ochar8_t* rhs) const noexcept
	{
		return !this->operator==(rhs);
	}

	constexpr u64 codeunit_sequence_view::size() const noexcept
	{
		return this->size_;
	}

	inline const ochar8_t* codeunit_sequence_view::data() const noexcept
	{
		return this->data_;
	}

	constexpr bool codeunit_sequence_view::is_empty() const noexcept
	{
		return this->size_ == 0;
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::subview(const u64 from, const u64 size) const noexcept
	{
		const u64 self_size = this->size();

		if(from >= self_size || size == 0)
			return { };
			
		const ochar8_t* first_data = this->data_ + from;
		const u64 actual_size = minimum(size, self_size - from);
		return { first_data, actual_size };
	}

	constexpr const ochar8_t& codeunit_sequence_view::read_at(const u64 index) const noexcept
	{
		return this->data_[index];
	}

	constexpr const ochar8_t& codeunit_sequence_view::read_from_last(const u64 index) const noexcept
	{
		return this->data_[this->size_ - index - 1];
	}

	constexpr u64 codeunit_sequence_view::index_of(const codeunit_sequence_view& pattern, const u64 from, const u64 size) const noexcept
	{
		const codeunit_sequence_view view = this->subview(from, size);
		if(view.size() < pattern.size())
			return global_constant::INDEX_INVALID;
		const ochar8_t pattern_last = pattern.read_from_last(0);
		u64 skip = 1;
		while(pattern.size() > skip && pattern.read_from_last(skip) != pattern_last)
			++skip;

		u64 i = 0;
		const u64 endpoint = view.size() - pattern.size() + 1;
		while(i < endpoint)
		{
			while(true)
			{
				if(view.read_at(i + pattern.size() - 1) == pattern_last)
					break;
				++i;
				if(i == endpoint)
					return global_constant::INDEX_INVALID;
			}
			if(view.subview(i, pattern.size()) == pattern)
				return i + from;
			i += skip;
		}
		return global_constant::INDEX_INVALID;
	}

	constexpr u64 codeunit_sequence_view::index_of(const ochar8_t codeunit, const u64 from, const u64 size) const noexcept
	{
		if(codeunit == 0)
			return global_constant::INDEX_INVALID;
		const codeunit_sequence_view view = this->subview(from, size);
		for(u64 i = 0; i < view.size(); ++i)
			if(view.read_at(i) == codeunit)
				return i + from;
		return global_constant::INDEX_INVALID;
	}

	constexpr u64 codeunit_sequence_view::last_index_of(const codeunit_sequence_view& pattern, const u64 from, const u64 size) const noexcept
	{
		const codeunit_sequence_view view = this->subview(from, size);
		if(pattern.is_empty())
			return global_constant::INDEX_INVALID;
		if(view.size() < pattern.size())
			return global_constant::INDEX_INVALID;

		const ochar8_t pattern_first = pattern.read_at(0);
		u64 skip = 1;
		while(pattern.size() > skip && pattern.read_at(skip) != pattern_first)
			++skip;

		u64 i = view.size() - pattern.size();
		while(true)
		{
			while(true)
			{
				if(view.read_at(i) == pattern_first)
					break;
				if(i == 0)
					return global_constant::INDEX_INVALID;
				--i;
			}
			if(view.subview(i, pattern.size()) == pattern)
				return i + from;
			if(i < skip)
				break;
			i -= skip;
		}
		return global_constant::INDEX_INVALID;
	}

	constexpr u64 codeunit_sequence_view::index_of_any(const codeunit_sequence_view& units, const u64 from, const u64 size) const noexcept
	{
		const u64 actual_size = minimum(this->size() - from, size);
		const u64 final_index = from + actual_size;
		for(u64 i = from; i < final_index; ++i)
			if(const ochar8_t& unit = this->read_at(i); units.contains(unit))
				return i;
		return global_constant::INDEX_INVALID;
	}

	constexpr u64 codeunit_sequence_view::last_index_of_any(const codeunit_sequence_view& units, const u64 from, const u64 size) const noexcept
	{
		const u64 actual_size = minimum(this->size() - from, size);
		const u64 final_index = from + actual_size;
		for(u64 i = final_index - 1; i >= from; --i)
			if(const ochar8_t& unit = this->read_at(i); units.contains(unit))
				return i;
		return global_constant::INDEX_INVALID;
	}

	constexpr bool codeunit_sequence_view::contains(const codeunit_sequence_view& pattern) const noexcept
	{
		return this->index_of(pattern) != global_constant::INDEX_INVALID;
	}

	constexpr bool codeunit_sequence_view::contains(const ochar8_t codeunit) const noexcept
	{
		return this->index_of(codeunit) != global_constant::INDEX_INVALID;
	}

	constexpr std::array<codeunit_sequence_view, 2> codeunit_sequence_view::split(const codeunit_sequence_view& splitter) const noexcept
	{
		const u64 index = this->index_of(splitter);
		if(index == global_constant::INDEX_INVALID)
			return { *this, { } };
		const codeunit_sequence_view first = this->subview(0, index);
		const codeunit_sequence_view second = this->subview(index + splitter.size());
		return { first, second };
	}

	inline u32 codeunit_sequence_view::split(const codeunit_sequence_view& splitter, std::vector<codeunit_sequence_view>& pieces, const bool cull_empty) const noexcept
	{
		codeunit_sequence_view view{ *this };
		u32 count = 0;
		while(true)
		{
			const auto [ left, right ] = view.split(splitter);
			if(!cull_empty || !left.is_empty())
				pieces.push_back(left);
			++count;
			if(right.is_empty())
				break;
			view = right;
		}
		return count;
	}

	constexpr u64 codeunit_sequence_view::count(const codeunit_sequence_view& pattern, const u64 from, const u64 size) const noexcept
	{
		codeunit_sequence_view view = this->subview(from, size);
		u64 count = 0;
		while(true)
		{
			const u64 index = view.index_of(pattern);
			if(index == global_constant::INDEX_INVALID)
				break;
			++count;
			view = view.subview(index + pattern.size());
		}
		return count;
	}

	constexpr bool codeunit_sequence_view::starts_with(const codeunit_sequence_view& prefix) const noexcept
	{
		if(prefix.is_empty())
			return true;
		return this->subview(0, prefix.size()) == prefix;
	}

	constexpr bool codeunit_sequence_view::ends_with(const codeunit_sequence_view& suffix) const noexcept
	{
		if(suffix.is_empty())
			return true;
		const u64 end_size = suffix.size();
		const u64 this_size = this->size();
		return this->subview(this_size - end_size, end_size) == suffix;
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::remove_prefix(const codeunit_sequence_view& prefix) const noexcept
	{
		return this->starts_with(prefix) ? this->subview(prefix.size()) : *this;
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::remove_suffix(const codeunit_sequence_view& suffix) const noexcept
	{
		return this->ends_with(suffix) ? this->subview(0, this->size() - suffix.size()) : *this;
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::trim_start(const codeunit_sequence_view& units) const noexcept
	{
		if(this->is_empty())
			return { };
		for(u64 i = 0; i < this->size(); ++i)
			if(const ochar8_t codeunit = this->read_at(i); !units.contains(codeunit))
				return this->subview(i);
		return { };
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::trim_end(const codeunit_sequence_view& units) const noexcept
	{
		if(this->is_empty())
			return { };

		for(u64 i = this->size(); i > 0; --i)
			if(const ochar8_t codeunit = this->read_at(i - 1); !units.contains(codeunit))
				return this->subview(0, i);
		return { };
	}

	constexpr codeunit_sequence_view codeunit_sequence_view::trim(const codeunit_sequence_view& units) const noexcept
	{
		return this->trim_start(units).trim_end(units);
	}
}

inline namespace literal
{
	[[nodiscard]] constexpr ostr::codeunit_sequence_view operator""_cuqv(const ochar8_t* str, const size_t len) noexcept
	{
		return { str, len };
	}
}
