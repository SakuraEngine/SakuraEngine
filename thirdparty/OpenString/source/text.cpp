
#include "text.h"

#include <algorithm>
#include "common/functions.h"
#include "wide_text.h"

namespace ostr
{
	text::text() noexcept = default;
	text::text(const text&) noexcept = default;
	text::text(text&&) noexcept = default;
	text& text::operator=(const text&) noexcept = default;
	text& text::operator=(text&&) noexcept = default;
	text::~text() = default;

	text::text(const ochar8_t* str) noexcept
		: sequence_{ str }
	{ }

	text::text(const text_view& view) noexcept
		: sequence_{ view.raw() }
	{ }

	text::text(codeunit_sequence sequence) noexcept
		: sequence_{ std::move(sequence) }
	{ }

	text::text(const codeunit_sequence_view& sequence) noexcept
		: sequence_{ sequence }
	{ }

	text text::from_utf8(const ochar8_t* string_utf8) noexcept
	{
		return text{ codeunit_sequence_view{ string_utf8 } };
	}

	text text::from_utf16(const char16_t* string_utf16) noexcept
	{
		const char16_t* p = string_utf16;
		u64 size = 0;
		while(*p != 0)
		{
			size += unicode::parse_utf8_length(*p);
			++p;
		}
		codeunit_sequence sequence{ size + 1 };
		p = string_utf16;
		while(*p != 0)
		{
			const u64 surrogate_sequence_length = unicode::utf16::parse_utf16_length(*p);
			sequence.append(codepoint{ p });
			p += surrogate_sequence_length;
		}
		return text{ std::move(sequence) };
	}

	text text::from_utf32(const char32_t* string_utf32) noexcept
	{
		const char32_t* p = string_utf32;
		u64 size = 0;
		while(*p != 0)
		{
			size += unicode::parse_utf8_length(*p);
			++p;
		}
		codeunit_sequence sequence{ size };
		p = string_utf32;
		while(*p != 0)
		{
			sequence.append(codepoint{ *p });
			++p;
		}
		return text{ std::move(sequence) };
	}

	text text::from_wide(const wchar_t* wide_string) noexcept
	{
		const wide_text wt{ wide_string };
		codeunit_sequence decoded;
		wt.decode(decoded);
		return text{ std::move(decoded) };
	}

	text::iterator::iterator(text& t, const u64 from, const u64 size) noexcept
		: from{ from }
		, size{ size }
		, owner{ &t }
	{ }

	text::iterator::codepoint_accessor::codepoint_accessor(iterator& iter) noexcept
		: it_{ iter }
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
		return this->it_.get_codepoint();
	}

	text::iterator::codepoint_accessor::operator codepoint() const noexcept
	{
		return this->get_codepoint();
	}

	void text::iterator::codepoint_accessor::assign(const codeunit_sequence_view& sequence_view) const noexcept
	{
		if(this->it_.is_valid())
		{
			this->it_.owner->sequence_.replace(sequence_view, it_.from, it_.size);
			this->it_.size = sequence_view.size();
		}
	}

	bool text::iterator::is_valid() const noexcept
	{
		return this->from != global_constant::INDEX_INVALID;
	}

	u64 text::iterator::raw_size() const noexcept
	{
		return this->size;
	}

	codepoint text::iterator::get_codepoint() const noexcept
	{
		const codeunit_sequence_view subview = this->owner->sequence_.subview( this->from, this->size );
		return codepoint{ subview.data(), static_cast<u8>(subview.size()) };
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
		const u64 next_start = this->from + this->size;
		const ochar8_t c = this->owner->sequence_.read_at(next_start);
		if(const u8 code_size = unicode::parse_utf8_length(c); code_size != 0)
		{
			this->from = next_start;
			this->size = code_size;
		}
		else
		{
			this->from = global_constant::INDEX_INVALID;
			this->size = SIZE_MAX;
		}
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
		u64 current_from = this->from;
		u8 code_size = 0;
		while(code_size == 0)
		{
			--current_from;
			code_size = unicode::parse_utf8_length(this->owner->sequence_.read_at(current_from));
		}
		this->from = current_from;
		this->size = code_size;
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
		return this->owner == rhs.owner && this->from == rhs.from && this->size == rhs.size;
	}

	bool text::iterator::operator!=(const iterator& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	text::iterator text::begin() noexcept
	{
		return iterator{ *this, 0, this->cbegin().raw_size() };
	}

	text::const_iterator text::begin() const noexcept
	{
		return this->view().begin();
	}

	text::iterator text::end() noexcept
	{
		return iterator{ *this, global_constant::INDEX_INVALID, global_constant::SIZE_INVALID };
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

	codeunit_sequence& text::raw() & noexcept
	{
		return this->sequence_;
	}

	const codeunit_sequence& text::raw() const& noexcept
	{
		return this->sequence_;
	}

	codeunit_sequence text::raw() && noexcept
	{
		return std::forward<codeunit_sequence>(this->sequence_);
	}

	text_view text::view() const noexcept
	{
		return this->sequence_.view();
	}

	u64 text::size() const noexcept
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

	text& text::append(const ochar8_t codeunit, const u64 count) noexcept
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

	text_view text::subview(const u64 from, const u64 size) const noexcept
	{
		return this->view().subview(from, size);
	}

	text& text::subtext(const u64 from, const u64 size) noexcept
	{
		const u64 self_size = this->size();
		if(from >= self_size || size == 0)
		{
			this->empty();
			return *this;
		}
		if(from == 0 && size >= self_size)
			// Do nothing
			return *this;
		const u64 actual_size = minimum({ size, self_size - from });
		const u64 lower_bound = this->view().get_codepoint_index( from );
		const u64 upper_bound = this->view().get_codepoint_index( from + actual_size );
		this->sequence_.subsequence(lower_bound, upper_bound - lower_bound);
		return *this;
	}

	u64 text::index_of(const text_view& pattern, const u64 from, const u64 size) const noexcept
	{
		return this->view().index_of(pattern, from, size);
	}

	u64 text::last_index_of(const text_view& pattern, const u64 from, const u64 size) const noexcept
	{
		return this->view().last_index_of(pattern, from, size);
	}

	u64 text::count(const text_view& pattern, const u64 from, const u64 size) const noexcept
	{
		return this->view().count(pattern, from, size);
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

	text& text::write_at(const u64 index, const codepoint cp) noexcept
	{
		this->replace(text_view{ cp }, index, 1);
		return *this;
	}

	codepoint text::read_at(const u64 index) const noexcept
	{
		return this->view().read_at(index);
	}

	codepoint text::operator[](const u64 index) const noexcept
	{
		return this->view().read_at(index);
	}

	text& text::reverse(const u64 from, const u64 size) noexcept
	{
		u64 raw_from = from;
		u64 raw_size = size;
		this->view().get_codeunit_range(raw_from, raw_size);
		this->sequence_.reverse(raw_from, raw_size);
		for(u64 i = 0; i < raw_size; ++i)
			if(const u8 code_size = unicode::parse_utf8_length( this->sequence_.read_at(raw_from + i) ); code_size != 0)
				this->sequence_.reverse(raw_from + i + 1 - code_size, code_size);
		return *this;
	}

	u32 text::split(const text_view& splitter, sequence<text_view>& pieces, const bool cull_empty) const noexcept
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

	text& text::replace(const text_view& destination, const text_view& source, const u64 from, const u64 size)
	{
		u64 raw_from = from;
		u64 raw_size = size;
		this->view().get_codeunit_range(raw_from, raw_size);
		this->sequence_.replace(destination.raw(), source.raw(), raw_from, raw_size);
		return *this;
	}

	text& text::replace(const text_view& destination, const u64 from, const u64 size)
	{
		u64 raw_from = from;
		u64 raw_size = size;
		this->view().get_codeunit_range(raw_from, raw_size);
		this->sequence_.replace(destination.raw(), raw_from, raw_size);
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
		u64 codeunit_index = 0;
		for(auto it = this->cbegin(); it != this->cend(); ++it)
		{
			if(!characters.contains(it.get_codepoint()))
				break;
			codeunit_index += it.raw_size();
		}
		this->sequence_.subsequence(codeunit_index);
		return *this;
	}

	text& text::self_trim_end(const text_view& characters) noexcept
	{
		if(this->is_empty())
			return *this;
		u64 codeunit_index = this->raw().size();
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
		this->sequence_.subsequence(0, codeunit_index);
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

	const ochar_t* text::c_str() const noexcept
	{
		return this->sequence_.c_str();
	}

	const ochar8_t* text::u8_str() const noexcept
	{
		return this->sequence_.u8_str();
	}

	bool operator==(const text_view& lhs, const text& rhs) noexcept
	{
		return rhs == lhs;
	}
}

