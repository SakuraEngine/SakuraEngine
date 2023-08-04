
#pragma once
#include "common/definitions.h"

#include <optional>
#include "codeunit_sequence_view.h"

namespace ostr
{
	// UTF-8 encoded string
	class OPEN_STRING_API text_view
	{
	public:

		// code-region-start: constructors
	
		constexpr text_view() noexcept = default;
		constexpr text_view(const text_view&) noexcept = default;
		constexpr text_view(text_view&&) noexcept = default;
		constexpr text_view& operator=(const text_view&) noexcept = default;
		constexpr text_view& operator=(text_view&&) noexcept = default;
		~text_view() noexcept = default;

		constexpr text_view(const ochar8_t* data, const u64 count) noexcept
			: view_{ data, count }
		{ }
		constexpr text_view(const ochar8_t* str) noexcept
			: view_{ str }
		{ }
		constexpr text_view(codeunit_sequence_view view) noexcept
			: view_{ std::move(view) }
		{ }
		constexpr text_view(const codepoint& cp) noexcept
			: view_{ cp.raw(), cp.size() }
		{ }

		constexpr text_view& operator=(const ochar8_t* str) noexcept
		{
			*this = text_view(str);
			return *this;
		}

		// code-region-end: constructors

		// code-region-start: iterators

		struct const_iterator
		{
			constexpr const_iterator() noexcept = default;
			
			explicit constexpr const_iterator(codeunit_sequence_view::const_iterator v) noexcept
				: value(v)
			{ }

			[[nodiscard]] constexpr u64 raw_size() const noexcept
			{
				return unicode::parse_utf8_length(*this->value);
			}

			[[nodiscard]] constexpr codepoint get_codepoint() const noexcept
			{
				return codepoint{ this->value.data() };
			}

			[[nodiscard]] constexpr codepoint operator*() const noexcept
			{
				return this->get_codepoint();
			}
		
			[[nodiscard]] constexpr i64 operator-(const const_iterator& rhs) const noexcept
			{
				if(rhs > *this)
					return - (rhs - *this);

				const_iterator cur = rhs;
				i64 diff = 0;
				while(cur != *this)
				{
					++cur;
					++diff;
				}
				return diff;
			}
		
			constexpr const_iterator& operator+=(const i64 diff) noexcept
			{
				if(diff != 0)
				{
					using self_operator = const_iterator& (const_iterator::*)();
					const auto& op = diff > 0 ? static_cast<self_operator>(&const_iterator::operator++) : static_cast<self_operator>(&const_iterator::operator--);
					const u64 diff_abs = abs(diff);
					for(u64 i = 0; i < diff_abs; ++i)
						(this->*op)();
				}
				return *this;
			}

			constexpr const_iterator& operator-=(const i64 diff) noexcept
			{
				return this->operator+=(-diff);
			}

			[[nodiscard]] constexpr const_iterator operator+(const i64 diff) const noexcept
			{
				const_iterator tmp = *this;
				tmp += diff;
				return tmp;
			}

			[[nodiscard]] constexpr const_iterator operator-(const i64 diff) const noexcept
			{
				const_iterator tmp = *this;
				tmp -= diff;
				return tmp;
			}

			constexpr const_iterator& operator++() noexcept
			{
				this->value += static_cast<i64>(this->raw_size());
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
				while(this->raw_size() == 0)
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
		
			codeunit_sequence_view::const_iterator value;
		};
	
		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			return const_iterator{ this->view_.cbegin() };
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			return const_iterator{ this->view_.cend() };
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
	
		[[nodiscard]] constexpr bool operator==(const text_view& rhs) const noexcept
		{
			return this->view_ == rhs.view_;
		}

		[[nodiscard]] constexpr bool operator!=(const text_view& rhs) const noexcept
		{
			return !this->operator==(rhs);
		}
	
		[[nodiscard]] constexpr bool operator==(const ochar8_t* rhs) const noexcept
		{
			return this->view_ == rhs;
		}
	
		[[nodiscard]] constexpr bool operator!=(const ochar8_t* rhs) const noexcept
		{
			return this->view_ != rhs;
		}

		[[nodiscard]] constexpr u64 size() const noexcept
		{
			return this->get_codepoint_index( this->view_.size() );
		}

		[[nodiscard]] constexpr codeunit_sequence_view raw() const noexcept
		{
			return this->view_;
		}

		// This is not allowed for getting rid of misuse.
		[[nodiscard]] const ochar_t* c_str() const noexcept = delete;
		[[nodiscard]] const ochar8_t* u8_str() const noexcept = delete;

		/// @return Is this an empty string
		[[nodiscard]] constexpr bool is_empty() const noexcept
		{
			return this->view_.is_empty();
		}

		[[nodiscard]] constexpr text_view subview(const u64 from, const u64 size = SIZE_MAX) const noexcept
		{
			u64 raw_from = from; u64 raw_size = size;
			this->get_codeunit_range(raw_from, raw_size);
			return { this->view_.subview(raw_from, raw_size) };
		}

		[[nodiscard]] constexpr codepoint read_at(const u64 index) const noexcept
		{
			const ochar8_t* data = &this->view_.read_at(this->get_codeunit_index(index));
			return codepoint{ data };
		}

		[[nodiscard]] constexpr codepoint operator[](const u64 index) const noexcept
		{
			return this->read_at(index);
		}

		[[nodiscard]] constexpr u64 index_of(const text_view& pattern, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept
		{
			if(pattern.is_empty())
				return global_constant::INDEX_INVALID;
			u64 raw_from = from; u64 raw_size = size;
			this->get_codeunit_range(raw_from, raw_size);
			const u64 found_raw_index = this->view_.index_of(pattern.view_, raw_from, raw_size);
			if(found_raw_index == global_constant::INDEX_INVALID)
				return global_constant::INDEX_INVALID;
			return this->get_codepoint_index(found_raw_index);
		}

		[[nodiscard]] constexpr u64 index_of(const codepoint& pattern, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept
		{
			return this->index_of(text_view{ pattern }, from, size);
		}
	
		[[nodiscard]] constexpr u64 last_index_of(const text_view& pattern, const u64 from = 0, const u64 size = SIZE_MAX) const noexcept
		{
			if(pattern.is_empty())
				return global_constant::INDEX_INVALID;
			u64 raw_from = from; u64 raw_size = size;
			this->get_codeunit_range(raw_from, raw_size);
			const u64 found_raw_index = this->view_.last_index_of(pattern.view_, raw_from, raw_size);
			if(found_raw_index == global_constant::INDEX_INVALID)
				return global_constant::INDEX_INVALID;
			return this->get_codepoint_index(found_raw_index);
		}
	
		[[nodiscard]] u64 count(const text_view& pattern, const u64 from, const u64 size) const noexcept
		{
			return this->view_.count(pattern.view_, from, size);
		}

		[[nodiscard]] constexpr bool contains(const text_view& pattern) const noexcept
		{
			return this->view_.contains(pattern.view_);
		}

		[[nodiscard]] constexpr bool contains(const codepoint& pattern) const noexcept
		{
			return this->contains(text_view{ pattern });
		}

		[[nodiscard]] constexpr std::array<text_view, 2> split(const text_view& splitter) const noexcept
		{
			const auto [ first, second ] = this->view_.split(splitter.view_);
			return { text_view{ first }, text_view{ second } };
		}

		[[nodiscard]] constexpr bool starts_with(const text_view& prefix) const noexcept
		{
			return this->view_.starts_with(prefix.view_);
		}

		[[nodiscard]] constexpr bool ends_with(const text_view& suffix) const noexcept
		{
			return this->view_.ends_with(suffix.view_);
		}

		[[nodiscard]] constexpr text_view remove_prefix(const text_view& prefix) const noexcept
		{
			return text_view{ this->view_.remove_prefix(prefix.view_) };
		}

		[[nodiscard]] constexpr text_view remove_suffix(const text_view& suffix) const noexcept
		{
			return text_view{ this->view_.remove_suffix(suffix.view_) };
		}

		[[nodiscard]] constexpr text_view trim_start(const text_view& text_set = text_view(u8" \t")) const noexcept
		{
			if(this->is_empty())
				return { };
			if(text_set.is_empty())
				return *this;
			auto iterator = this->begin();
			const u64 size = this->size();
			for(u64 i = 0; i < size; ++i)
			{
				if(const auto cp = *iterator; !text_set.contains(cp))
					return this->subview(i);
				++iterator;
			}
			return { };
		}

		[[nodiscard]] constexpr text_view trim_end(const text_view& text_set = text_view(u8" \t")) const noexcept
		{
			if(this->is_empty())
				return { };
			if(text_set.is_empty())
				return *this;
			auto iterator = this->end();
			const u64 size = this->size();
			for(u64 i = size; i > 0; --i)
			{
				--iterator;
				if(const auto cp = *iterator; !text_set.contains(cp))
					return this->subview(0, i);
			}
			return { };
		}

		[[nodiscard]] constexpr text_view trim(const text_view& text_set = text_view(u8" \t")) const noexcept
		{
			return this->trim_start(text_set).trim_end(text_set);
		}

		[[nodiscard]] constexpr u64 get_codepoint_index(const u64 codeunit_index) const noexcept
		{
			const u64 view_size = this->view_.size();
			u64 index = 0;
			u64 offset = 0;
			while(offset < codeunit_index && offset < view_size)
			{
				const u8 sequence_length = unicode::parse_utf8_length(this->view_.read_at(offset));
				const u8 offset_delta = sequence_length == 0 ? 1 : sequence_length;
				offset += offset_delta;
				++index;
			}
			return index;
		}

		[[nodiscard]] constexpr u64 get_codeunit_index(const u64 codepoint_index) const noexcept
		{
			const u64 view_size = this->view_.size();
			u64 index = 0;
			u64 offset = 0;
			while(index < codepoint_index && offset < view_size)
			{
				const u8 sequence_length = unicode::parse_utf8_length(this->view_.read_at(offset));
				offset += sequence_length;
				++index;
			}
			return offset;
		}

		constexpr void get_codeunit_range(u64& from, u64& size) const noexcept
		{
			const u64 origin_from = from;
			const u64 actual_size = minimum(size, this->size() - from);
			const u64 last = this->get_codeunit_index(origin_from + actual_size);
			from = this->get_codeunit_index(origin_from);
			size = last - from;
		}

		// todo: a better conversion approach
		[[nodiscard]] constexpr std::optional<i32> try_as_integer() const
		{
			if(this->is_empty())
				return { };
			i32 result = 0;
			i32 signal = 1;
			codeunit_sequence_view raw_view = this->raw();
			if(raw_view.starts_with(u8"+"_cuqv))
			{
				raw_view = raw_view.subview(1);
			}
			if(raw_view.starts_with(u8"-"_cuqv))
			{
				raw_view = raw_view.subview(1);
				signal = -1;
			}
			for(const ochar8_t c : raw_view)
			{
				if(c < u8'0' || c > u8'9')
					return { };
				result *= 10;
				result += (c - u8'0');
			}
			return result * signal;
		}
	
	private:
	
		codeunit_sequence_view view_{ };
	
	};
}

inline namespace literal
{
	[[nodiscard]] constexpr ostr::text_view operator""_txtv(const ochar8_t* str, const size_t len) noexcept
	{
		return { str, len };
	}
}
