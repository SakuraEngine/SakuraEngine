// OpenString - view of readonly text
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once

#include "codeunit_sequence_view.h"
#include "format.h"

OPEN_STRING_NS_BEGIN

// UTF-8 encoded string
class text_view
{
public:

// code-region-start: constructors
	
	constexpr text_view() noexcept = default;
	constexpr text_view(const text_view&) noexcept = default;
	constexpr text_view(text_view&&) noexcept = default;
	constexpr text_view& operator=(const text_view&) noexcept = default;
	constexpr text_view& operator=(text_view&&) noexcept = default;
	~text_view() noexcept = default;

	explicit constexpr text_view(const ochar8_t* data, const i32 count) noexcept
		: view_(data, count)
	{ }
	constexpr text_view(const ochar8_t* data, const size_t count) noexcept
		: view_(data, count)
	{ }
	constexpr text_view(const ochar8_t* str) noexcept
		: view_(str)
	{ }
	constexpr text_view(const codeunit_sequence_view& view) noexcept
		: view_(view)
	{ }
	constexpr text_view(const codepoint& cp) noexcept
		: view_(cp.raw(), cp.size())
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
		constexpr const_iterator() noexcept
			: value()
		{ }
		explicit constexpr const_iterator(const ochar8_t* v) noexcept
			: value(v)
		{ }

		[[nodiscard]] constexpr i32 raw_size() const noexcept
		{
			return unicode::parse_utf8_length(*this->value);
		}

		[[nodiscard]] constexpr codepoint get_codepoint() const noexcept
		{
			return codepoint{ this->value };
		}

		[[nodiscard]] constexpr codepoint operator*() const noexcept
		{
			return this->get_codepoint();
		}
		
		[[nodiscard]] constexpr i32 operator-(const const_iterator& rhs) const noexcept
		{
			if(rhs > *this)
				return - (rhs - *this);

			const_iterator cur = rhs;
			i32 diff = 0;
			while(cur != *this)
			{
				++cur;
				++diff;
			}
			return diff;
		}
		
		constexpr const_iterator& operator+=(const i32 diff) noexcept
		{
			if(diff != 0)
			{
				using self_operator = const_iterator& (const_iterator::*)();
				const auto& op = diff > 0 ? static_cast<self_operator>(&const_iterator::operator++) : static_cast<self_operator>(&const_iterator::operator--);
				const i32 diff_abs = diff > 0 ? diff : -diff;
				for(i32 i = 0; i < diff_abs; ++i)
					(this->*op)();
			}
			return *this;
		}

		constexpr const_iterator& operator-=(const i32 diff) noexcept
		{
			return this->operator+=(-diff);
		}

		[[nodiscard]] constexpr const_iterator operator+(const i32 diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp += diff;
			return tmp;
		}

		[[nodiscard]] constexpr const_iterator operator-(const i32 diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp -= diff;
			return tmp;
		}

	    constexpr const_iterator& operator++() noexcept
		{
			this->value += this->raw_size();
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
		
		const ochar8_t* value;
	};
	
	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator{ this->view_.u8_str() };
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{ this->view_.u8_str() + this->view_.size() };
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

	[[nodiscard]] constexpr i32 size() const noexcept
	{
		return this->get_codepoint_index( this->view_.size() );
	}

	[[nodiscard]] constexpr codeunit_sequence_view raw() const noexcept
	{
		return this->view_;
	}

	[[nodiscard]] const ochar_t* c_str() const noexcept
	{
		return this->raw().c_str();
	}

	[[nodiscard]] constexpr const ochar8_t* u8_str() const noexcept
	{
		return this->raw().u8_str();
	}

	/// @return Is this an empty string
	[[nodiscard]] constexpr bool is_empty() const noexcept
	{
		return this->view_.is_empty();
	}

	[[nodiscard]] constexpr text_view subview(const index_interval& range) const noexcept
	{
		const index_interval selection = range.select(this->size());
		const i32 first = this->get_codeunit_index( selection.get_inclusive_min() );
		const i32 last = this->get_codeunit_index( selection.get_exclusive_max() );
		return { this->view_.subview({ '[', first, last, ')' }) };
	}

	[[nodiscard]] constexpr text_view operator[](const index_interval& range) const noexcept
	{
		return this->subview(range);
	}

	[[nodiscard]] constexpr codepoint read_at(const i32 index) const noexcept
	{
		const ochar8_t* data = &this->view_[ this->get_codeunit_index(index) ];
		return codepoint{ data };
	}

	[[nodiscard]] constexpr codepoint operator[](const i32 index) const noexcept
	{
		return this->read_at(index);
	}

	[[nodiscard]] constexpr i32 index_of(const text_view& pattern, const index_interval& range = index_interval::all()) const noexcept
	{
		if(pattern.is_empty())
			return index_invalid;
		const index_interval sequence_range = this->get_codeunit_range(range);
		const i32 found_sequence_index = this->view_.index_of(pattern.view_, sequence_range);
		if(found_sequence_index == index_invalid)
			return index_invalid;
		return this->get_codepoint_index(found_sequence_index);
	}

	[[nodiscard]] constexpr i32 index_of(const codepoint& pattern, const index_interval& range = index_interval::all()) const noexcept
	{
		return this->index_of(text_view{ pattern }, range);
	}
	
	[[nodiscard]] constexpr i32 last_index_of(const text_view& pattern, const index_interval& range = index_interval::all()) const noexcept
	{
		if(pattern.is_empty())
			return index_invalid;
		const index_interval sequence_range = this->get_codeunit_range(range);
		const i32 found_sequence_index = this->view_.last_index_of(pattern.view_, sequence_range);
		if(found_sequence_index == index_invalid)
			return index_invalid;
		return this->get_codepoint_index(found_sequence_index);
	}
	
	[[nodiscard]] i32 count(const text_view& pattern) const noexcept
	{
		return this->view_.count(pattern.view_);
	}

	[[nodiscard]] constexpr bool contains(const text_view& pattern) const noexcept
	{
		return this->view_.contains(pattern.view_);
	}

	[[nodiscard]] constexpr bool contains(const codepoint& pattern) const noexcept
	{
		return this->contains(text_view{ pattern });
	}

	[[nodiscard]] constexpr std::tuple<text_view, text_view> split(const text_view& splitter) const noexcept
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

	[[nodiscard]] constexpr text_view trim_start(const text_view& text_set = text_view(OSTR_UTF8(" \t"))) const noexcept
	{
		if(this->is_empty())
			return { };
		if(text_set.is_empty())
			return *this;
		auto iterator = this->begin();
		const i32 size = this->size();
		for(i32 i = 0; i < size; ++i)
		{
			if(const auto cp = *iterator; !text_set.contains(cp))
				return this->subview({ '[', i, '~' });
			++iterator;
		}
		return { };
	}

	[[nodiscard]] constexpr text_view trim_end(const text_view& text_set = text_view(OSTR_UTF8(" \t"))) const noexcept
	{
		if(this->is_empty())
			return { };
		if(text_set.is_empty())
			return *this;
		auto iterator = this->end();
		const i32 size = this->size();
		for(i32 i = size - 1; i >= 0; --i)
		{
			--iterator;
			if(const auto cp = *iterator; !text_set.contains(cp))
				return this->subview({ '[', 0, i, ']' });
		}
		return { };
	}

	[[nodiscard]] constexpr text_view trim(const text_view& text_set = text_view(OSTR_UTF8(" \t"))) const noexcept
	{
		return this->trim_start(text_set).trim_end(text_set);
	}

	[[nodiscard]] constexpr u32 get_hash() const noexcept
	{
		return this->view_.get_hash();
	}

	[[nodiscard]] constexpr i32 get_codepoint_index(const i32 codeunit_index) const noexcept
	{
		const i32 view_size = this->view_.size();
		i32 index = 0;
		i32 offset = 0;
		while(offset < codeunit_index && offset < view_size)
		{
			const i32 sequence_length = unicode::parse_utf8_length(this->u8_str()[offset]);
			const i32 offset_delta = sequence_length == 0 ? 1 : sequence_length;
			offset += offset_delta;
			++index;
		}
		return index;
	}

	[[nodiscard]] constexpr i32 get_codeunit_index(const i32 codepoint_index) const noexcept
	{
		const i32 view_size = this->view_.size();
		i32 index = 0;
		i32 offset = 0;
		while(index < codepoint_index && offset < view_size)
		{
			const i32 sequence_length = unicode::parse_utf8_length(this->u8_str()[offset]);
			offset += sequence_length;
			++index;
		}
		return offset;
	}

	[[nodiscard]] index_interval get_codeunit_range(const index_interval& text_range) const noexcept
	{
		const index_interval selection = text_range.is_finite_positive() ? text_range : text_range.select(this->size());
		const i32 selection_lower_bound = selection.get_inclusive_min();
		const i32 selection_upper_bound = selection.get_exclusive_max();
		const i32 sequence_lower_bound = this->get_codeunit_index(selection_lower_bound);
		const i32 sequence_upper_bound = this->get_codeunit_index(selection_upper_bound);
		return { '[', sequence_lower_bound, sequence_upper_bound, ')' };
	}
	
private:
	
	codeunit_sequence_view view_;
	
};

template<> 
struct argument_formatter<text_view>
{
    static codeunit_sequence produce(const text_view& value, const codeunit_sequence_view& specification)
    {
        return codeunit_sequence{ value.raw() };
    }
};

OPEN_STRING_NS_END

inline namespace literal
{
	[[nodiscard]] constexpr OPEN_STRING_NS::text_view operator""_txtv(const ochar8_t* str, const size_t len) noexcept
	{
		return { str, len };
	}
}
