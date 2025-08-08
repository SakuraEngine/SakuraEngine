#include "SkrBase/containers/bit_tools/bit_iterator.hpp"
#include "SkrTestFramework/framework.hpp"

#include "SkrBase/algo/bit_algo.hpp"

template <typename TBlock, typename TSize>
void testBitAlgo()
{
    // dummy
    // using TBlock = uint64_t;
    // using TSize  = size_t;

    using namespace skr;
    using BitAlgo = algo::BitAlgo<TBlock>;

    SUBCASE("num blocks")
    {
        REQUIRE_EQ(BitAlgo::num_blocks(0), 0);
        REQUIRE_EQ(BitAlgo::num_blocks(BitAlgo::PerBlockSize), 1);
        REQUIRE_EQ(BitAlgo::num_blocks(BitAlgo::PerBlockSize + 1), 2);
        REQUIRE_EQ(BitAlgo::num_blocks(BitAlgo::PerBlockSize - 1), 1);
    }

    SUBCASE("block mask")
    {
        REQUIRE_EQ(BitAlgo::first_block_mask(TSize(3)), static_cast<TBlock>(BitAlgo::FullMask << 3));
        REQUIRE_EQ(BitAlgo::first_block_mask(BitAlgo::PerBlockSize + 3), static_cast<TBlock>(BitAlgo::FullMask << 3));
        REQUIRE_EQ(BitAlgo::first_block_mask(TSize(0)), BitAlgo::FullMask);
        REQUIRE_EQ(BitAlgo::first_block_mask(BitAlgo::PerBlockSize), BitAlgo::FullMask);
        REQUIRE_EQ(BitAlgo::last_block_mask(TSize(3)), static_cast<TBlock>(BitAlgo::FullMask >> (BitAlgo::PerBlockSize - 3)));
        REQUIRE_EQ(BitAlgo::last_block_mask(BitAlgo::PerBlockSize + 3), static_cast<TBlock>(BitAlgo::FullMask >> (BitAlgo::PerBlockSize - 3)));
        REQUIRE_EQ(BitAlgo::last_block_mask(TSize(0)), BitAlgo::FullMask);
        REQUIRE_EQ(BitAlgo::last_block_mask(BitAlgo::PerBlockSize), BitAlgo::FullMask);
    }

    constexpr TSize DATA_SIZE = 1024;
    constexpr TSize BIT_SIZE = 1024 * BitAlgo::PerBlockSize;
    TBlock data[DATA_SIZE] = { 0 };

    SUBCASE("set block")
    {
        for (TSize i = 0; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
        }
        BitAlgo::set_block_masked(data[0], TBlock(0b1111), true);
        REQUIRE_EQ(data[0], TBlock(0b1111));
        for (TSize i = 1; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
        }
        BitAlgo::set_block_masked(data[0], TBlock(0b0011), false);
        REQUIRE_EQ(data[0], TBlock(0b1100));
        for (TSize i = 1; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
        }
        BitAlgo::set_blocks(data, TSize(1), DATA_SIZE / 2 - 1, true);
        REQUIRE_EQ(data[0], TBlock(0b1100));
        for (TSize i = 1; i < DATA_SIZE / 2; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::FullMask);
        }
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, false);
        for (TSize i = 0; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
        }
    }

    SUBCASE("set & get bits")
    {
        BitAlgo::set_range(data, TSize(4), BIT_SIZE - 4, true);
        REQUIRE_EQ(data[0], static_cast<TBlock>(BitAlgo::FullMask << 4));
        for (TSize i = 1; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::FullMask);
        }
        for (TSize i = 0; i < 4; ++i)
        {
            REQUIRE_EQ(BitAlgo::get(data, i), false);
        }
        for (TSize i = 4; i < BIT_SIZE; ++i)
        {
            REQUIRE_EQ(BitAlgo::get(data, i), true);
        }
        for (TSize i = 4; i < BIT_SIZE; ++i)
        {
            BitAlgo::set(data, i, false);
        }
        for (TSize i = 0; i < DATA_SIZE; ++i)
        {
            REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
        }
    }

    SUBCASE("find bit in block")
    {
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::FullMask, BitAlgo::FullMask, true), 0);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::EmptyMask, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::EmptyMask, BitAlgo::FullMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::FullMask, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(TBlock(0xF), BitAlgo::FullMask, true), 0);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::FullMask, BitAlgo::FullMask, true), BitAlgo::PerBlockSize - 1);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::EmptyMask, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::EmptyMask, BitAlgo::FullMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::FullMask, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(TBlock(1), BitAlgo::FullMask, true), 0);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::EmptyMask, BitAlgo::FullMask, false), 0);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::EmptyMask, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::FullMask, BitAlgo::FullMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(BitAlgo::FullMask, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_block_masked(TBlock(0xF), BitAlgo::FullMask, false), 4);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::EmptyMask, BitAlgo::FullMask, false), BitAlgo::PerBlockSize - 1);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::EmptyMask, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::FullMask, BitAlgo::FullMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(BitAlgo::FullMask, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_last_block_masked(~TBlock(1), BitAlgo::FullMask, false), 0);
    }

    SUBCASE("find bit")
    {
        BitAlgo::set(data, 1145, true);
        BitAlgo::set(data, 2145, true);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), BIT_SIZE, true), 1145);
        REQUIRE_EQ(BitAlgo::find(data, TSize(2000), BIT_SIZE - 2000, true), 2145);
        REQUIRE_EQ(BitAlgo::find(data, TSize(4000), BIT_SIZE - 4000, true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), TSize(1000), true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(4000), TSize(0), true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), TSize(0), true), npos_of<TSize>);
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, true);
        BitAlgo::set(data, 1145, false);
        BitAlgo::set(data, 2145, false);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), BIT_SIZE, false), 1145);
        REQUIRE_EQ(BitAlgo::find(data, TSize(2000), BIT_SIZE - 2000, false), 2145);
        REQUIRE_EQ(BitAlgo::find(data, TSize(4000), BIT_SIZE - 4000, false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), TSize(1000), false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(4000), TSize(0), false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find(data, TSize(0), TSize(0), false), npos_of<TSize>);
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, false);
    }

    SUBCASE("find last bit")
    {
        BitAlgo::set(data, 1145, true);
        BitAlgo::set(data, 2145, true);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), BIT_SIZE, true), 2145);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(2000), true), 1145);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(4000), BIT_SIZE - 4000, true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(1000), true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(4000), TSize(0), true), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(0), true), npos_of<TSize>);
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, true);
        BitAlgo::set(data, 1145, false);
        BitAlgo::set(data, 2145, false);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), BIT_SIZE, false), 2145);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(2000), false), 1145);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(4000), BIT_SIZE - 4000, false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(1000), false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(4000), TSize(0), false), npos_of<TSize>);
        REQUIRE_EQ(BitAlgo::find_last(data, TSize(0), TSize(0), false), npos_of<TSize>);
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, false);
    }

    SUBCASE("find then flip bit in block")
    {
        TBlock block;
        block = TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(0b0101));
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, BitAlgo::FullMask, true), 0);
        REQUIRE_EQ(block, TBlock(0b0100));
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, BitAlgo::FullMask, true), 2);
        REQUIRE_EQ(block, TBlock(0b0000));
        block = TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, TBlock(0b0101), true), 0);
        REQUIRE_EQ(block, TBlock(0b0100));
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, TBlock(0b0101), true), 2);
        REQUIRE_EQ(block, TBlock(0b0000));
        block = TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, TBlock(0b1010), true), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(0b0101));
        REQUIRE_EQ(BitAlgo::find_flip_block_masked(block, TBlock(0b1010), true), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(0b0101));

        block = ~TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, BitAlgo::EmptyMask, true), npos_of<TBlock>);
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, BitAlgo::EmptyMask, false), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0101)));
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, BitAlgo::FullMask, false), 2);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0001)));
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, BitAlgo::FullMask, false), 0);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0000)));
        block = ~TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, TBlock(0b0101), false), 2);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0001)));
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, TBlock(0b0101), false), 0);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0000)));
        block = ~TBlock(0b0101);
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, TBlock(0b1010), false), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0101)));
        REQUIRE_EQ(BitAlgo::find_flip_last_block_masked(block, TBlock(0b1010), false), npos_of<TBlock>);
        REQUIRE_EQ(block, TBlock(~TBlock(0b0101)));
    }

    SUBCASE("find then flip bit")
    {
        for (TSize i = 0; i < BIT_SIZE; ++i)
        {
            if (i % 3 == 0)
            {
                BitAlgo::set(data, i, true);
            }
        }
        {
            TSize pos = BitAlgo::find_flip(data, TSize(0), BIT_SIZE, true);
            TSize idx = 0;
            while (pos != npos_of<TSize>)
            {
                REQUIRE_EQ(pos % 3, 0);
                REQUIRE_EQ(pos, idx * 3);
                ++idx;
                pos = BitAlgo::find_flip(data, TSize(0), BIT_SIZE, true);
            }
            for (TSize i = 0; i < DATA_SIZE; ++i)
            {
                REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
            }
        }
        for (TSize i = 0; i < BIT_SIZE; ++i)
        {
            if (i % 3 != 0)
            {
                BitAlgo::set(data, i, true);
            }
        }
        {
            TSize pos = BitAlgo::find_flip(data, TSize(0), BIT_SIZE, false);
            TSize idx = 0;
            while (pos != npos_of<TSize>)
            {
                REQUIRE_EQ(pos % 3, 0);
                REQUIRE_EQ(pos, idx * 3);
                ++idx;
                pos = BitAlgo::find_flip(data, TSize(0), BIT_SIZE, false);
            }
            for (TSize i = 0; i < DATA_SIZE; ++i)
            {
                REQUIRE_EQ(data[i], BitAlgo::FullMask);
            }
        }
    }

    SUBCASE("find last then flip")
    {
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, false);
        for (TSize i = 0; i < BIT_SIZE; ++i)
        {
            if (i % 3 == 0)
            {
                BitAlgo::set(data, i, true);
            }
        }
        {
            TSize pos = BitAlgo::find_last_flip(data, TSize(0), BIT_SIZE, true);
            TSize idx = 0;
            while (pos != npos_of<TSize>)
            {
                REQUIRE_EQ(pos % 3, 0);
                REQUIRE_EQ(pos, BIT_SIZE - 1 - (BIT_SIZE - 1) % 3 - idx * 3);
                ++idx;
                pos = BitAlgo::find_last_flip(data, TSize(0), BIT_SIZE, true);
            }
            for (TSize i = 0; i < DATA_SIZE; ++i)
            {
                REQUIRE_EQ(data[i], BitAlgo::EmptyMask);
            }
        }
        for (TSize i = 0; i < BIT_SIZE; ++i)
        {
            if (i % 3 != 0)
            {
                BitAlgo::set(data, i, true);
            }
        }
        {
            TSize pos = BitAlgo::find_last_flip(data, TSize(0), BIT_SIZE, false);
            TSize idx = 0;
            while (pos != npos_of<TSize>)
            {
                REQUIRE_EQ(pos % 3, 0);
                REQUIRE_EQ(pos, BIT_SIZE - 1 - (BIT_SIZE - 1) % 3 - idx * 3);
                ++idx;
                pos = BitAlgo::find_last_flip(data, TSize(0), BIT_SIZE, false);
            }
            for (TSize i = 0; i < DATA_SIZE; ++i)
            {
                REQUIRE_EQ(data[i], BitAlgo::FullMask);
            }
        }
    }

    SUBCASE("BitAlgo::find overflow")
    {
        BitAlgo::set_blocks(data, TSize(0), DATA_SIZE, false);
        BitAlgo::set(data, BIT_SIZE - 1, true);
        auto found_idx = BitAlgo::find(data, TSize(0), BIT_SIZE - 1, true);
        REQUIRE_EQ(found_idx, npos_of<TSize>);
        found_idx = BitAlgo::find_last(data, TSize(0), BIT_SIZE - 1, true);
        REQUIRE_EQ(found_idx, npos_of<TSize>);
    }
}

TEST_CASE("test bit algo")
{
    SUBCASE("Block：uint8_t")
    {
        testBitAlgo<uint8_t, size_t>();
    }
    SUBCASE("Block：uint16_t")
    {
        testBitAlgo<uint16_t, size_t>();
    }
    SUBCASE("Block：uint32_t")
    {
        testBitAlgo<uint32_t, size_t>();
    }
    SUBCASE("Block：uint64_t")
    {
        testBitAlgo<uint64_t, size_t>();
    }
}