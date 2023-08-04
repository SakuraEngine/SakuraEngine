#include <algorithm>
#include "OpenString/codeunit_sequence.h"
#include "OpenString/common/basic_types.h"
#include "OpenString/common/adapters.h"
#include "OpenString/common/functions.h"

namespace ostr
{
	namespace details
	{
		[[nodiscard]] constexpr u64 get_capacity(const u64 v) noexcept
		{
			u8 bit_pos = 0;
			u64 value = v;
			while(value != 0)
			{
				value >>= 1;
				++bit_pos;
			}
			u64 ans = 1ull << bit_pos;
			if(ans == (v << 1))
				ans >>= 1;
			return ans;
		}
	}

	// code-region-start: iterators

	codeunit_sequence::iterator::iterator() noexcept
		: value{ }
	{ }

	codeunit_sequence::iterator::iterator(ochar8_t* v) noexcept
		: value{ v }
	{ }

	ochar8_t* codeunit_sequence::iterator::data() const noexcept
	{
		return this->value;
	}

	ochar8_t& codeunit_sequence::iterator::operator*() const noexcept
	{
		return *this->value;
	}

	i64 codeunit_sequence::iterator::operator-(const iterator& rhs) const noexcept
	{
		return this->value - rhs.value;
	}

	codeunit_sequence::iterator& codeunit_sequence::iterator::operator+=(const i64 diff) noexcept
	{
		this->value += diff;
		return *this;
	}

	codeunit_sequence::iterator& codeunit_sequence::iterator::operator-=(const i64 diff) noexcept
	{
		this->value -= diff;
		return *this;
	}

	codeunit_sequence::iterator& codeunit_sequence::iterator::operator+=(const u64 diff) noexcept
	{
		this->value += diff;
		return *this;
	}

	codeunit_sequence::iterator& codeunit_sequence::iterator::operator-=(const u64 diff) noexcept
	{
		this->value -= diff;
		return *this;
	}

	codeunit_sequence::iterator codeunit_sequence::iterator::operator+(const i64 diff) const noexcept
	{
		iterator tmp = *this;
		tmp += diff;
		return tmp;
	}

	codeunit_sequence::iterator codeunit_sequence::iterator::operator-(const i64 diff) const noexcept
	{
		iterator tmp = *this;
		tmp -= diff;
		return tmp;
	}

	codeunit_sequence::iterator codeunit_sequence::iterator::operator+(const u64 diff) const noexcept
	{
		iterator tmp = *this;
		tmp += diff;
		return tmp;
	}

	codeunit_sequence::iterator codeunit_sequence::iterator::operator-(const u64 diff) const noexcept
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
		return iterator(this->data() + this->size());
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

	codeunit_sequence::codeunit_sequence() noexcept = default;

	codeunit_sequence::codeunit_sequence(const u64 size) noexcept
	{
		if(size > SSO_SIZE_MAX)
		{
			const u64 memory_capacity = details::get_capacity(size + 1);
			ochar8_t* data = allocator<ochar8_t>::allocate_array(memory_capacity);
			data[0] = '\0';
			this->as_norm().alloc = true;
			this->as_norm().size = 0;
			this->as_norm().data = data;
			this->as_norm().capacity = static_cast<u32>(memory_capacity - 1);
		}
	}

	codeunit_sequence::codeunit_sequence(const codeunit_sequence& other) noexcept
		: codeunit_sequence{ other.view() }
	{ }

	codeunit_sequence::codeunit_sequence(codeunit_sequence&& other) noexcept
		: store_{ other.store_ }
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
		codeunit_sequence result{ view };
		this->transfer_data(result);
		return *this;
	}

	codeunit_sequence::~codeunit_sequence() noexcept
	{
		this->deallocate();
	}

	codeunit_sequence::codeunit_sequence(const ochar8_t* data) noexcept
		: codeunit_sequence(codeunit_sequence_view(data))
	{ }

	codeunit_sequence::codeunit_sequence(const ochar8_t* from, const ochar8_t* last) noexcept
		: codeunit_sequence(codeunit_sequence_view(from, last))
	{ }

	codeunit_sequence::codeunit_sequence(const ochar8_t* data, const u64 count) noexcept
		: codeunit_sequence(codeunit_sequence_view(data, count))
	{ }

	codeunit_sequence::codeunit_sequence(const codeunit_sequence_view& sv) noexcept
		: codeunit_sequence(sv.size())
	{
		std::copy(sv.data(), sv.cend().data(), this->data());
		const u64 size = sv.size();
		this->write_at(size,'\0');
		this->set_size(size);
	}

	u64 codeunit_sequence::size() const noexcept
	{
		return this->is_short() ? this->as_sso().size : this->as_norm().size;
	}

	codeunit_sequence_view codeunit_sequence::view() const& noexcept
	{
		return { this->u8_str(), this->size() };
	}

	bool codeunit_sequence::is_empty() const noexcept
	{
		return this->size() == 0;
	}

	bool codeunit_sequence::operator==(const codeunit_sequence_view& rhs) const noexcept
	{
		return this->view() == rhs;
	}

	bool codeunit_sequence::operator==(const codeunit_sequence& rhs) const noexcept
	{
		return this->view() == rhs.view();
	}

	bool codeunit_sequence::operator==(const ochar8_t* rhs) const noexcept
	{
		return this->view() == codeunit_sequence_view(rhs);
	}

	bool codeunit_sequence::operator!=(const codeunit_sequence_view& rhs) const noexcept
	{
		return this->view() != rhs;
	}

	bool codeunit_sequence::operator!=(const codeunit_sequence& rhs) const noexcept
	{
		return this->view() != rhs.view();
	}

	bool codeunit_sequence::operator!=(const ochar8_t* rhs) const noexcept
	{
		return this->view() != codeunit_sequence_view(rhs);
	}

	codeunit_sequence& codeunit_sequence::append(const codeunit_sequence_view& rhs) noexcept
	{
		if(rhs.is_empty())
			return *this;
		const u64 answer_size = this->size() + rhs.size();
		this->reserve(answer_size);
		std::copy_n(rhs.data(), rhs.size(), this->last());
		this->write_at(answer_size,'\0');
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

	codeunit_sequence& codeunit_sequence::append(const ochar8_t* rhs) noexcept
	{
		return this->append(codeunit_sequence_view{ rhs });
	}

	codeunit_sequence& codeunit_sequence::append(const ochar8_t codeunit, const u64 count) noexcept
	{
		if(count <= 0)
			return *this;
		const u64 old_size = this->size();
		const u64 answer_size = old_size + count;
		this->reserve(answer_size);
		if(codeunit != '\0')
			std::fill_n(this->data() + old_size, count, codeunit);
		this->write_at(answer_size, '\0');
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

	codeunit_sequence& codeunit_sequence::operator+=(const ochar8_t* rhs) noexcept
	{
		return this->append(rhs);
	}

	codeunit_sequence& codeunit_sequence::operator+=(const ochar8_t codeunit) noexcept
	{
		return this->append(codeunit);
	}

	codeunit_sequence_view codeunit_sequence::subview(const u64 from, const u64 size) const noexcept
	{
		return this->view().subview(from, size);
	}

	codeunit_sequence& codeunit_sequence::subsequence(const u64 from, const u64 size) noexcept
	{
		const u64 self_size = this->size();
		if(from >= self_size)
		{
			this->empty();
			return *this;
		}
		if(from == 0 && size >= self_size)
			// Do nothing
			return *this;
		const u64 actual_size = minimum({ size, self_size - from });
		if(from != 0)
		{
			const u64 last = from + actual_size;
			std::move(this->data() + from, this->data() + last, this->data());
		}
		this->write_at(actual_size, '\0');
		this->set_size(actual_size);

		return *this;
	}

	codeunit_sequence& codeunit_sequence::replace(const codeunit_sequence_view& destination, const codeunit_sequence_view& source, const u64 from, const u64 size)
	{
		if(source.is_empty())
			return *this;
		const codeunit_sequence_view view = this->subview(from, size);
		const u64 count = view.count(source);
		if(count == 0)
			return *this;
		const u64 old_size = this->size();
		const u64 src_size = source.size();
		const u64 dest_size = destination.size();
		const i64 per_delta = static_cast<i64>(dest_size) - static_cast<i64>(src_size);
		const u64 whole_delta = per_delta * count;
		const u64 answer_size = old_size + whole_delta;

		if(per_delta == 0)
		{
			u64 search_from = from;
			u64 search_size = size;
			while(true)
			{
				const u64 index = this->index_of(source, search_from, search_size);
				if(index == global_constant::INDEX_INVALID)
					break;
				std::copy_n(destination.data(), destination.size(), this->data() + index);
				search_from = index + dest_size;
				search_size = from + size - search_from;
			}
		}
		else if(per_delta < 0)
		{
			u64 search_from = from;
			u64 search_size = size;
			u64 found_index = this->index_of(source, search_from, search_size);
			u64 read_index = found_index;
			u64 write_index = found_index;
			while(true)
			{
				std::copy_n(destination.data(), dest_size, this->data() + write_index);
				read_index += src_size;
				write_index += dest_size;
				search_from = found_index + src_size;
				search_size -= (search_from - from);
				found_index = this->index_of(source, search_from, search_size);
				if(found_index == global_constant::INDEX_INVALID)
					break;
				const u64 copy_count = found_index - read_index;
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
			const u64 prefix_size = from;
			std::copy_n(this->data(), prefix_size, result.data());
		
			u64 search_from = from;
			u64 search_size = size;
			u64 found_index = this->index_of(source, search_from, search_size);
			std::copy_n(this->data(), found_index, result.data());
			u64 read_index = found_index;
			u64 write_index = found_index;
			while(true)
			{
				std::copy_n(destination.data(), dest_size, result.data() + write_index);
				read_index += src_size;
				write_index += dest_size;
				search_from = found_index + src_size;
				search_size -= (search_from - from);
				found_index = this->index_of(source, search_from, search_size);
				if(found_index == global_constant::INDEX_INVALID)
					break;
				const u64 copy_count = found_index - read_index;
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

	codeunit_sequence& codeunit_sequence::replace(const codeunit_sequence_view& destination, const u64 from, const u64 size)
	{
		const u64 self_size = this->size();
		if(from >= self_size || size == 0)
			// invalid range
			return *this;
		const u64 actual_size = minimum(size, self_size - from);
		const i64 delta = static_cast<i64>(destination.size()) - static_cast<i64>(actual_size);
		const u64 answer_size = self_size + delta;
		if(delta <= 0)
		{
			// Simply assignment
			std::copy_n(destination.data(), destination.size(), this->data() + from);
			if (delta != 0)
			{
				ochar8_t* target = this->data() + from + destination.size();
				const ochar8_t* source = target - delta;
				std::copy(source, this->cend().data(), target);
				this->write_at(answer_size, '\0');
				this->set_size(answer_size);
			}
		}
		else
		{
			codeunit_sequence result(answer_size);
			const u64 prefix_size = from;
			const ochar8_t* start = this->data();
			ochar8_t* target = result.data();
			std::copy_n(start, prefix_size, target);
			start += prefix_size;
			target += prefix_size;
			const u64 dest_size = destination.size();
			std::copy_n(destination.data(), dest_size, target);
			start += actual_size;
			target += dest_size;
			const u64 suffix_size = self_size - (from + actual_size);
			std::copy_n(start, suffix_size, target);
			result.set_size(answer_size);
			this->transfer_data(result);
		}

		return *this;
	}

	codeunit_sequence& codeunit_sequence::self_remove_prefix(const codeunit_sequence_view& prefix) noexcept
	{
		return this->starts_with(prefix) ? this->subsequence(prefix.size()) : *this;
	}

	codeunit_sequence& codeunit_sequence::self_remove_suffix(const codeunit_sequence_view& suffix) noexcept
	{
		return this->ends_with(suffix) ? this->subsequence(0, this->size() - suffix.size()) : *this;
	}

	u64 codeunit_sequence::index_of(const codeunit_sequence_view& pattern, const u64 from, const u64 size) const noexcept
	{
		return this->view().index_of(pattern, from, size);
	}

	u64 codeunit_sequence::last_index_of(const codeunit_sequence_view& pattern, const u64 from, const u64 size) const noexcept
	{
		return this->view().last_index_of(pattern, from, size);
	}

	u64 codeunit_sequence::count(const codeunit_sequence_view& pattern) const noexcept
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
		this->write_at(0, '\0');
	}

	void codeunit_sequence::empty(const u64 size)
	{
		if(size <= this->get_capacity())
		{
			this->empty();
		}
		else
		{
			this->deallocate();
			const u64 memory_capacity = details::get_capacity(size + 1);
			this->as_norm().alloc = true;
			this->as_norm().size = 0;
			this->as_norm().data = allocator<ochar8_t>::allocate_array(memory_capacity);
			this->as_norm().data[0] = '\0';
			this->as_norm().capacity = static_cast<u32>( memory_capacity - 1 );
		}
	}

	void codeunit_sequence::reserve(const u64 size)
	{
		if(size <= this->get_capacity())
			return;
		codeunit_sequence result{ size };
		const u64 self_size = this->size();
		std::copy_n(this->data(), self_size + 1, result.data());
		result.set_size(self_size);
		this->transfer_data(result);
	}

	codeunit_sequence& codeunit_sequence::write_at(const u64 index, const ochar8_t codeunit) noexcept
	{
		this->data()[index] = codeunit;
		return *this;
	}

	const ochar8_t& codeunit_sequence::read_at(const u64 index) const noexcept
	{
		return this->view().read_at(index);
	}

	ochar8_t& codeunit_sequence::operator[](const u64 index) noexcept
	{
		return this->data()[index];
	}

	const ochar8_t& codeunit_sequence::operator[](const u64 index) const noexcept
	{
		return this->read_at(index);
	}

	codeunit_sequence& codeunit_sequence::reverse(const u64 from, const u64 size) noexcept
	{
		const u64 self_size = this->size();
		if(from >= self_size || size == 0)
			return *this;
		const u64 actual_size = minimum(size, self_size - from);
		const u64 last = from + actual_size - 1;
		for(u64 i = 0; i < actual_size / 2; ++i)
		{
			const ochar8_t temp = this->read_at(from + i);
			this->write_at(from + i, this->read_at(last - i));
			this->write_at(last - i, temp);
		}
		return *this;
	}

	u32 codeunit_sequence::split(const codeunit_sequence_view& splitter, std::vector<codeunit_sequence_view>& pieces, const bool cull_empty) const noexcept
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
		const u64 size = this->size();
		for(u64 i = 0; i < size; ++i)
			if(!characters.contains(this->read_at(i)))
				return this->subsequence(i);
		this->empty();
		return *this;
	}

	codeunit_sequence& codeunit_sequence::self_trim_end(const codeunit_sequence_view& characters) noexcept
	{
		if(this->is_empty())
			return *this;
		const u64 size = this->size();
		for(u64 i = size; i > 0; --i)
			if(!characters.contains(this->view().read_at(i - 1)))
				return this->subsequence(0, i);
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

	ochar8_t* codeunit_sequence::data() noexcept
	{
		return this->is_short() ? this->as_sso().data.data() : this->as_norm().data;
	}

	const ochar8_t* codeunit_sequence::data() const noexcept
	{
		return this->is_short() ? this->as_sso().data.data() : this->as_norm().data;
	}

	const ochar_t* codeunit_sequence::c_str() const noexcept
	{
		return (const ochar_t*)this->data();
	}

	const ochar8_t* codeunit_sequence::u8_str() const noexcept
	{
		return this->data();
	}

	bool codeunit_sequence::is_short_size(const u64 size) noexcept
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

	u64 codeunit_sequence::get_capacity() const
	{
		return this->is_short() ? SSO_SIZE_MAX : this->as_norm().capacity;
	}

	ochar8_t* codeunit_sequence::last()
	{
		return this->data() + this->size();
	}

	const ochar8_t* codeunit_sequence::last() const
	{
		return this->data() + this->size();
	}

	void codeunit_sequence::deallocate()
	{
		if(!this->is_short())
			allocator<ochar8_t>::deallocate_array( this->as_norm().data );
	}

	void codeunit_sequence::set_size(const u64 size)
	{
		if(this->is_short())
			this->as_sso().size = static_cast<u8>(size);
		else
			this->as_norm().size = static_cast<u32>(size);
	}

	void codeunit_sequence::transfer_data(codeunit_sequence& other)
	{
		bitwise_swap(this->store_, other.store_);
		other.empty();
	}

	bool operator==(const codeunit_sequence_view& lhs, const codeunit_sequence& rhs) noexcept
	{
		return rhs == lhs;
	}
}
