// OpenString - human-readable string
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#include <algorithm>
#include <memory.h>
#include "text.h"
#include "adapters.h"

OPEN_STRING_NS_BEGIN

text::text() noexcept = default;
text::text(const text&) noexcept = default;
text::text(text&&) noexcept = default;
text& text::operator=(const text&) noexcept = default;
text& text::operator=(text&&) noexcept = default;
text::~text() = default;

text::text(const ochar8_t* str) noexcept
	: sequence_(str)
{ }

text::text(const text_view view) noexcept
	: sequence_(view.raw())
{ }

text::text(codeunit_sequence sequence) noexcept
	: sequence_(std::move(sequence))
{ }

text::text(codeunit_sequence_view sequence) noexcept
	: sequence_(sequence)
{ }

text text::from_utf8(const ochar8_t* str) noexcept
{
	return { codeunit_sequence_view(str) };
}

text text::from_utf32(const char32_t* str) noexcept
{
	const char32_t* p = str;
	i32 size = 0;
	while(*p != 0)
	{
		size += unicode::parse_utf8_length(*p);
		++p;
	}
	codeunit_sequence sequence(size);
	p = str;
	while(*p != 0)
	{
		sequence += codepoint(*p);
		++p;
	}
	return { std::move(sequence) };
}

text::iterator::iterator(text& t, const index_interval& r) noexcept
	: sequence_range(r)
	, owner(&t)
{ }

text::iterator::codepoint_accessor::codepoint_accessor(iterator& iter) noexcept
	: it_(iter)
{ }

text::iterator::codepoint_accessor& text::iterator::codepoint_accessor::operator=(const ochar8_t c) noexcept
{
	return *this = codepoint{ c };
}

text::iterator::codepoint_accessor& text::iterator::codepoint_accessor::operator=(const char32_t cp) noexcept
{
	return *this = codepoint{ cp };
}

text::iterator::codepoint_accessor& text::iterator::codepoint_accessor::operator=(const codepoint& cp) noexcept
{
	this->assign(codeunit_sequence_view{ cp });
	return *this;
}

text::iterator::codepoint_accessor& text::iterator::codepoint_accessor::operator=(const text_view& tv) noexcept
{
	this->assign(tv.raw());
	return *this;
}

text::iterator::codepoint_accessor& text::iterator::codepoint_accessor::operator=(const text& t) noexcept
{
	return *this = t.view();
}

codepoint text::iterator::codepoint_accessor::get_codepoint() const noexcept
{
	return it_.get_codepoint();
}

text::iterator::codepoint_accessor::operator codepoint() const noexcept
{
	return this->get_codepoint();
}

void text::iterator::codepoint_accessor::assign(const codeunit_sequence_view& sequence_view) const noexcept
{
	it_.owner->sequence_.replace(it_.sequence_range, sequence_view);
	const i32 origin_start = it_.sequence_range.get_inclusive_min();
	it_.sequence_range = { '[', origin_start, origin_start + sequence_view.size(), ')' };
}

i32 text::iterator::raw_size() const noexcept
{
	return this->sequence_range.size();
}

codepoint text::iterator::get_codepoint() const noexcept
{
	return codepoint{ this->owner->sequence_.subview( this->sequence_range ).u8_str(), this->sequence_range.size() };
}

codepoint text::iterator::operator*() const noexcept
{
	return this->get_codepoint();
}

text::iterator::codepoint_accessor text::iterator::operator*() noexcept
{
	return codepoint_accessor{ *this };
}

text::iterator& text::iterator::operator++() noexcept
{
	const i32 next_start = this->sequence_range.get_exclusive_max();
	const ochar8_t c = this->owner->sequence_[next_start];
	const i32 size = unicode::parse_utf8_length(c);
	this->sequence_range = { '[', next_start, next_start + size, ')' };
	return *this;
}

text::iterator text::iterator::operator++(int) noexcept
{
	const iterator tmp = *this;
	++*this;
	return tmp;
}

text::iterator& text::iterator::operator--() noexcept
{
	i32 prev_start = this->sequence_range.get_inclusive_min();
	i32 size = 0;
	while(size == 0)
	{
		--prev_start;
		size = unicode::parse_utf8_length(this->owner->sequence_[prev_start]);
	}
	this->sequence_range = { '[', prev_start, prev_start + size, ')' };
	return *this;
}

text::iterator text::iterator::operator--(int) noexcept
{
	const iterator tmp = *this;
	--*this;
	return tmp;
}

bool text::iterator::operator==(const iterator& rhs) const noexcept
{
	return this->owner == rhs.owner && this->sequence_range == rhs.sequence_range;
}

bool text::iterator::operator!=(const iterator& rhs) const noexcept
{
	return !(*this == rhs);
}

text::iterator text::begin() noexcept
{
	return iterator{ *this, { '[', 0, this->cbegin().raw_size(), ')' } };
}

text::const_iterator text::begin() const noexcept
{
	return this->view().begin();
}

text::iterator text::end() noexcept
{
	return iterator{ *this, index_interval::empty() };
}

text::const_iterator text::end() const noexcept
{
	return this->view().end();
}

text::const_iterator text::cbegin() const noexcept
{
	return this->view().cbegin();
}

text::const_iterator text::cend() const noexcept
{
	return this->view().cend();
}

codeunit_sequence text::raw() && noexcept
{
	return std::forward<codeunit_sequence>(this->sequence_);
}

const codeunit_sequence& text::raw() const& noexcept
{
	return this->sequence_;
}

text_view text::view() const noexcept
{
	return this->sequence_.view();
}

i32 text::size() const noexcept
{
	return this->view().size();
}

bool text::is_empty() const noexcept
{
	return this->sequence_.is_empty();
}

bool text::operator==(const text_view& rhs) const noexcept
{
	return this->view() == rhs;
}

bool text::operator==(const text& rhs) const noexcept
{
	return this->view() == rhs.view();
}

bool text::operator==(const ochar8_t* rhs) const noexcept
{
	return this->view() == rhs;
}

bool text::operator!=(const text_view& rhs) const noexcept
{
	return this->view() != rhs;
}

bool text::operator!=(const text& rhs) const noexcept
{
	return this->view() != rhs.view();
}

bool text::operator!=(const ochar8_t* rhs) const noexcept
{
	return this->view() != rhs;
}

text& text::append(const text_view& rhs) noexcept
{
	this->sequence_.append(rhs.raw());
	return *this;
}

text& text::append(const text& rhs) noexcept
{
	return this->append(rhs.view());
}

text& text::append(const codepoint& cp) noexcept
{
	this->sequence_.append(cp);
	return *this;
}

text& text::append(const ochar8_t* rhs) noexcept
{
	this->sequence_.append(rhs);
	return *this;
}

text& text::append(const ochar8_t codeunit, const i32 count) noexcept
{
	this->sequence_.append(codeunit, count);
	return *this;
}

text& text::operator+=(const text_view& rhs) noexcept
{
	return this->append(rhs);
}

text& text::operator+=(const text& rhs) noexcept
{
	return this->append(rhs);
}

text& text::operator+=(const codepoint& cp) noexcept
{
	return this->append(cp);
}

text& text::operator+=(const ochar8_t* rhs) noexcept
{
	return this->append(rhs);
}

text& text::operator+=(const ochar8_t codeunit) noexcept
{
	return this->append(codeunit);
}

text_view text::subview(const index_interval& range) const noexcept
{
	return this->view().subview(range);
}

text& text::subtext(const index_interval& range) noexcept
{
	const i32 self_size = this->size();
	const index_interval selection = range.select(self_size);
	if(selection.is_empty())
	{
		this->empty();
		return *this;
	}
	if(selection == index_interval::from_universal(self_size))
		// Do nothing
		return *this;
	const i32 lower_bound = this->view().get_codepoint_index( selection.get_inclusive_min() );
	const i32 upper_bound = this->view().get_codepoint_index( selection.get_exclusive_max() );
	this->sequence_.subsequence({ '[', lower_bound, upper_bound, ')' });
	return *this;
}

i32 text::index_of(const text_view& pattern, const index_interval& range) const noexcept
{
	return this->view().index_of(pattern, range);
}

i32 text::last_index_of(const text_view& pattern, const index_interval& range) const noexcept
{
	return this->view().last_index_of(pattern, range);
}

i32 text::count(const text_view& pattern) const noexcept
{
	return this->view().count(pattern);
}

bool text::starts_with(const text_view& prefix) const noexcept
{
	return this->view().starts_with(prefix);
}

bool text::ends_with(const text_view& suffix) const noexcept
{
	return this->view().ends_with(suffix);
}

void text::empty() noexcept
{
	this->sequence_.empty();
}

text& text::write_at(const i32 index, const codepoint cp) noexcept
{
	this->replace({ '[', index, index, ']' }, text_view(cp));
	return *this;
}

codepoint text::read_at(const i32 index) const noexcept
{
	return this->view().read_at(index);
}

codepoint text::operator[](const i32 index) const noexcept
{
	return this->view()[index];
}

text& text::reverse(const index_interval& range) noexcept
{
	const index_interval sequence_range = this->view().get_codeunit_range(range);
	this->sequence_.reverse(sequence_range);
	i32 lower_bound = sequence_range.get_inclusive_min();
	for(const i32 i : sequence_range)
	{
		if(unicode::parse_utf8_length( this->sequence_[i] ) != 0)
		{
			this->sequence_.reverse({ '[', lower_bound, i, ']' });
			lower_bound = i + 1;
		}
	}
	return *this;
}

[[nodiscard]] std::vector<text_view> text::split(const text_view& splitter, const bool cull_empty) const noexcept
{
	std::vector<text_view> pieces;
	this->split(splitter, pieces, cull_empty);
	return pieces;
}

u32 text::split(const text_view& splitter, std::vector<text_view>& pieces, const bool cull_empty) const noexcept
{
	text_view view = this->view();
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

text& text::replace(const text_view& source, const text_view& destination, const index_interval& range)
{
	const index_interval codeunit_range = this->view().get_codeunit_range(range);
	this->sequence_.replace(source.raw(), destination.raw(), codeunit_range);
	return *this;
}

text& text::replace(const index_interval& range, const text_view& destination)
{
	const index_interval codeunit_range = this->view().get_codeunit_range(range);
	this->sequence_.replace(codeunit_range, destination.raw());
	return *this;
}

text& text::self_remove_prefix(const text_view& prefix) noexcept
{
	this->sequence_.self_remove_prefix(prefix.raw());
	return *this;
}

text& text::self_remove_suffix(const text_view& suffix) noexcept
{
	this->sequence_.self_remove_suffix(suffix.raw());
	return *this;
}

text_view text::view_remove_prefix(const text_view& prefix) const noexcept
{
	return this->view().remove_prefix(prefix);
}

text_view text::view_remove_suffix(const text_view& suffix) const noexcept
{
	return this->view().remove_suffix(suffix);
}

text& text::self_trim_start(const text_view& characters) noexcept
{
	if(this->is_empty())
		return *this;
	i32 codeunit_index = 0;
	for(auto it = this->cbegin(); it != this->cend(); ++it)
	{
		if(!characters.contains(it.get_codepoint()))
			break;
		codeunit_index += it.raw_size();
	}
	this->sequence_.subsequence({ '[', codeunit_index, '~' });
	return *this;
}

text& text::self_trim_end(const text_view& characters) noexcept
{
	if(this->is_empty())
		return *this;
	i32 codeunit_index = 0;
	auto it = this->cend();
	while(true)
	{
		--it;
		if(!characters.contains(it.get_codepoint()))
			break;
		codeunit_index -= it.raw_size();
		if(it == this->cbegin())
			break;
	}
	this->sequence_.subsequence({ '[', 0, codeunit_index, ')' });
	return *this;
}

text& text::self_trim(const text_view& characters) noexcept
{
	return this->self_trim_end(characters).self_trim_start(characters);
}

text_view text::view_trim_start(const text_view& characters) const noexcept
{
	return this->view().trim_start(characters);
}

text_view text::view_trim_end(const text_view& characters) const noexcept
{
	return this->view().trim_end(characters);
}

text_view text::view_trim(const text_view& characters) const noexcept
{
	return this->view().trim(characters);
}

u32 text::get_hash() const noexcept
{
	return this->view().get_hash();
}

const ochar_t* text::c_str() const noexcept
{
	return this->view().c_str();
}

const ochar8_t* text::u8_str() const noexcept
{
	return this->view().u8_str();
}

bool operator==(const text_view& lhs, const text& rhs) noexcept
{
	return rhs == lhs;
}

OPEN_STRING_NS_END

