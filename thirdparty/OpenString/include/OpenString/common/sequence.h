
#pragma once
#include <array>
#include "common/adapters.h"
#include "common/basic_types.h"
#include "common/assertion.h"
#include "common/functions.h"
#include "common/linear_iterator.h"

namespace ostr
{
	template<class T, u64 I = 0>
	struct OPEN_STRING_API sequence
	{
	public:
		sequence() noexcept;

		template<class T1, u64 I1>
		explicit sequence(const std::array<T1, I1>& array) noexcept;

		template<class T1, u64 I1>
		explicit sequence(std::array<T1, I1>&& array) noexcept;

		sequence(std::initializer_list<T> list) noexcept;

		// Copy constructor is not allowed because element inside maybe not copyable.
		// TODO: Maybe we can use std::is_copy_constructible_v<T> to check.
		sequence(const sequence& other) noexcept = delete;
		sequence(sequence&& other) noexcept;
		
		sequence& operator=(const sequence& other) noexcept = delete;

		// The purpose of this function is not clear.
		// Do you want to move the memory ownership from other to this?
		// Or do you want to just copy the data from other to this?
		// TODO: Two functions for two purposed.
		sequence& operator=(sequence&& other) noexcept = delete;

		~sequence() noexcept;

		using const_iterator = linear_iterator<const T>;
		using iterator = linear_iterator<T>;

		[[nodiscard]] const_iterator cbegin() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;
		[[nodiscard]] iterator begin() noexcept;
		[[nodiscard]] iterator end() noexcept;
		[[nodiscard]] const_iterator begin() const noexcept;
		[[nodiscard]] const_iterator end() const noexcept;

		[[nodiscard]] bool is_empty() const noexcept;

		[[nodiscard]] bool operator==(const sequence& other) const noexcept;

		[[nodiscard]] u64 size() const noexcept;
		[[nodiscard]] u64 capacity() const noexcept;

		[[nodiscard]] T* data() noexcept;
		[[nodiscard]] const T* data() const noexcept;

		void empty() noexcept;
		void empty(u64 size) noexcept;

		[[nodiscard]] const T& read_at(const u64 index) const noexcept;
		[[nodiscard]] const T& read_from_last(const u64 index = 0) const noexcept;
		[[nodiscard]] T& access_at(const u64 index) noexcept;
		[[nodiscard]] T& access_from_last(const u64 index = 0) noexcept;

		void reserve(const u64 size) noexcept;

		void push_back_uninitialized(const u64 size = 1) noexcept;

		void resize_uninitialized(const u64 size) noexcept;

		template<class...Args>
		void resize(const u64 size, Args&&...args) noexcept;

		void push_back(T element) noexcept;

		template<class...Args>
		void emplace_back(Args&&...args) noexcept;

		template<class T1 = T>
		T1 pop_back() noexcept;

		void append(const T* const source, const u64 size) noexcept;

	private:

		[[nodiscard]] bool is_short() const noexcept;

		[[nodiscard]] T* data_at(u64 index) noexcept;
		[[nodiscard]] const T* data_at(u64 index) const noexcept;

		static constexpr u64 POINTER_SIZE = sizeof(void*);
		static constexpr u64 ELEMENT_SIZE = sizeof(T);
		static constexpr u64 ELEMENT_SHORT_CAPACITY = maximum({ POINTER_SIZE / ELEMENT_SIZE, I });
		static constexpr u64 STORAGE_SHORT_CAPACITY = maximum({ ELEMENT_SHORT_CAPACITY, 1 }) * ELEMENT_SIZE;

		struct short_sequence_storage
		{
			std::array<T, ELEMENT_SHORT_CAPACITY> stack_storage;
		};

		struct large_sequence_storage
		{
			T* heap_storage;
		};

		[[nodiscard]] short_sequence_storage& as_small() noexcept
		{
			return reinterpret_cast<short_sequence_storage&>(this->storage_);
		}

		[[nodiscard]] const short_sequence_storage& as_small() const noexcept
		{
			return reinterpret_cast<const short_sequence_storage&>(this->storage_);
		}

		[[nodiscard]] const large_sequence_storage& as_large() const noexcept
		{
			return reinterpret_cast<const large_sequence_storage&>(this->storage_);
		}

		[[nodiscard]] large_sequence_storage& as_large() noexcept
		{
			return reinterpret_cast<large_sequence_storage&>(this->storage_);
		}

		struct storage
		{
			std::array<byte, STORAGE_SHORT_CAPACITY> data;

			[[nodiscard]] byte* raw_data() noexcept
			{
				return *reinterpret_cast<byte**>(this->data.data());
			}
		};

	private:
		u64 capacity_{ ELEMENT_SHORT_CAPACITY };
		u64 size_{ 0 };
		storage storage_{ };
	};

	template <class T, u64 I>
	sequence<T, I>::sequence() noexcept = default;

	template <class T, u64 I>
	template <class T1, u64 I1>
	sequence<T, I>::sequence(const std::array<T1, I1>& array) noexcept
	{
		this->reserve(array.size());
		std::copy_n(array.data(), array.size(), this->data());
		this->size_ = array.size();
	}

	template <class T, u64 I>
	template <class T1, u64 I1>
	sequence<T, I>::sequence(std::array<T1, I1>&& array) noexcept
	{
		this->reserve(array.size());
		std::move(array.data(), array.end(), this->data());
		this->size_ = array.size();
	}

	template <class T, u64 I>
	sequence<T, I>::sequence(std::initializer_list<T> list) noexcept
	{
		this->reserve(list.size());
		std::copy_n(list.begin(), list.size(), this->data());
		this->size_ = list.size();
	}

	template <class T, u64 I>
	sequence<T, I>::sequence(sequence&& other) noexcept
		: capacity_{ other.capacity_ }
		, size_{ other.size_ }
		, storage_{ std::move(other.storage_) }
	{
		other.capacity_ = ELEMENT_SHORT_CAPACITY;
		other.size_ = 0;
	}

	template <class T, u64 I>
	sequence<T, I>::~sequence() noexcept
	{
		for(u64 i = 0; i < this->size(); ++i)
			std::destroy_at(this->data_at(i));
		this->size_ = 0;
		if(!this->is_short())
		{
			const byte* data = reinterpret_cast<byte*>(this->as_large().heap_storage);
			allocator<byte>::deallocate_array(data);
		}
	}

	template <class T, u64 I>
	typename sequence<T, I>::const_iterator sequence<T, I>::cbegin() const noexcept
	{
		return const_iterator{ this->data() };
	}

	template <class T, u64 I>
	typename sequence<T, I>::const_iterator sequence<T, I>::cend() const noexcept
	{
		return const_iterator{ this->data_at(this->size_) };
	}

	template <class T, u64 I>
	typename sequence<T, I>::iterator sequence<T, I>::begin() noexcept
	{
		return iterator{ this->data() };
	}

	template <class T, u64 I>
	typename sequence<T, I>::iterator sequence<T, I>::end() noexcept
	{
		return iterator{ this->data_at(this->size_) };
	}

	template <class T, u64 I>
	typename sequence<T, I>::const_iterator sequence<T, I>::begin() const noexcept
	{
		return this->cbegin();
	}

	template <class T, u64 I>
	typename sequence<T, I>::const_iterator sequence<T, I>::end() const noexcept
	{
		return this->cend();
	}

	template <class T, u64 I>
	bool sequence<T, I>::is_empty() const noexcept
	{
		return this->size_ == 0;
	}

	template <class T, u64 I>
	bool sequence<T, I>::operator==(const sequence& other) const noexcept
	{
		if(this->size() != other.size())
			return false;
		for(u64 i = 0; i < this->size(); ++i)
			if(this->data_at(i) != other.data_at(i))
				return false;
		return true;
	}

	template <class T, u64 I>
	u64 sequence<T, I>::size() const noexcept
	{
		return this->size_;
	}

	template <class T, u64 I>
	u64 sequence<T, I>::capacity() const noexcept
	{
		return this->capacity_;
	}

	template <class T, u64 I>
	T* sequence<T, I>::data() noexcept
	{
		return this->is_short() ? this->as_small().stack_storage.data() : this->as_large().heap_storage;
	}

	template <class T, u64 I>
	const T* sequence<T, I>::data() const noexcept
	{
		return this->is_short() ? this->as_small().stack_storage.data() : this->as_large().heap_storage;
	}

	template <class T, u64 I>
	void sequence<T, I>::empty() noexcept
	{
		for(T& e : *this)
			std::destroy_at(&e);
		this->size_ = 0;
	}

	template <class T, u64 I>
	void sequence<T, I>::empty(u64 size) noexcept
	{
		this->empty();
		if(size >= this->capacity_)
		{
			T* allocated = allocator<T>::allocate_array(size);
			if(!this->is_short())
				allocator<T>::deallocate_array(this->data());
			this->as_large().heap_storage = allocated;
			this->capacity_ = size;
		}
	}

	template <class T, u64 I>
	const T& sequence<T, I>::read_at(const u64 index) const noexcept
	{
		OPEN_STRING_CHECK(index < this->size_, "index out of range");
		return *(this->data_at(index));
	}

	template <class T, u64 I>
	const T& sequence<T, I>::read_from_last(const u64 index) const noexcept
	{
		OPEN_STRING_CHECK(index < this->size_, "index out of range");
		return this->read_at(this->size_ - index - 1);
	}

	template <class T, u64 I>
	T& sequence<T, I>::access_at(const u64 index) noexcept
	{
		OPEN_STRING_CHECK(index < this->size_, "index out of range");
		return *(this->data_at(index));
	}

	template <class T, u64 I>
	T& sequence<T, I>::access_from_last(const u64 index) noexcept
	{
		OPEN_STRING_CHECK(index < this->size_, "index out of range");
		return this->access_at(this->size_ - index - 1);
	}

	template <class T, u64 I>
	void sequence<T, I>::reserve(const u64 size) noexcept
	{
		if(size <= this->capacity_)
			return;
		byte* allocated_byte = allocator<byte>::allocate_array(size * ELEMENT_SIZE);
		T* old_data = this->data();
		const auto source = reinterpret_cast<byte*>(old_data);
		if(!this->is_empty())
		{
			const auto source_end = reinterpret_cast<byte*>(old_data + this->size_);
			std::move(source, source_end, allocated_byte);
		}
		if(!this->is_short())
			allocator<byte>::deallocate_array(source);
		this->as_large().heap_storage = reinterpret_cast<T*>(allocated_byte);
		this->capacity_ = size;
	}

	template <class T, u64 I>
	void sequence<T, I>::push_back_uninitialized(const u64 size) noexcept
	{
		const u64 size_required = this->size_ + size;
		if(size_required > this->capacity_)
		{
			// Golden ratio is a good growth factor.
			// reference: https://stackoverflow.com/a/5232342
			// " I have an intuition (that is currently not backed up by any hard data)
			// that a growth rate in line with the golden ratio
			// (because of its relationship to the fibonacci sequence)
			// will prove to be the most efficient growth rate for real-world loads
			// in terms of minimizing both extra space used and time. "
			u64 grown_capacity = round_ceil(static_cast<f64>(maximum({ this->capacity_, 1 })) * global_constant::GOLDEN_RATIO);
			while(grown_capacity < size_required)
				grown_capacity = round_ceil(static_cast<f64>(grown_capacity) * global_constant::GOLDEN_RATIO);
			this->reserve(grown_capacity);
		}
		this->size_ = size_required;
	}

	template <class T, u64 I>
	void sequence<T, I>::resize_uninitialized(const u64 size) noexcept
	{
		if(const u64 current_size = this->size(); size > current_size)
		{
			this->push_back_uninitialized(size - current_size);
		}
		else
		{
			for(u64 i = size; i < current_size; ++i)
				std::destroy_at(this->data_at(i));
			this->size_ = size;
		}
	}

	template <class T, u64 I>
	template <class ... Args>
	void sequence<T, I>::resize(const u64 size, Args&&... args) noexcept
	{
		const u64 origin_size = this->size();
		this->resize_uninitialized(size);
		for(u64 i = origin_size; i < size; ++i)
			allocator<T>::placement_construct(this->data_at(i), std::forward<Args>(args)...);
	}

	template <class T, u64 I>
	void sequence<T, I>::push_back(T element) noexcept
	{
		this->emplace_back(std::move(element));
	}

	template <class T, u64 I>
	template <class ... Args>
	void sequence<T, I>::emplace_back(Args&&... args) noexcept
	{
		this->push_back_uninitialized();
		T* const emplace_target = this->data_at(this->size_ - 1);
		allocator<T>::placement_construct(emplace_target, std::forward<Args>(args)...);
	}

	template <class T, u64 I>
	template <class T1>
	T1 sequence<T, I>::pop_back() noexcept
	{
		OPEN_STRING_CHECK(this->size_ != 0, "Pop failed! Sequence is empty!");
		T result = std::move(this->access_at(this->size_ - 1));
		std::destroy_at(this->data_at(this->size_ - 1));
		--this->size_;
		return result;
	}

	template <class T, u64 I>
	void sequence<T, I>::append(const T* const source, const u64 size) noexcept
	{
		this->push_back_uninitialized(size);
		std::copy_n(source, size, this->data_at(this->size_ - size));
	}

	template <class T, u64 I>
	bool sequence<T, I>::is_short() const noexcept
	{
		return this->capacity_ == ELEMENT_SHORT_CAPACITY;
	}

	template <class T, u64 I>
	T* sequence<T, I>::data_at(u64 index) noexcept
	{
		return this->data() + index;
	}

	template <class T, u64 I>
	const T* sequence<T, I>::data_at(u64 index) const noexcept
	{
		return this->data() + index;
	}
}
