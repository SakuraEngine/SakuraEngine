// OpenString - human-readable string
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once

#include "text_view.h"
#include "codeunit_sequence.h"

OPEN_STRING_NS_BEGIN

class OPEN_STRING_API text
{
public:

	text() noexcept;
	text(const text&) noexcept;
	text(text&&) noexcept;
	text& operator=(const text&) noexcept;
	text& operator=(text&&) noexcept;

	~text();

	text(const char* str) noexcept;
	text(text_view view) noexcept;
	text(codeunit_sequence sequence) noexcept;
	text(codeunit_sequence_view sequence) noexcept;

	static text from_utf8(const char* str) noexcept;
	static text from_utf16(const char16_t* str) noexcept;
	static text from_utf32(const char32_t* str) noexcept;
	
	template<class...Args>
	static text build(const Args&... argument);
	template<typename Container>
	static text join(const Container& container, const text_view& separator) noexcept;

// code-region-start: iterator

	using const_iterator = text_view::const_iterator;

	struct iterator
	{
		iterator() noexcept = default;
		iterator(text& t, const index_interval& r) noexcept;

		struct codepoint_accessor
		{
			explicit codepoint_accessor(iterator& iter) noexcept;
			
			codepoint_accessor& operator=(char c) noexcept;
			codepoint_accessor& operator=(char32_t cp) noexcept;
			codepoint_accessor& operator=(const codepoint& cp) noexcept;
			codepoint_accessor& operator=(const text_view& tv) noexcept;
			codepoint_accessor& operator=(const text& t) noexcept;

			[[nodiscard]] codepoint get_codepoint() const noexcept;
			[[nodiscard]] operator codepoint() const noexcept;

		private:
			
			void assign(const codeunit_sequence_view& sequence_view) const noexcept;
			
			iterator& it_;
		};

		[[nodiscard]] i32 raw_size() const noexcept;
		[[nodiscard]] codepoint get_codepoint() const noexcept;
		[[nodiscard]] codepoint operator*() const noexcept;
		[[nodiscard]] codepoint_accessor operator*() noexcept;
		iterator& operator++() noexcept;
		iterator operator++(int) noexcept;
		iterator& operator--() noexcept;
		iterator operator--(int) noexcept;
		[[nodiscard]] bool operator==(const iterator& rhs) const noexcept;
		[[nodiscard]] bool operator!=(const iterator& rhs) const noexcept;

		index_interval sequence_range = index_interval::empty();
		text* owner = nullptr;
	};
	
	[[nodiscard]] iterator begin() noexcept;
	[[nodiscard]] const_iterator begin() const noexcept;
	[[nodiscard]] iterator end() noexcept;
	[[nodiscard]] const_iterator end() const noexcept;
	[[nodiscard]] const_iterator cbegin() const noexcept;
	[[nodiscard]] const_iterator cend() const noexcept;

// code-region-end: iterator

	[[nodiscard]] codeunit_sequence raw() && noexcept;
	[[nodiscard]] const codeunit_sequence& raw() const& noexcept;
	
	[[nodiscard]] text_view view() const noexcept;

	[[nodiscard]] i32 size() const noexcept;
	[[nodiscard]] bool is_empty() const noexcept;

	[[nodiscard]] bool operator==(const text_view& rhs) const noexcept;
	[[nodiscard]] bool operator==(const text& rhs) const noexcept;
	[[nodiscard]] bool operator==(const char* rhs) const noexcept;
	[[nodiscard]] bool operator!=(const text_view& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const text& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const char* rhs) const noexcept;

	text& append(const text_view& rhs) noexcept;
	text& append(const text& rhs) noexcept;
	text& append(const codepoint& cp) noexcept;
	text& append(const char* rhs) noexcept;
	text& append(char codeunit, i32 count = 1) noexcept;

	text& operator+=(const text_view& rhs) noexcept;
	text& operator+=(const text& rhs) noexcept;
	text& operator+=(const codepoint& cp) noexcept;
	text& operator+=(const char* rhs) noexcept;
	text& operator+=(char codeunit) noexcept;
	
	[[nodiscard]] text_view subview(const index_interval& range) const noexcept;

	text& subtext(const index_interval& range) noexcept;
	
	[[nodiscard]] i32 index_of(const text_view& pattern, const index_interval& range = index_interval::all()) const noexcept;
	[[nodiscard]] i32 last_index_of(const text_view& pattern, const index_interval& range = index_interval::all()) const noexcept;
	
	[[nodiscard]] i32 count(const text_view& pattern) const noexcept;

	[[nodiscard]] bool starts_with(const text_view& prefix) const noexcept;

	[[nodiscard]] bool ends_with(const text_view& suffix) const noexcept;

	/**
	 * Empty the text without reallocation.
	 */
	void empty() noexcept;

	text& write_at(i32 index, codepoint cp) noexcept;
	[[nodiscard]] codepoint read_at(i32 index) const noexcept;
	
	/// @note: Please use operator[] for read-only access, or use write_at method for write access.
	[[nodiscard]] codepoint operator[](i32 index) const noexcept;

	text& reverse(const index_interval& range = index_interval::all()) noexcept;

	[[nodiscard]] std::vector<text_view> split(const text_view& splitter, bool cull_empty = true) const noexcept;
	u32 split(const text_view& splitter, std::vector<text_view>& pieces, bool cull_empty = true) const noexcept;

	text& replace(const text_view& source, const text_view& destination, const index_interval& range = index_interval::all());
	text& replace(const index_interval& range, const text_view& destination);
	
	text& self_remove_prefix(const text_view& prefix) noexcept;
	text& self_remove_suffix(const text_view& suffix) noexcept;
	[[nodiscard]] text_view view_remove_prefix(const text_view& prefix) const noexcept;
	[[nodiscard]] text_view view_remove_suffix(const text_view& suffix) const noexcept;

	text& self_trim_start(const text_view& characters = text_view(" \t")) noexcept;
	text& self_trim_end(const text_view& characters = text_view(" \t")) noexcept;
	text& self_trim(const text_view& characters = text_view(" \t")) noexcept;

	[[nodiscard]] text_view view_trim_start(const text_view& characters = text_view(" \t")) const noexcept;
	[[nodiscard]] text_view view_trim_end(const text_view& characters = text_view(" \t")) const noexcept;
	[[nodiscard]] text_view view_trim(const text_view& characters = text_view(" \t")) const noexcept;

	[[nodiscard]] u32 get_hash() const noexcept;

	[[nodiscard]] const char* c_str() const noexcept;

private:

	codeunit_sequence sequence_;
	
};

namespace details
{
	template<>
	[[nodiscard]] inline codeunit_sequence_view view_sequence<text_view>(const text_view& v)
	{
		return v.raw();
	}
	template<>
	[[nodiscard]] inline codeunit_sequence_view view_sequence<text>(const text& v)
	{
		return v.view().raw();
	}
}

template<class...Args>
text text::build(const Args&... argument)
{
	return { codeunit_sequence::build(argument...) };
}

template<typename Container>
text text::join(const Container& container, const text_view& separator) noexcept
{
	return { codeunit_sequence::join(container, separator.raw()) };
}

[[nodiscard]] bool operator==(const text_view& lhs, const text& rhs) noexcept;

template<> 
struct formatter<text>
{
    static codeunit_sequence format_argument(const text& value, const codeunit_sequence_view& specification)
    {
        return value.raw();
    }
};

OPEN_STRING_NS_END
