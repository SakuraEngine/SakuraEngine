#pragma once

#include <vector>
#include "codeunit_sequence_view.h"

OPEN_STRING_NS_BEGIN

class OPEN_STRING_API codeunit_sequence
{
public:

// code-region-start: constructors

	codeunit_sequence() noexcept;
	explicit codeunit_sequence(i32 size) noexcept;
	codeunit_sequence(const codeunit_sequence&) noexcept;
	codeunit_sequence(codeunit_sequence&&) noexcept;
	codeunit_sequence& operator=(const codeunit_sequence&) noexcept;
	codeunit_sequence& operator=(codeunit_sequence&&) noexcept;
	
	codeunit_sequence& operator=(const codeunit_sequence_view& view) noexcept;

	~codeunit_sequence() noexcept;

	explicit codeunit_sequence(const ochar8_t* data) noexcept;
	codeunit_sequence(const ochar8_t* from, const ochar8_t* last) noexcept;
	codeunit_sequence(const ochar8_t* data, i32 count) noexcept;

	explicit codeunit_sequence(codeunit_sequence_view sv) noexcept;

	template<class...Args>
	static codeunit_sequence build(const Args&... argument);
	template<typename Container>
	static codeunit_sequence join(const Container& container, const codeunit_sequence_view& separator) noexcept;

// code-region-end: constructors

// code-region-start: iterators

	using const_iterator = codeunit_sequence_view::const_iterator;

	struct iterator
	{
		iterator() noexcept;
		explicit iterator(ochar8_t* v) noexcept;
		[[nodiscard]] ochar8_t& operator*() const noexcept;
		[[nodiscard]] std::ptrdiff_t operator-(const iterator& rhs) const noexcept;
		iterator& operator+=(std::ptrdiff_t diff) noexcept;
		iterator& operator-=(std::ptrdiff_t diff) noexcept;
		[[nodiscard]] iterator operator+(std::ptrdiff_t diff) const noexcept;
		[[nodiscard]] iterator operator-(std::ptrdiff_t diff) const noexcept;
	    iterator& operator++() noexcept;
	    iterator operator++(int) noexcept;
		iterator& operator--() noexcept;
		iterator operator--(int) noexcept;
	    [[nodiscard]] bool operator==(const iterator& rhs) const noexcept;
	    [[nodiscard]] bool operator!=(const iterator& rhs) const noexcept;
	    [[nodiscard]] bool operator<(const iterator& rhs) const noexcept;
	    [[nodiscard]] bool operator>(const iterator& rhs) const noexcept;
	    [[nodiscard]] bool operator<=(const iterator& rhs) const noexcept;
	    [[nodiscard]] bool operator>=(const iterator& rhs) const noexcept;
		
		ochar8_t* value;
	};
	
	[[nodiscard]] iterator begin() noexcept;
	[[nodiscard]] const_iterator begin() const noexcept;
	[[nodiscard]] iterator end() noexcept;
	[[nodiscard]] const_iterator end() const noexcept;
	[[nodiscard]] const_iterator cbegin() const noexcept;
	[[nodiscard]] const_iterator cend() const noexcept;

// code-region-end: iterators

	[[nodiscard]] codeunit_sequence_view view() const& noexcept;
	/**
	 * FATAL: You must NOT get view from rvalue of a codeunit sequence
	 */
	codeunit_sequence_view view() && = delete;

	/**
	 * @return The length of this codeunit sequence
	 */
	[[nodiscard]] i32 size() const noexcept;
	
	/**
	 * @return Whether this codeunit sequence is empty or not.
	 */
	[[nodiscard]] bool is_empty() const noexcept;

	/**
	 * @param rhs Another codeunit sequence
	 * @return Whether two codeunit sequences are equal.
	 */
	[[nodiscard]] bool operator==(codeunit_sequence_view rhs) const noexcept;
	[[nodiscard]] bool operator==(const codeunit_sequence& rhs) const noexcept;
	[[nodiscard]] bool operator==(const ochar8_t* rhs) const noexcept;

	/**
	 * @param rhs Another codeunit sequence
	 * @return Whether two codeunit sequences are different.
	 */
	[[nodiscard]] bool operator!=(codeunit_sequence_view rhs) const noexcept;
	[[nodiscard]] bool operator!=(const codeunit_sequence& rhs) const noexcept;
	[[nodiscard]] bool operator!=(const ochar8_t* rhs) const noexcept;

	/**
	 * Append a codeunit sequence back.
	 * @return ref of this codeunit sequence.
	 */
	codeunit_sequence& append(const codeunit_sequence_view& rhs) noexcept;
	codeunit_sequence& append(const codeunit_sequence& rhs) noexcept;
	codeunit_sequence& append(const codepoint& cp) noexcept;
	codeunit_sequence& append(const ochar8_t* rhs) noexcept;
	codeunit_sequence& append(ochar8_t codeunit, i32 count = 1) noexcept;

	codeunit_sequence& operator+=(const codeunit_sequence_view& rhs) noexcept;
	codeunit_sequence& operator+=(const codeunit_sequence& rhs) noexcept;
	codeunit_sequence& operator+=(const codepoint& cp) noexcept;
	codeunit_sequence& operator+=(const ochar8_t* rhs) noexcept;
	codeunit_sequence& operator+=(ochar8_t codeunit) noexcept;

	[[nodiscard]] codeunit_sequence_view subview(const index_interval& range) const noexcept;

	/**
	 * Make this a subsequence from specific range
	 * 
	 * Example: codeunit_sequence("codeunit_sequence").subsequence(2, 3) == "ar_";
	 * 
	 * @param range Given range
	 * @return ref of this codeunit sequence
	 */
	codeunit_sequence& subsequence(const index_interval& range) noexcept;

	/**
	 * Get the index of specific codeunit_sequence.
	 * 
	 * Example: codeunit_sequence("codeunit_sequence").index_of("ar_") == 2;
	 * 
	 * @param pattern pattern to search.
	 * @param range range to search.
	 * @return the index of where the codeunit_sequence first found
	 */
	[[nodiscard]] i32 index_of(const codeunit_sequence_view& pattern, const index_interval& range = index_interval::all()) const noexcept;
	[[nodiscard]] i32 last_index_of(const codeunit_sequence_view& pattern, const index_interval& range = index_interval::all()) const noexcept;
	
	[[nodiscard]] i32 count(const codeunit_sequence_view& pattern) const noexcept;

	[[nodiscard]] bool starts_with(const codeunit_sequence_view& pattern) const noexcept;
	[[nodiscard]] bool ends_with(const codeunit_sequence_view& pattern) const noexcept;

	/**
	 * Empty the string without reallocation.
	 */
	void empty();
	/**
	 * Empty the string with size reserved.
	 * @param size the size reserved
	 */
	void empty(i32 size);
	
	void reserve(i32 size);

	codeunit_sequence& write_at(i32 index, ochar8_t codeunit) noexcept;
	[[nodiscard]] const ochar8_t& read_at(i32 index) const noexcept;

	[[nodiscard]] ochar8_t& operator[](i32 index) noexcept;
	[[nodiscard]] const ochar8_t& operator[](i32 index) const noexcept;

	codeunit_sequence& reverse(const index_interval& range = index_interval::all()) noexcept;

	[[nodiscard]] std::vector<codeunit_sequence_view> split(const codeunit_sequence_view& splitter, bool cull_empty = true) const noexcept;
	u32 split(const codeunit_sequence_view& splitter, std::vector<codeunit_sequence_view>& pieces, bool cull_empty = true) const noexcept;

	codeunit_sequence& replace(const codeunit_sequence_view& source, const codeunit_sequence_view& destination, const index_interval& range = index_interval::all());
	codeunit_sequence& replace(const index_interval& range, const codeunit_sequence_view& destination);

	codeunit_sequence& self_remove_prefix(const codeunit_sequence_view& prefix) noexcept;
	codeunit_sequence& self_remove_suffix(const codeunit_sequence_view& suffix) noexcept;
	[[nodiscard]] codeunit_sequence_view view_remove_prefix(const codeunit_sequence_view& prefix) const noexcept;
	[[nodiscard]] codeunit_sequence_view view_remove_suffix(const codeunit_sequence_view& suffix) const noexcept;

	codeunit_sequence& self_trim_start(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) noexcept;
	codeunit_sequence& self_trim_end(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) noexcept;
	codeunit_sequence& self_trim(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) noexcept;
	[[nodiscard]] codeunit_sequence_view view_trim_start(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept;
	[[nodiscard]] codeunit_sequence_view view_trim_end(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept;
	[[nodiscard]] codeunit_sequence_view view_trim(const codeunit_sequence_view& characters = codeunit_sequence_view(OSTR_UTF8(" \t"))) const noexcept;

	[[nodiscard]] u32 get_hash() const noexcept;

	[[nodiscard]] const ochar8_t* u8_str() const noexcept;
	[[nodiscard]] const ochar8_t* c_str() const noexcept;

private:

	static constexpr u8 SSO_SIZE_MAX = 14;
	static bool is_short_size(i32 size) noexcept;

	struct sso
	{
		u8 alloc : 1;
		u8 size : 7;
		std::array<ochar8_t, SSO_SIZE_MAX + 1> data;
	};

	struct norm
	{
		u32 alloc : 1;
		i32 size : 15;
		i32 capacity;	// character capacity, which is 1 less than memory capacity
		ochar8_t* data;
	};

	[[nodiscard]] sso& as_sso();
	[[nodiscard]] const sso& as_sso() const;

	[[nodiscard]] norm& as_norm();
	[[nodiscard]] const norm& as_norm() const;

	/// @return is this a sequence with less than 15 chars
	[[nodiscard]] bool is_short() const;

	[[nodiscard]] i32 get_capacity() const;

	[[nodiscard]] ochar8_t* data();

	[[nodiscard]] const ochar8_t* data() const;

	[[nodiscard]] ochar8_t* last();

	[[nodiscard]] const ochar8_t* last() const;

	void deallocate();

	void set_size(i32 size);

	void transfer_data(codeunit_sequence& other);

	std::array<u8, 16> store_;
};

namespace details
{
	template<class T>
	[[nodiscard]] inline codeunit_sequence_view view_sequence(const T& v)
	{
		return codeunit_sequence_view{ v };
	}

	template<>
	[[nodiscard]] inline codeunit_sequence_view view_sequence<codeunit_sequence>(const codeunit_sequence& v)
	{
		return v.view();
	}
}

template<class...Args>
codeunit_sequence codeunit_sequence::build(const Args&... argument)
{
	i32 size = 0;
	std::array<codeunit_sequence_view, sizeof...(Args)> arguments{ details::view_sequence<Args>(argument)... };
	for(const codeunit_sequence_view& a : arguments)
		size += a.size();
	codeunit_sequence result(size);
	for(const codeunit_sequence_view& a : arguments)
		result.append(a);
	return { result };
}

template<typename Container>
codeunit_sequence codeunit_sequence::join(const Container& container, const codeunit_sequence_view& separator) noexcept
{
	codeunit_sequence result;
	for(const auto& element : container)
	{
		if(!result.is_empty())
			result += separator;
		result.append(details::view_sequence(element));
	}
	return result;
}

[[nodiscard]] bool operator==(const codeunit_sequence_view& lhs, const codeunit_sequence& rhs) noexcept;

OPEN_STRING_NS_END
