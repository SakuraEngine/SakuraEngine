#pragma once
#include "containers/vector.hpp"
#include <algorithm>
#include "utils/bits.hpp"

namespace hbv
{
	namespace hbv_detail
	{
		using index_t = uint32_t;
		using flag_t = uint64_t;

		//取得最低的一位标志位的位置
		//001000 -> 3
		__forceinline index_t lowbit_pos(flag_t id)
		{
			return skr::CountTrailingZeros64(id);
		}

		//取得最高的一位标志位的位置
		__forceinline index_t highbit_pos(flag_t id)
		{
			return skr::CountLeadingZeros64(id);
		}


		//分层位数组的常数,硬编码,勿动😀

		//每个节点64位,即1<<6
		constexpr index_t BitsPerLayer = 6u;
		constexpr flag_t EmptyNode = 0u;
		constexpr flag_t FullNode = EmptyNode - 1u;
		//硬编码分层为4层,方便优化和编码
		constexpr index_t LayerCount = 4u;

		//节点位置
		template<index_t layer>
		constexpr index_t index_of(index_t id) noexcept
		{
			return id >> ((LayerCount - layer)*BitsPerLayer);
		}

		//节点值
		template<index_t layer>
		constexpr flag_t value_of(index_t id) noexcept
		{
			constexpr index_t mask = (1 << BitsPerLayer) - 1;
			index_t index = index_of<layer + 1>(id);
			return flag_t(1u) << (index & mask);
		}

		/*
		分层位数组(Hierarchical Bit Vector),利用额外的位数组来记录下层位数组的连续空位,整体符合如下规则
		Layer(n-1)[i] = Layer(n)[i] & Layer(n)[i + 1] & ... & Layer(n)[i + 63]
		通过跳过连续的空位来加速稀疏位数组的遍历,在数据紧密但位置分散的时候能取得很好的性能
		*/
		class hbv final
		{
			//硬编码4层
			flag_t& _layer0;
			skr::vector<flag_t>& _layer1;
			skr::vector<flag_t>& _layer2;
			skr::vector<flag_t>& _layer3;
		public:
			hbv(flag_t& layer0, skr::vector<flag_t>& layer1, skr::vector<flag_t>& layer2, skr::vector<flag_t>& layer3) noexcept
                : _layer0(layer0), _layer1(layer1), _layer2(layer2), _layer3(layer3)
            {
            }

			//生长容量,填入更小的容量无效
			void grow_to(index_t to) noexcept
			{
				to = std::min<index_t>(16'777'216u, to);
				if (to < size()) return;
                _layer3.resize(index_of<3>(to) + 1, false);
                _layer2.resize(index_of<2>(to) + 1, 0u);
                _layer1.resize(index_of<1>(to) + 1, 0u);
			}

			index_t size() const noexcept
			{
                if (_layer3.empty()) return 0;
				return _layer3.size() << BitsPerLayer;
			}

			//设置标志位, 性能一般
			void set(index_t id, bool value) noexcept
			{
                grow_to(id);
				index_t index_3 = index_of<3>(id);
				flag_t value_3 = value_of<3>(id);

				if (value)
				{
					//bubble for new node
					bubble_fill(id);
					_layer3[index_3] |= value_3;
				}
				else
				{
					//bubble for empty node
					_layer3[index_3] &= ~value_3;
					bubble_empty(id);
				}
			}

			//判断特定位, 性能一般
			bool test(index_t id) const noexcept
			{
				index_t index_3 = index_of<3>(id);
				if (index_3 > _layer3.size()) return 0;
				return (_layer3[index_3] & value_of<3>(id));
			}

			//清零位数组
			void clear() noexcept
			{
                std::fill(_layer3.begin(), _layer3.end(), 0u);
                std::fill(_layer2.begin(), _layer2.end(), 0u);
                std::fill(_layer1.begin(), _layer1.end(), 0u);
                _layer0 = 0u;
			}

			//直接读指定层标志位
			flag_t layer0() const noexcept
			{
				return _layer0;
			}

			flag_t layer1(index_t id) const noexcept
			{
				if (id >= _layer1.size())
					return 0u;
				return _layer1[id];
			}

			flag_t layer2(index_t id) const noexcept
			{
				if (id >= _layer2.size())
					return 0u;
				return _layer2[id];
			}

			flag_t layer3(index_t id) const noexcept
			{
				if (id >= _layer3.size())
					return 0u;
				return _layer3[id];
			}

			flag_t layer(index_t level, index_t id) const noexcept
			{
				switch (level)
				{
				case 0:
					return layer0();
				case 1:
					return layer1(id);
				case 2:
					return layer2(id);
				case 3:
					return layer3(id);
				default:
					return 0;
				}
			}

		private:
			void bubble_empty(index_t id)
			{
				//简单的尝试上浮空节点,直到遇到非空节点位置
				index_t index_3 = index_of<3>(id);
				if (_layer3[index_3] != EmptyNode) return;
				index_t index_2 = index_of<2>(id);
				_layer2[index_2] &= ~value_of<2>(id);
				if (_layer2[index_2] != EmptyNode) return;
				index_t index_1 = index_of<1>(id);
				_layer1[index_1] &= ~value_of<1>(id);
				if (_layer1[index_1] != EmptyNode) return;
				_layer0 &= ~value_of<0>(id);
			}

			void bubble_fill(index_t id)
			{
				index_t index_3 = index_of<3>(id);
				if (_layer3[index_3] == EmptyNode)
				{
					//直接修改父节点,1 | 1 = 1
					_layer2[index_of<2>(id)] |= value_of<2>(id);
					_layer1[index_of<1>(id)] |= value_of<1>(id);
					_layer0 |= value_of<0>(id);
				}
			}
		};

        struct hbvv
        {
			//硬编码4层
			flag_t _layer0;
			skr::span<flag_t> _layer1;
			skr::span<flag_t> _layer2;
			skr::span<flag_t> _layer3;

            //直接读指定层标志位
			flag_t layer0() const noexcept
			{
				return _layer0;
			}

			flag_t layer1(index_t id) const noexcept
			{
				if (id >= _layer1.size())
					return 0u;
				return _layer1[id];
			}

			flag_t layer2(index_t id) const noexcept
			{
				if (id >= _layer2.size())
					return 0u;
				return _layer2[id];
			}

			flag_t layer3(index_t id) const noexcept
			{
				if (id >= _layer3.size())
					return 0u;
				return _layer3[id];
			}

			flag_t layer(index_t level, index_t id) const noexcept
			{
				switch (level)
				{
				case 0:
					return layer0();
				case 1:
					return layer1(id);
				case 2:
					return layer2(id);
				case 3:
					return layer3(id);
				default:
					return 0;
				}
			}
        };

        //遍历位数组(或组合位数组)
		template<index_t Level = 3, typename T, typename F>
		void for_each(const T& vec, const F& f) noexcept
		{
			eastl::array<flag_t, Level + 1> nodes{};
			eastl::array<index_t, Level + 1> prefix{};
			nodes[0] = vec.layer0();
			index_t level = 0;
			if (nodes[0] == EmptyNode) return;

			while (true)
			{
				//遍历节点
				index_t low = lowbit_pos(nodes[level]);
				nodes[level] &= ~(flag_t(1u) << low);
				index_t id = prefix[level] | low;
				//上层节点,遍历子节点
				if (level < Level)
				{
					++level;
					nodes[level] = vec.layer(level, id);
					prefix[level] = id << BitsPerLayer;
				}
				else
				{
					f(id);
					//子节点遍历完,回到上层节点
					while (nodes[level] == EmptyNode)
					{
						//直到Layer0被遍历完
						if (level == 0)
							return;
						--level;
					}
				}
			}
		}
    }
    using hbv_detail::hbv;
    using hbv_detail::hbvv;
    using hbv_detail::for_each;
}