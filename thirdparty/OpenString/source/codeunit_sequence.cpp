
#include <algorithm>
#include <memory.h>
#include "codeunit_sequence.h"
#include "adapters.h"

OPEN_STRING_NS_BEGIN

namespace details
{
	[[nodiscard]] constexpr i32 get_capacity(const i32 v) noexcept
	{
		u8 bit_pos = 0;
		i32 value = v;
		while(value != 0)
		{
			value >>= 1;
			++bit_pos;
		}
		i32 ans = 1 << bit_pos;
		if(ans == (v << 1))
			ans >>= 1;
		return ans;
	}
}

// code-region-start: iterators

codeunit_sequence::iterator::iterator() noexcept
	: value()
{ }

codeunit_sequence::iterator::iterator(char* v) noexcept
	: value(v)
{ }

char& codeunit_sequence::iterator::operator*() const noexcept
{
	return *this->value;
}

std::ptrdiff_t codeunit_sequence::iterator::operator-(const iterator& rhs) const noexcept
{
	return this->value - rhs.value;
}

codeunit_sequence::iterator& codeunit_sequence::iterator::operator+=(const std::ptrdiff_t diff) noexcept
{
	this->value += diff;
	return *this;
}

codeunit_sequence::iterator& codeunit_sequence::iterator::operator-=(const std::ptrdiff_t diff) noexcept
{
	return this->operator+=(-diff);
}

codeunit_sequence::iterator codeunit_sequence::iterator::operator+(const std::ptrdiff_t diff) const noexcept
{
	iterator tmp = *this;
	tmp += diff;
	return tmp;
}

codeunit_sequence::iterator codeunit_sequence::iterator::operator-(const std::ptrdiff_t diff) const noexcept
{
	iterator tmp = *this;
	tmp -= diff;
	return tmp;
}

codeunit_sequence::iterator& codeunit_sequence::iterator::operator++() noexcept
{
	++this->value;
	return *this;
}

codeunit_sequence::iterator codeunit_sequence::iterator::operator++(int) noexcept
{
	const iterator tmp = *this;
	++*this;
	return tmp;
}

codeunit_sequence::iterator& codeunit_sequence::iterator::operator--() noexcept
{
	--this->value;
	return *this;
}

codeunit_sequence::iterator codeunit_sequence::iterator::operator--(int) noexcept
{
	const iterator tmp = *this;
	--*this;
	return tmp;
}

bool codeunit_sequence::iterator::operator==(const iterator& rhs) const noexcept
{
	return this->value == rhs.value;
}

bool codeunit_sequence::iterator::operator!=(const iterator& rhs) const noexcept
{
	return !(*this == rhs);
}

bool codeunit_sequence::iterator::operator<(const iterator& rhs) const noexcept
{
	return this->value < rhs.value;
}

bool codeunit_sequence::iterator::operator>(const iterator& rhs) const noexcept
{
	return rhs < *this;
}

bool codeunit_sequence::iterator::operator<=(const iterator& rhs) const noexcept
{
	return rhs >= *this;
}

bool codeunit_sequence::iterator::operator>=(const iterator& rhs) const noexcept
{
	return !(*this < rhs);
}

codeunit_sequence::iterator codeunit_sequence::begin() noexcept
{
	return iterator(this->data());
}

codeunit_sequence::const_iterator codeunit_sequence::begin() const noexcept
{
	return this->view().begin();
}

codeunit_sequence::iterator codeunit_sequence::end() noexcept
{
	return this->begin() + this->size();
}

codeunit_sequence::const_iterator codeunit_sequence::end() const noexcept
{
	return this->view().end();
}

codeunit_sequence::const_iterator codeunit_sequence::cbegin() const noexcept
{
	return this->begin();
}

codeunit_sequence::const_iterator codeunit_sequence::cend() const noexcept
{
	return this->end();
}

// code-region-end: iterators

codeunit_sequence::codeunit_sequence() noexcept
	: store_()
{ }

codeunit_sequence::codeunit_sequence(const i32 size) noexcept
	: store_()
{
	if(size > SSO_SIZE_MAX)
	{
		const i32 memory_capacity = details::get_capacity(size + 1);
		char* data = allocator<char>::allocate_array(memory_capacity);
		data[0] = '\0';
		this->as_norm().alloc = true;
		this->as_norm().size = 0;
		this->as_norm().data = data;
		this->as_norm().capacity = memory_capacity - 1;
	}
}

codeunit_sequence::codeunit_sequence(const codeunit_sequence& other) noexcept
	: codeunit_sequence(other.view())
{ }

codeunit_sequence::codeunit_sequence(codeunit_sequence&& other) noexcept
	: store_(other.store_)
{
	other.store_.fill(0);
}

codeunit_sequence& codeunit_sequence::operator=(const codeunit_sequence& other) noexcept
{
	this->operator=(codeunit_sequence(other.view()));
	return *this;
}

codeunit_sequence& codeunit_sequence::operator=(codeunit_sequence&& other) noexcept
{
	this->transfer_data(other);
	return *this;
}

codeunit_sequence& codeunit_sequence::operator=(const codeunit_sequence_view& view) noexcept
{
	codeunit_sequence result(view);
	this->transfer_data(result);
	return *this;
}

codeunit_sequence::~codeunit_sequence() noexcept
{
	this->deallocate();
}

codeunit_sequence::codeunit_sequence(const char* data) noexcept
	: codeunit_sequence(codeunit_sequence_view(data))
{ }

codeunit_sequence::codeunit_sequence(const char* from, const char* last) noexcept
	: codeunit_sequence(codeunit_sequence_view(from, last))
{ }

codeunit_sequence::codeunit_sequence(const char* data, const i32 count) noexcept
	: codeunit_sequence(codeunit_sequence_view(data, count))
{ }

codeunit_sequence::codeunit_sequence(const codeunit_sequence_view sv) noexcept
	: codeunit_sequence(sv.size())
{
	std::copy(sv.c_str(), sv.last(), this->data());
	const i32 size = sv.size();
	this->data()[size] = '\0';
	this->set_size(size);
}

i32 codeunit_sequence::size() const noexcept
{
	if (this->is_short())
		return this->as_sso().size;
	return this->as_norm().size;
}

codeunit_sequence_view codeunit_sequence::view() const& noexcept
{
	return { this->c_str(), this->size() };
}

bool codeunit_sequence::is_empty() const noexcept
{
	return this->size() == 0;
}

bool codeunit_sequence::operator==(codeunit_sequence_view rhs) const noexcept
{
	return this->view() == rhs;
}

bool codeunit_sequence::operator==(const codeunit_sequence& rhs) const noexcept
{
	return this->view() == rhs.view();
}

bool codeunit_sequence::operator==(const char* rhs) const noexcept
{
	return this->view() == codeunit_sequence_view(rhs);
}

bool codeunit_sequence::operator!=(codeunit_sequence_view rhs) const noexcept
{
	return this->view() != rhs;
}

bool codeunit_sequence::operator!=(const codeunit_sequence& rhs) const noexcept
{
	return this->view() != rhs.view();
}

bool codeunit_sequence::operator!=(const char* rhs) const noexcept
{
	return this->view() != codeunit_sequence_view(rhs);
}

codeunit_sequence& codeunit_sequence::append(const codeunit_sequence_view& rhs) noexcept
{
	const i32 answer_size = this->size() + rhs.size();
	this->reserve(answer_size);
	std::copy_n(rhs.c_str(), rhs.size(), this->last());
	this->data()[answer_size] = '\0';
	this->set_size(answer_size);
	return *this;
}

codeunit_sequence& codeunit_sequence::append(const codeunit_sequence& rhs) noexcept
{
	return this->append(rhs.view());
}

codeunit_sequence& codeunit_sequence::append(const codepoint& cp) noexcept
{
	return this->append(codeunit_sequence_view{ cp });
}

codeunit_sequence& codeunit_sequence::append(const char* rhs) noexcept
{
	return this->append(codeunit_sequence_view{ rhs });
}

codeunit_sequence& codeunit_sequence::append(const char codeunit, const i32 count) noexcept
{
	if(count <= 0)
		return *this;
	const i32 old_size = this->size();
	const i32 answer_size = old_size + count;
	this->reserve(answer_size);
	if(codeunit != '\0')
		memset(this->data() + old_size, codeunit, count);
	this->data()[answer_size] = '\0';
	this->set_size(answer_size);
	return *this;
}

codeunit_sequence& codeunit_sequence::operator+=(const codeunit_sequence_view& rhs) noexcept
{
	return this->append(rhs);
}

codeunit_sequence& codeunit_sequence::operator+=(const codeunit_sequence& rhs) noexcept
{
	return this->append(rhs);
}

codeunit_sequence& codeunit_sequence::operator+=(const codepoint& cp) noexcept
{
	return this->append(cp);
}

codeunit_sequence& codeunit_sequence::operator+=(const char* rhs) noexcept
{
	return this->append(rhs);
}

codeunit_sequence& codeunit_sequence::operator+=(const char codeunit) noexcept
{
	return this->append(codeunit);
}

codeunit_sequence_view codeunit_sequence::subview(const index_interval& range) const noexcept
{
	return this->view().subview(range);
}

codeunit_sequence& codeunit_sequence::subsequence(const index_interval& range) noexcept
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
	const i32 from = selection.get_inclusive_min();
	if(from != 0)
	{
		const i32 last = selection.get_exclusive_max();
		std::move(this->data() + from, this->data() + last, this->data());
	}
	const i32 post_size = selection.size();
	this->data()[post_size] = '\0';
	this->set_size(post_size);

	return *this;
}

codeunit_sequence& codeunit_sequence::replace(const codeunit_sequence_view& source, const codeunit_sequence_view& destination, const index_interval& range)
{
	if(source.is_empty())
		return *this;
	const index_interval selection = range.select(this->size());
	const codeunit_sequence_view view = this->subview(selection);
	const i32 count = view.count(source);
	if(count == 0)
		return *this;
	const i32 old_size = this->size();
	const i32 src_size = source.size();
	const i32 dest_size = destination.size();
	const i32 per_delta = dest_size - src_size;
	const i32 whole_delta = per_delta * count;
	const i32 answer_size = old_size + whole_delta;

	if(per_delta == 0)
	{
		index_interval search_range = selection;
		while(true)
		{
			const i32 index = this->index_of(source, search_range);
			if(index == index_invalid)
				break;
			std::copy_n(destination.c_str(), destination.size(), this->data() + index);
			search_range = selection.intersect({ '[', index + dest_size, '~' });
		}
	}
	else if(per_delta < 0)
	{
		index_interval search_range = selection;
		i32 found_index = this->index_of(source, search_range);
		i32 read_index = found_index;
		i32 write_index = found_index;
		while(true)
		{
			std::copy_n(destination.c_str(), dest_size, this->data() + write_index);
			read_index += src_size;
			write_index += dest_size;
			search_range = selection.intersect({ '[', found_index + src_size, '~' });
			found_index = this->index_of(source, search_range);
			if(found_index == index_invalid)
				break;
			const i32 copy_count = found_index - read_index;
			std::copy_n(this->data() + read_index, copy_count, this->data() + write_index);
			read_index = found_index;
			write_index += copy_count;
		}
		std::copy(this->data() + read_index, this->last(), this->data() + write_index);
		this->set_size(answer_size);
	}
	else
	{
		codeunit_sequence result(answer_size);
		const i32 prefix_size = selection.get_inclusive_min();
		std::copy_n(this->data(), prefix_size, result.data());
		
		index_interval search_range = selection;
		i32 found_index = this->index_of(source, search_range);
		std::copy_n(this->data(), found_index, result.data());
		i32 read_index = found_index;
		i32 write_index = found_index;
		while(true)
		{
			std::copy_n(destination.c_str(), dest_size, result.data() + write_index);
			read_index += src_size;
			write_index += dest_size;
			search_range = selection.intersect({ '[', found_index + src_size, '~' });
			found_index = this->index_of(source, search_range);
			if(found_index == index_invalid)
				break;
			const i32 copy_count = found_index - read_index;
			std::copy_n(this->data() + read_index, copy_count, result.data() + write_index);
			read_index = found_index;
			write_index += copy_count;
		}
		std::copy(this->data() + read_index, this->last(), result.data() + write_index);
		result.set_size(answer_size);
		this->transfer_data(result);
	}
	
	return *this;
}

codeunit_sequence& codeunit_sequence::replace(const index_interval& range, const codeunit_sequence_view& destination)
{
	const i32 self_size = this->size();
	const index_interval selection = range.select(self_size);
	if(selection.is_empty())
		// Invalid range
		return *this;
	const i32 delta = destination.size() - selection.size();
	const i32 answer_size = self_size + delta;
	if(delta <= 0)
	{
		// Simply assignment
		for(i32 i = 0; i < destination.size(); ++i)
			this->data()[selection.get_inclusive_min() + i] = destination.c_str()[i];
		if (delta != 0)
		{
			const index_interval source_suffix = index_interval { '[', selection.get_exclusive_max(), '~' }.select(self_size);
			for(const i32 i : source_suffix)
				this->data()[i + delta] = this->data()[i];
			this->data()[answer_size] = '\0';
			this->set_size(answer_size);
		}
	}
	else
	{
		codeunit_sequence result(answer_size);
		const i32 prefix_size = selection.get_inclusive_min();
		const char* start = this->data();
		char* target = result.data();
		std::copy_n(start, prefix_size, target);
		start += prefix_size;
		target += prefix_size;
		const i32 dest_size = destination.size();
		std::copy_n(destination.c_str(), dest_size, target);
		start += selection.size();
		target += dest_size;
		const i32 suffix_size = self_size - selection.get_exclusive_max() + 1;
		std::copy_n(start, suffix_size, target);
		result.set_size(answer_size);
		this->transfer_data(result);
	}

	return *this;
}

codeunit_sequence& codeunit_sequence::self_remove_prefix(const codeunit_sequence_view& prefix) noexcept
{
	return this->starts_with(prefix) ? this->subsequence({ '[', prefix.size(), '~' }) : *this;
}

codeunit_sequence& codeunit_sequence::self_remove_suffix(const codeunit_sequence_view& suffix) noexcept
{
	return this->ends_with(suffix) ? this->subsequence({ '[', 0, -suffix.size(), ')' }) : *this;
}

i32 codeunit_sequence::index_of(const codeunit_sequence_view& pattern, const index_interval& range) const noexcept
{
	return this->view().index_of(pattern, range);
}

i32 codeunit_sequence::last_index_of(const codeunit_sequence_view& pattern, const index_interval& range) const noexcept
{
	return this->view().last_index_of(pattern, range);
}

i32 codeunit_sequence::count(const codeunit_sequence_view& pattern) const noexcept
{
	return this->view().count(pattern);
}

bool codeunit_sequence::starts_with(const codeunit_sequence_view& pattern) const noexcept
{
	return this->view().starts_with(pattern);
}

bool codeunit_sequence::ends_with(const codeunit_sequence_view& pattern) const noexcept
{
	return this->view().ends_with(pattern);
}

void codeunit_sequence::empty()
{
	this->set_size(0);
	this->data()[0] = '\0';
}

void codeunit_sequence::empty(const i32 size)
{
	if(size <= this->get_capacity())
	{
		this->empty();
	}
	else
	{
		this->deallocate();
		const i32 memory_capacity = details::get_capacity(size + 1);
		this->as_norm().alloc = true;
		this->as_norm().size = 0;
		this->as_norm().data = allocator<char>::allocate_array(memory_capacity);
		this->as_norm().data[0] = '\0';
		this->as_norm().capacity = memory_capacity - 1;
	}
}

void codeunit_sequence::reserve(const i32 size)
{
	if(size <= this->get_capacity())
		return;
	codeunit_sequence result(size);
	const i32 self_size = this->size();
	std::copy_n(this->data(), self_size + 1, result.data());
	result.set_size(self_size);
	this->transfer_data(result);
}

codeunit_sequence& codeunit_sequence::write_at(const i32 index, const char codeunit) noexcept
{
	this->data()[index + (index >= 0 ? 0 : this->size())] = codeunit;
	return *this;
}

const char& codeunit_sequence::read_at(const i32 index) const noexcept
{
	return this->view().read_at(index);
}

char& codeunit_sequence::operator[](const i32 index) noexcept
{
	return this->data()[index + (index >= 0 ? 0 : this->size())];
}

const char& codeunit_sequence::operator[](const i32 index) const noexcept
{
	return this->read_at(index);
}

codeunit_sequence& codeunit_sequence::reverse(const index_interval& range) noexcept
{
	const i32 self_size = this->size();
	const index_interval selection = range.select(self_size);
	const i32 size = selection.size();
	for(i32 i = 0; i < size / 2; ++i)
	{
		char& forward = this->data()[selection.get_inclusive_min() + i];
		char& backward = this->data()[selection.get_inclusive_max() - i];
		const char temp = forward;
		forward = backward;
		backward = temp;
	}
	return *this;
}

[[nodiscard]] std::vector<codeunit_sequence_view> codeunit_sequence::split(const codeunit_sequence_view& splitter, bool cull_empty) const noexcept
{
	std::vector<codeunit_sequence_view> pieces;
	this->split(splitter, pieces, cull_empty);
	return pieces;
}

u32 codeunit_sequence::split(const codeunit_sequence_view& splitter, std::vector<codeunit_sequence_view>& pieces, bool cull_empty) const noexcept
{
	codeunit_sequence_view view = this->view();
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

codeunit_sequence_view codeunit_sequence::view_remove_prefix(const codeunit_sequence_view& prefix) const noexcept
{
	return this->view().remove_prefix(prefix);
}

codeunit_sequence_view codeunit_sequence::view_remove_suffix(const codeunit_sequence_view& suffix) const noexcept
{
	return this->view().remove_suffix(suffix);
}

codeunit_sequence& codeunit_sequence::self_trim_start(const codeunit_sequence_view& characters) noexcept
{
	if(this->is_empty())
		return *this;
	const i32 size = this->size();
	for(i32 i = 0; i < size; ++i)
		if(!characters.contains(this->view()[i]))
			return this->subsequence({ '[', i, '~' });
	this->empty();
	return *this;
}

codeunit_sequence& codeunit_sequence::self_trim_end(const codeunit_sequence_view& characters) noexcept
{
	if(this->is_empty())
		return *this;
	const i32 size = this->size();
	for(i32 i = size - 1; i >= 0; --i)
		if(!characters.contains(this->view()[i]))
			return this->subsequence({ '[', 0, i, ']' });
	this->empty();
	return *this;
}

codeunit_sequence& codeunit_sequence::self_trim(const codeunit_sequence_view& characters) noexcept
{
	// trim_start involves memory copy, but trim_end do not
	// so trim_end first may reduce copy
	return this->self_trim_end(characters).self_trim_start(characters);
}

codeunit_sequence_view codeunit_sequence::view_trim_start(const codeunit_sequence_view& characters) const noexcept
{
	return this->view().trim_start(characters);
}

codeunit_sequence_view codeunit_sequence::view_trim_end(const codeunit_sequence_view& characters) const noexcept
{
	return this->view().trim_end(characters);
}

codeunit_sequence_view codeunit_sequence::view_trim(const codeunit_sequence_view& characters) const noexcept
{
	return this->view().trim(characters);
}

u32 codeunit_sequence::get_hash() const noexcept
{
	return this->view().get_hash();
}

const char* codeunit_sequence::c_str() const noexcept
{
	return this->is_short() ? this->as_sso().data.data() : this->as_norm().data;
}

bool codeunit_sequence::is_short_size(const i32 size) noexcept
{
	return size <= SSO_SIZE_MAX;
}

codeunit_sequence::sso& codeunit_sequence::as_sso()
{
	return reinterpret_cast<sso&>(this->store_);
}

const codeunit_sequence::sso& codeunit_sequence::as_sso() const
{
	return reinterpret_cast<const sso&>(this->store_);
}

codeunit_sequence::norm& codeunit_sequence::as_norm()
{
	return reinterpret_cast<norm&>(this->store_);
}

const codeunit_sequence::norm& codeunit_sequence::as_norm() const
{
	return reinterpret_cast<const norm&>(this->store_);
}

bool codeunit_sequence::is_short() const
{
	return !this->as_sso().alloc;
}

i32 codeunit_sequence::get_capacity() const
{
	return this->is_short() ? SSO_SIZE_MAX : this->as_norm().capacity;
}

char* codeunit_sequence::data()
{
	return this->is_short() ? this->as_sso().data.data() : this->as_norm().data;
}

const char* codeunit_sequence::data() const
{
	return this->is_short() ? this->as_sso().data.data() : this->as_norm().data;
}

char* codeunit_sequence::last()
{
	return this->data() + this->size();
}

const char* codeunit_sequence::last() const
{
	return this->data() + this->size();
}

void codeunit_sequence::deallocate()
{
	if(!this->is_short())
		allocator<char>::deallocate_array( this->as_norm().data );
}

void codeunit_sequence::set_size(const i32 size)
{
	if(this->is_short()) this->as_sso().size = static_cast<u8>(size); else this->as_norm().size = size;
}

void codeunit_sequence::transfer_data(codeunit_sequence& other)
{
	std::swap(this->store_, other.store_);
	other.data()[0] = '\0';
	other.set_size(0);
}

bool operator==(const codeunit_sequence_view& lhs, const codeunit_sequence& rhs) noexcept
{
	return rhs == lhs;
}

OPEN_STRING_NS_END
