#pragma once
#include "SkrBase/containers/bit_array/bit_ref.hpp"
#include "SkrBase/algo/bit_algo.hpp"

namespace skr::container
{
template <typename TBlock, size_t N>
constexpr size_t bit_set_block_count()
{
    return N <= 0 ? 1 : ((N - 1) / (8 * sizeof(TBlock)) + 1);
}

template <size_t N, typename TBlock>
struct Bitset {
    using Algo     = algo::BitAlgo<TBlock>;
    using SizeType = size_t;

    static constexpr size_t NumBlock = bit_set_block_count<TBlock, N>();
    static_assert(sizeof(TBlock) == 4 || sizeof(TBlock) == 8, "TBlock size must be 4 or 8");

    // ctor
    Bitset();
    Bitset(uint32_t value);
    Bitset(uint64_t value);

    // copy & move
    Bitset(const Bitset& x);
    Bitset(Bitset&& x);

    // assign & move assign
    Bitset& operator=(const Bitset& x);
    Bitset& operator=(Bitset&& x);

    // bit ops
    Bitset& operator&=(const Bitset& x);
    Bitset& operator|=(const Bitset& x);
    Bitset& operator^=(const Bitset& x);
    Bitset  operator&(const Bitset& x);
    Bitset  operator|(const Bitset& x);
    Bitset  operator^(const Bitset& x);
    Bitset  operator~() const;

    // bit move
    Bitset& operator<<=(SizeType n);
    Bitset& operator>>=(SizeType n);
    Bitset  operator<<(SizeType n) const;
    Bitset  operator>>(SizeType n) const;

    // modify
    void set();
    void set(SizeType i, bool value);
    void reset();
    void reset(SizeType i);
    void flip();
    void flip(SizeType i);

    // visitor
    BitRef<TBlock> operator[](SizeType i);
    bool           operator[](SizeType i) const;

    // compare
    bool operator==(const Bitset& x) const;
    bool operator!=(const Bitset& x) const;

    // summary
    bool     all() const;
    bool     any() const;
    bool     none() const;
    SizeType count() const;

    // data
    TBlock*       data();
    const TBlock* data() const;
    SizeType      size() const;

    // cast
    void     from_uint32(uint32_t value);
    void     from_uint64(uint64_t value);
    uint32_t to_uint32(uint32_t& value) const;
    uint32_t to_uint64(uint64_t& value) const;

    // TODO. find api, if need

private:
    // help function
    void    _clear_unused_bits();
    TBlock& _block_at(SizeType i);
    TBlock  _block_at(SizeType i) const;

private:
    TBlock _data[NumBlock];
};
} // namespace skr::container

// Bitset
namespace skr::container
{
// ctor
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>::Bitset()
{
    if constexpr (NumBlock == 1)
    {
        _data[0] = Algo::EmptyMask;
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] = Algo::EmptyMask;
        _data[1] = Algo::EmptyMask;
    }
    else
    {
        reset();
    }
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>::Bitset(uint32_t value)
{
    from_uint32(value);
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>::Bitset(uint64_t value)
{
    from_uint64(value);
}

// copy & move
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>::Bitset(const Bitset& x) = default;
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>::Bitset(Bitset&& x) = default;

// assign & move assign
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator=(const Bitset& x) = default;
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator=(Bitset&& x) = default;

// bit ops
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator&=(const Bitset& x)
{
    if constexpr (NumBlock == 1)
    {
        _data[0] &= x._data[0];
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] &= x._data[0];
        _data[1] &= x._data[1];
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] &= x._data[i];
        }
    }
    return *this;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator|=(const Bitset& x)
{
    if constexpr (NumBlock == 1)
    {
        _data[0] |= x._data[0];
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] |= x._data[0];
        _data[1] |= x._data[1];
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] |= x._data[i];
        }
    }
    return *this;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator^=(const Bitset& x)
{
    if constexpr (NumBlock == 1)
    {
        _data[0] ^= x._data[0];
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] ^= x._data[0];
        _data[1] ^= x._data[1];
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] ^= x._data[i];
        }
    }
    return *this;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator&(const Bitset& x)
{
    Bitset result(*this);
    result &= x;
    return result;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator|(const Bitset& x)
{
    Bitset result(*this);
    result |= x;
    return result;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator^(const Bitset& x)
{
    Bitset result(*this);
    result ^= x;
    return result;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator~() const
{
    Bitset result(*this);
    result.flip();
    return result;
}

// bit move
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator<<=(SizeType n)
{
    if constexpr (NumBlock == 1)
    {
        _data[0] <<= n;
    }
    else if constexpr (NumBlock == 2)
    {
        if (n)
        {
            // move block
            if (n >= Algo::PerBlockSize)
            {
                _data[1] = _data[0];
                _data[0] = 0;
                n -= Algo::PerBlockSize;
            }

            // move
            _data[1] = (_data[1] << n) | (_data[0] >> (Algo::PerBlockSize - n));
            _data[0] <<= n;
        }
    }
    else
    {
        const SizeType n_block_shift = static_cast<SizeType>(n >> Algo::PerBlockSizeLog2);

        if (n_block_shift)
        {
            for (int i = (int)(NumBlock - 1); i >= 0; --i)
            {
                _data[i] = (((SizeType)i) >= n_block_shift) ? _data[i - n_block_shift] : (TBlock)0;
            }
        }

        n &= Algo::PerBlockSizeMask;

        if (n)
        {
            for (size_t i = (NumBlock - 1); i > 0; --i)
            {
                _data[i] = (TBlock)((_data[i] << n) | (_data[i - 1] >> (Algo::PerBlockSize - n)));
            }
            _data[0] <<= n;
        }
    }

    _clear_unused_bits();

    return *this;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock>& Bitset<N, TBlock>::operator>>=(SizeType n)
{
    if (n < N)
    {
        if constexpr (NumBlock == 1)
        {
            _data[0] >>= n;
        }
        else if constexpr (NumBlock == 2)
        {
            if (n)
            {
                // move block
                if (n >= Algo::PerBlockSize)
                {
                    _data[0] = _data[1];
                    _data[1] = 0;
                    n -= Algo::PerBlockSize;
                }

                // move
                _data[0] = (_data[0] >> n) | (_data[1] << (Algo::PerBlockSize - n));
                _data[1] >>= n;
            }
        }
        else
        {
            const SizeType n_block_shift = static_cast<SizeType>(n >> Algo::PerBlockSizeLog2);

            if (n_block_shift)
            {
                for (SizeType i = 0; i < NumBlock; ++i)
                {
                    _data[i] = ((NumBlock - (SizeType)i) > n_block_shift) ? _data[i + n_block_shift] : (TBlock)0;
                }
            }

            n &= Algo::PerBlockSizeMask;

            if (n)
            {
                for (SizeType i = 0; i < (NumBlock - 1); ++i)
                {
                    _data[i] = (TBlock)((_data[i] >> n) | (_data[i + 1] << (Algo::PerBlockSize - n)));
                }
                _data[NumBlock - 1] >>= n;
            }
        }
    }
    else
    {
        reset();
    }

    return *this;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator<<(SizeType n) const
{
    Bitset result(*this);
    result <<= n;
    return result;
}
template <size_t N, typename TBlock>
inline Bitset<N, TBlock> Bitset<N, TBlock>::operator>>(SizeType n) const
{
    Bitset result(*this);
    result >>= n;
    return result;
}

// modify
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::set()
{
    if constexpr (NumBlock == 1)
    {
        _data[0] = Algo::FullMask;
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] = Algo::FullMask;
        _data[1] = Algo::FullMask;
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] = Algo::FullMask;
        }
    }

    _clear_unused_bits();
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::set(SizeType i, bool value)
{
    SKR_ASSERT(i < N && "Bitset<N, TBlock>::set(SizeType i, bool value) i out of range");
    if constexpr (NumBlock == 1)
    {
        if (value)
        {
            _data[0] |= (static_cast<TBlock>(1) << i);
        }
        else
        {
            _data[0] &= ~(static_cast<TBlock>(1) << i);
        }
    }
    else if constexpr (NumBlock == 2)
    {
        if (value)
        {
            _block_at(i) |= (static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
        }
        else
        {
            _block_at(i) &= ~(static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
        }
    }
    else
    {
        if (value)
        {
            _block_at(i) |= (static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
        }
        else
        {
            _block_at(i) &= ~(static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
        }
    }
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::reset()
{
    if constexpr (NumBlock == 1)
    {
        _data[0] = Algo::EmptyMask;
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] = Algo::EmptyMask;
        _data[1] = Algo::EmptyMask;
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] = Algo::EmptyMask;
        }
    }
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::reset(SizeType i)
{
    SKR_ASSERT(i < N && "Bitset<N, TBlock>::reset(SizeType i) i out of range");
    if constexpr (NumBlock == 1)
    {
        _data[0] &= ~(static_cast<TBlock>(1) << i);
    }
    else if constexpr (NumBlock == 2)
    {
        _block_at(i) &= ~(static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
    }
    else
    {
        _block_at(i) &= ~(static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
    }
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::flip()
{
    // flip words
    if constexpr (NumBlock == 1)
    {
        _data[0] = ~_data[0];
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] = ~_data[0];
        _data[1] = ~_data[1];
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            _data[i] = ~_data[i];
        }
    }

    // clear unused bits
    _clear_unused_bits();
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::flip(SizeType i)
{
    SKR_ASSERT(i < N && "Bitset<N, TBlock>::flip(SizeType i) i out of range");
    _block_at(i) ^= (static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask));
}

// visitor
template <size_t N, typename TBlock>
inline BitRef<TBlock> Bitset<N, TBlock>::operator[](SizeType i)
{
    SKR_ASSERT(i < N && "Bitset<N, TBlock>::operator[](SizeType i) i out of range");
    return BitRef<TBlock>::At(_data, i);
}
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::operator[](SizeType i) const
{
    SKR_ASSERT(i < N && "Bitset<N, TBlock>::operator[](SizeType i) i out of range");
    return (_block_at(i) & (static_cast<TBlock>(1) << (i & Algo::PerBlockSizeMask))) != 0;
}

// compare
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::operator==(const Bitset& x) const
{
    if constexpr (NumBlock == 1)
    {
        return _data[0] == x._data[0];
    }
    else if constexpr (NumBlock == 2)
    {
        return (_data[0] == x._data[0]) && (_data[1] == x._data[1]);
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            if (_data[i] != x._data[i])
            {
                return false;
            }
        }
        return true;
    }
}
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::operator!=(const Bitset& x) const
{
    return !(*this == x);
}

// summary
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::all() const
{
    return count() == size();
}
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::any() const
{
    if constexpr (NumBlock == 1)
    {
        return _data[0] != Algo::EmptyMask;
    }
    else if constexpr (NumBlock == 2)
    {
        return (_data[0] != Algo::EmptyMask) || (_data[1] != Algo::EmptyMask);
    }
    else
    {
        for (size_t i = 0; i < NumBlock; ++i)
        {
            if (_data[i] != Algo::EmptyMask)
            {
                return true;
            }
        }
        return false;
    }
}
template <size_t N, typename TBlock>
inline bool Bitset<N, TBlock>::none() const
{
    return !any();
}
template <size_t N, typename TBlock>
inline typename Bitset<N, TBlock>::SizeType Bitset<N, TBlock>::count() const
{
    if constexpr (NumBlock == 1)
    {
        return pop_count(_data[0]);
    }
    else if constexpr (NumBlock == 2)
    {
        return pop_count(_data[0]) + pop_count(_data[1]);
    }
    else
    {
        SizeType count = 0;
        for (size_t i = 0; i < NumBlock; ++i)
        {
            count += pop_count(_data[i]);
        }
        return count;
    }
}

// data
template <size_t N, typename TBlock>
inline TBlock* Bitset<N, TBlock>::data()
{
    return _data;
}
template <size_t N, typename TBlock>
inline const TBlock* Bitset<N, TBlock>::data() const
{
    return _data;
}
template <size_t N, typename TBlock>
inline typename Bitset<N, TBlock>::SizeType Bitset<N, TBlock>::size() const
{
    return N;
}

// cast
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::from_uint32(uint32_t value)
{
    // assign
    if constexpr (NumBlock == 1)
    {
        _data[0] = static_cast<TBlock>(value);
    }
    else if constexpr (NumBlock == 2)
    {
        _data[0] = static_cast<TBlock>(value);
        _data[1] = Algo::EmptyMask;
    }
    else
    {
        reset();
        _data[0] = static_cast<TBlock>(value);
    }

    _clear_unused_bits();
}
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::from_uint64(uint64_t value)
{
    // assign
    if constexpr (NumBlock == 1)
    {
        _data[0] = static_cast<TBlock>(value);
    }
    else if constexpr (NumBlock == 2)
    {
        if constexpr (sizeof(TBlock) == 4)
        {
            _data[0] = static_cast<TBlock>(value);
            _data[1] = static_cast<TBlock>(value >> 32);
        }
        else
        {
            _data[0] = static_cast<TBlock>(value);
            _data[1] = Algo::EmptyMask;
        }
    }
    else
    {
        reset();
        if constexpr (sizeof(TBlock) == 4)
        {
            _data[0] = static_cast<TBlock>(value);
            _data[1] = static_cast<TBlock>(value >> 32);
        }
        else
        {
            _data[0] = static_cast<TBlock>(value);
        }
    }

    _clear_unused_bits();
}
template <size_t N, typename TBlock>
inline uint32_t Bitset<N, TBlock>::to_uint32(uint32_t& value) const
{
    if constexpr (NumBlock == 1)
    {
        value = static_cast<uint32_t>(_data[0]);
    }
    else if constexpr (NumBlock == 2)
    {
        value = static_cast<uint32_t>(_data[0]);
    }
    else
    {
        value = static_cast<uint32_t>(_data[0]);
    }
}
template <size_t N, typename TBlock>
inline uint32_t Bitset<N, TBlock>::to_uint64(uint64_t& value) const
{
    if constexpr (NumBlock == 1)
    {
        value = static_cast<uint64_t>(_data[0]);
    }
    else if constexpr (NumBlock == 2)
    {
        if constexpr (sizeof(TBlock) == 4)
        {
            value = static_cast<uint64_t>(_data[0]) | (static_cast<uint64_t>(_data[1]) << 32);
        }
        else
        {
            value = static_cast<uint64_t>(_data[0]);
        }
    }
    else
    {
        if constexpr (sizeof(TBlock) == 4)
        {
            value = static_cast<uint64_t>(_data[0]) | (static_cast<uint64_t>(_data[1]) << 32);
        }
        else
        {
            value = static_cast<uint64_t>(_data[0]);
        }
    }
}

// help function
template <size_t N, typename TBlock>
inline void Bitset<N, TBlock>::_clear_unused_bits()
{
    if constexpr (N & Algo::PerBlockSizeMask || N == 0)
    {
        _data[NumBlock - 1] &= ~(Algo::FullMask << (N & Algo::PerBlockSizeMask));
    }
}
template <size_t N, typename TBlock>
inline TBlock& Bitset<N, TBlock>::_block_at(SizeType i)
{
    return _data[i >> Algo::PerBlockSizeLog2];
}
template <size_t N, typename TBlock>
inline TBlock Bitset<N, TBlock>::_block_at(SizeType i) const
{
    return _data[i >> Algo::PerBlockSizeLog2];
}
} // namespace skr::container