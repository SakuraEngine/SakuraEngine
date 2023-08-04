
#pragma once

#include "common/basic_types.h"

namespace ostr
{
	template<class T>
	struct linear_iterator
	{
		constexpr linear_iterator() noexcept = default;
	
		constexpr explicit linear_iterator(T* v) noexcept
			: data_{ v }
		{ }

		[[nodiscard]] constexpr T* data() const noexcept
		{
			return this->data_;
		}

		[[nodiscard]] constexpr T& operator*() const noexcept
		{
			return *this->data_;
		}
	
		constexpr linear_iterator& operator+=(const i64 diff) noexcept
		{
			this->data_ += diff;
			return *this;
		}

		constexpr linear_iterator& operator-=(const i64 diff) noexcept
		{
			this->data_ -= diff;
			return *this;
		}
	
		constexpr linear_iterator& operator+=(const u64 diff) noexcept
		{
			this->data_ += diff;
			return *this;
		}

		constexpr linear_iterator& operator-=(const u64 diff) noexcept
		{
			this->data_ -= diff;
			return *this;
		}

		[[nodiscard]] constexpr linear_iterator operator+(const i64 diff) const noexcept
		{
			linear_iterator tmp = *this;
			tmp += diff;
			return tmp;
		}

		[[nodiscard]] constexpr linear_iterator operator-(const i64 diff) const noexcept
		{
			linear_iterator tmp = *this;
			tmp -= diff;
			return tmp;
		}

		[[nodiscard]] constexpr linear_iterator operator+(const u64 diff) const noexcept
		{
			linear_iterator tmp = *this;
			tmp += diff;
			return tmp;
		}

		[[nodiscard]] constexpr linear_iterator operator-(const u64 diff) const noexcept
		{
			linear_iterator tmp = *this;
			tmp -= diff;
			return tmp;
		}

		constexpr linear_iterator& operator++() noexcept
		{
			++this->data_;
			return *this;
		}

		constexpr linear_iterator operator++(int) noexcept
		{
			const linear_iterator tmp = *this;
			++*this;
			return tmp;
		}

		constexpr linear_iterator& operator--() noexcept
		{
			--this->data_;
			return *this;
		}

		constexpr linear_iterator operator--(int) noexcept 
		{
			const linear_iterator tmp = *this;
			--*this;
			return tmp;
		}
		
	private:
		T* data_;
	};

	template<class T1, class T2>
	[[nodiscard]] constexpr i64 operator-(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return lhs.data() - rhs.data();
	}

	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator==(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return lhs.data() == rhs.data();
	}

	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator!=(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator<(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return lhs.data() < rhs.data();
	}
 
	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator>(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return rhs < lhs;
	}
 
	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator<=(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return rhs >= lhs;
	}
 
	template<class T1, class T2>
	[[nodiscard]] constexpr bool operator>=(const linear_iterator<T1>& lhs, const linear_iterator<T2>& rhs) noexcept
	{
		return !(lhs < rhs);
	}
}
