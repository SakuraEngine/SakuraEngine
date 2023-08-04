
#pragma once
#include <algorithm>
#include <math.h>

namespace ostr
{
	[[nodiscard]] constexpr u64 minimum(const u64 a, const u64 b) noexcept
	{
		return std::min(a, b);
	}
	
	[[nodiscard]] constexpr u64 minimum(const std::initializer_list<u64> v) noexcept
	{
		return std::min(v);
	}
	
	[[nodiscard]] constexpr u64 maximum(const u64 a, const u64 b) noexcept
	{
		return std::max(a, b);
	}
	
	[[nodiscard]] constexpr u64 maximum(const std::initializer_list<u64> v) noexcept
	{
		return std::max(v);
	}

	[[nodiscard]] inline u64 round_ceil(const f64 v) noexcept
	{
		return static_cast<u64>(ceil(v));
	}

	[[nodiscard]] constexpr u64 power(const u64 base, const u64 exponent) noexcept
	{
		u64 result = 1;
		for(u64 i = 0; i < exponent; ++i)
			result *= base;
		return result;
	}

	template<class T>
	constexpr std::enable_if_t<std::is_trivial_v<T>> bitwise_swap(T& a, T& b) noexcept
	{
		T temp = a;
		a = b;
		b = temp;
	}

	template<class T>
	std::enable_if_t<!std::is_trivial_v<T>> bitwise_swap(T& a, T& b) noexcept
	{
		std::array<byte, sizeof(T)> temp;
		std::copy_n(reinterpret_cast<byte*>(&a), sizeof(T), temp.data());
		std::copy_n(reinterpret_cast<byte*>(&b), sizeof(T), reinterpret_cast<byte*>(&a));
		std::copy_n(temp.data(), sizeof(T), reinterpret_cast<byte*>(&b));
	}

	namespace details
	{
		[[nodiscard]] constexpr u64 hash_byte_crc64_implementation(const byte b) noexcept
		{
			u64 result = 0;
			u64 c = b;
			c <<= 56;
			for (u64 j = 0; j < 8; j++)
			{
				if ((result ^ c) & 0x8000000000000000ull)
				{
					constexpr u64 crc64_ecma182_poly = 0x42F0E1EBA9EA3693ull;
					result = (result << 1) ^ crc64_ecma182_poly;
				}
				else
				{
					result <<= 1;
				}
				c <<= 1;
			}
			return result;
		}
	}
	
	[[nodiscard]] constexpr u64 hash_byte_crc64(const byte b) noexcept
	{
		constexpr auto h = details::hash_byte_crc64_implementation;
		constexpr u64 crc64_table[256] = 
		{
			h(0), h(1), h(2), h(3), h(4), h(5), h(6), h(7),
			h(8), h(9), h(10), h(11), h(12), h(13), h(14), h(15),
			h(16), h(17), h(18), h(19), h(20), h(21), h(22), h(23),
			h(24), h(25), h(26), h(27), h(28), h(29), h(30), h(31),
			h(32), h(33), h(34), h(35), h(36), h(37), h(38), h(39),
			h(40), h(41), h(42), h(43), h(44), h(45), h(46), h(47),
			h(48), h(49), h(50), h(51), h(52), h(53), h(54), h(55),
			h(56), h(57), h(58), h(59), h(60), h(61), h(62), h(63),
			h(64), h(65), h(66), h(67), h(68), h(69), h(70), h(71),
			h(72), h(73), h(74), h(75), h(76), h(77), h(78), h(79),
			h(80), h(81), h(82), h(83), h(84), h(85), h(86), h(87),
			h(88), h(89), h(90), h(91), h(92), h(93), h(94), h(95),
			h(96), h(97), h(98), h(99), h(100), h(101), h(102), h(103),
			h(104), h(105), h(106), h(107), h(108), h(109), h(110), h(111),
			h(112), h(113), h(114), h(115), h(116), h(117), h(118), h(119),
			h(120), h(121), h(122), h(123), h(124), h(125), h(126), h(127),
			h(128), h(129), h(130), h(131), h(132), h(133), h(134), h(135),
			h(136), h(137), h(138), h(139), h(140), h(141), h(142), h(143),
			h(144), h(145), h(146), h(147), h(148), h(149), h(150), h(151),
			h(152), h(153), h(154), h(155), h(156), h(157), h(158), h(159),
			h(160), h(161), h(162), h(163), h(164), h(165), h(166), h(167),
			h(168), h(169), h(170), h(171), h(172), h(173), h(174), h(175),
			h(176), h(177), h(178), h(179), h(180), h(181), h(182), h(183),
			h(184), h(185), h(186), h(187), h(188), h(189), h(190), h(191),
			h(192), h(193), h(194), h(195), h(196), h(197), h(198), h(199),
			h(200), h(201), h(202), h(203), h(204), h(205), h(206), h(207),
			h(208), h(209), h(210), h(211), h(212), h(213), h(214), h(215),
			h(216), h(217), h(218), h(219), h(220), h(221), h(222), h(223),
			h(224), h(225), h(226), h(227), h(228), h(229), h(230), h(231),
			h(232), h(233), h(234), h(235), h(236), h(237), h(238), h(239),
			h(240), h(241), h(242), h(243), h(244), h(245), h(246), h(247),
			h(248), h(249), h(250), h(251), h(252), h(253), h(254), h(255)
		};
		return crc64_table[b];
	}

	template<typename T = byte>
	[[nodiscard]] constexpr u64 hash_sequence_crc64(const T* data, const u64 length, const u64 seed = 0) noexcept
	{
		static_assert(sizeof(T) == 1, "CRC64 hash only supports byte-sized types.");
		
		u64 ans = seed;
		for (u64 i = 0; i < length; ++i)
		{
			const T& value = data[i];
			const byte hash_byte = ((ans >> 56) ^ value) & 0xFF;
			const u64 hash_next = hash_byte_crc64(hash_byte);
			ans = hash_next ^ (ans << 8);
		}
		return ans;
	}
}
