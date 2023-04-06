// OpenString - index interval for accessing range of sequence container 
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once
#include "definitions.h"
#include "basic_types.h"
#include <limits>

OPEN_STRING_NS_BEGIN

class index_interval
{
public:
	
	struct bound
	{
		enum class inclusion : u8
		{
			inclusive,
			exclusive,
			infinity
		};

		inclusion type;
		i32 value;

		constexpr bound(const inclusion t, const i32 v) noexcept
			: type(t)
			, value(v)
		{ }

		constexpr bound(const ochar8_t t, const i32 v) noexcept
			: type(parse_symbol(t))
			, value(v)
		{ }
		
		static constexpr enum inclusion parse_symbol(const ochar8_t t) noexcept
		{
			if(t == '[' || t == ']')
				return inclusion::inclusive;
			if(t == '(' || t == ')')
				return inclusion::exclusive;
			// usually use '~'
			return inclusion::infinity;
		}

		[[nodiscard]] constexpr bool operator==(const bound& rhs) const noexcept
		{
			if(this->type == inclusion::infinity && rhs.type == inclusion::infinity)
				return true;
			return this->type == rhs.type && this->value == rhs.value;
		}

		[[nodiscard]] constexpr bool operator!=(const bound& rhs) const noexcept
		{
			return !this->operator==(rhs);
		}

		[[nodiscard]] constexpr bool is_infinity() const noexcept
		{
			return this->type == inclusion::infinity;
		}

		[[nodiscard]] constexpr bound universalize(i32 universe_size) const noexcept
		{
			return { this->type, this->value >= 0 ? this->value : universe_size + this->value };
		}
	};
	
	constexpr index_interval(const bound lower_bound, const bound upper_bound) noexcept
		: lower_(lower_bound)
		, upper_(upper_bound)
	{ }
	
	constexpr index_interval(const ochar8_t lower_symbol, const i32 lower_value, const i32 upper_value, const ochar8_t upper_symbol) noexcept
		: lower_(lower_symbol, lower_value)
		, upper_(upper_symbol, upper_value)
	{ }
	
	constexpr index_interval(const ochar8_t lower_symbol, const i32 value, const ochar8_t upper_symbol) noexcept
		: lower_(lower_symbol, value)
		, upper_(upper_symbol, value)
	{ }

	// return (0,0)
	static constexpr index_interval empty() noexcept
	{
		return { '(', 0, ')' };
	}
	// return (-∞,+∞)
	static constexpr index_interval all() noexcept
	{
		return { '~', 0, '~' };
	}
	// return [0,size)
	static constexpr index_interval from_universal(i32 size) noexcept
	{
		return { '[', 0, size, ')' };
	}

// code-region-start: iterators

	struct const_iterator
	{
		constexpr const_iterator() noexcept
			: value()
		{ }
		explicit constexpr const_iterator(i32 v) noexcept
			: value(v)
		{ }

		[[nodiscard]] constexpr i32 operator*() const noexcept
		{
			return this->value;
		}

		constexpr decltype(auto) operator-(const const_iterator& rhs) const noexcept
		{
			return this->value - rhs.value;
		}
		
		constexpr const_iterator& operator+=(const i32 diff) noexcept
		{
			this->value += diff;
			return *this;
		}

		constexpr const_iterator& operator-=(const i32 diff) noexcept
		{
			return this->operator+=(-diff);
		}

		constexpr const_iterator operator+(const i32 diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp += diff;
			return tmp;
		}

		constexpr const_iterator operator-(const i32 diff) const noexcept
		{
			const_iterator tmp = *this;
			tmp -= diff;
			return tmp;
		}

	    constexpr const_iterator& operator++() noexcept
		{
			++this->value;
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
		
		i32 value;
	};

	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator{ this->get_inclusive_min() };
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{ this->get_exclusive_max() };
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

	[[nodiscard]] constexpr i32 operator[](const i32 index) const noexcept
	{
		return this->get_inclusive_min() + index;
	}

	[[nodiscard]] constexpr bool operator==(const index_interval& rhs) const noexcept
	{
		if(this->is_empty() && rhs.is_empty())
			return true;
		return this->lower_ == rhs.lower_ && this->upper_ == rhs.upper_;
	}

	[[nodiscard]] constexpr bool contains(const i32 index) const noexcept
	{
		if(this->lower_.is_infinity()) return true;
		if(this->upper_.is_infinity()) return true;
		if(this->get_inclusive_min() > index) return false;
		if(this->get_inclusive_max() < index) return false;
		return true;
	}

	[[nodiscard]] constexpr bool is_positive() const noexcept
	{
		return this->get_inclusive_min() >= 0 && this->get_inclusive_max() >= 0;
	}

	[[nodiscard]] constexpr bool is_infinity() const noexcept
	{
		// Infinity upper bound is more common.
		return this->upper_.is_infinity() || this->lower_.is_infinity();
	}

	[[nodiscard]] constexpr bool is_finite_positive() const noexcept
	{
		return !this->is_infinity() && this->is_positive();
	}

	[[nodiscard]] constexpr index_interval select(const i32 universe_size) const noexcept
	{
		return from_universal(universe_size).intersect({ this->lower_.universalize(universe_size), this->upper_.universalize(universe_size) });
	}

	[[nodiscard]] constexpr bool is_empty() const noexcept
	{
		if(this->lower_.is_infinity()) return false;
		if(this->upper_.is_infinity()) return false;
		if(this->lower_.value > this->upper_.value) return true;
		if(this->lower_.value < this->upper_.value) return false;
		return this->lower_.type == bound::inclusion::exclusive || this->upper_.type == bound::inclusion::exclusive;
	}

	[[nodiscard]] constexpr index_interval intersect(const index_interval& rhs) const noexcept
	{
		if(this->is_empty() || rhs.is_empty())
			return empty();

		struct bound_intersect
		{
			static constexpr const bound& get_lower(const bound& a, const bound& b)
			{
				if(a.is_infinity()) return b;
				if(b.is_infinity()) return a;
				if(a.value < b.value) return b;
				if(a.value > b.value) return a;
				if(b.type == bound::inclusion::exclusive) return b;
				return a;
			}
			static constexpr const bound& get_upper(const bound& a, const bound& b)
			{
				if(a.is_infinity()) return b;
				if(b.is_infinity()) return a;
				if(a.value < b.value) return a;
				if(a.value > b.value) return b;
				if(b.type == bound::inclusion::exclusive) return b;
				return a;
			}
		};

		return { bound_intersect::get_lower(this->lower_, rhs.lower_), { bound_intersect::get_upper(this->upper_, rhs.upper_) } };
	}

	[[nodiscard]] constexpr i32 get_inclusive_min() const noexcept
	{
		if(this->is_empty())
			return std::numeric_limits<i32>::min();
		if(this->lower_.is_infinity())
			return std::numeric_limits<i32>::min();
		if(this->lower_.type == bound::inclusion::inclusive)
			return this->lower_.value;
		// if(this->lower_.type == bound::type::exclusive)
		return this->lower_.value + 1;
	}

	[[nodiscard]] constexpr i32 get_inclusive_max() const noexcept
	{
		if(this->is_empty())
			return std::numeric_limits<i32>::min();
		if(this->upper_.is_infinity())
			return std::numeric_limits<i32>::max();
		if(this->upper_.type == bound::inclusion::inclusive)
			return this->upper_.value;
		return this->upper_.value - 1;
	}

	[[nodiscard]] constexpr i32 get_exclusive_max() const noexcept
	{
		if(this->is_empty())
			return std::numeric_limits<i32>::min();
		if(this->upper_.is_infinity())
			return std::numeric_limits<i32>::max();
		if(this->upper_.type == bound::inclusion::inclusive)
			return this->upper_.value + 1;
		return this->upper_.value;
	}

	[[nodiscard]] constexpr const bound& get_lower_bound() const noexcept
	{
		return this->lower_;
	}

	[[nodiscard]] constexpr const bound& get_upper_bound() const noexcept
	{
		return this->upper_;
	}

	[[nodiscard]] constexpr i32 size() const noexcept
	{
		return this->get_exclusive_max() - this->get_inclusive_min();
	}

protected:

	bound lower_;
	bound upper_;
	
};

OPEN_STRING_NS_END
