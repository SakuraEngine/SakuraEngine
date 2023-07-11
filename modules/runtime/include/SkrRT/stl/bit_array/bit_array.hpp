#pragma once
#include "SkrRT/base/config.hpp"
#include "SkrRT/algo/bit_algo.hpp"
#include "SkrRT/stl/allocator/allocator.hpp"
#include "bit_ref.hpp"
#include "bit_iterator.hpp"

// BitArray def
// TODO. 使用 BitArrayDataDef 来替代 SizeType 的返回结构，隐藏 npos 细节，由于 BitRef 的特殊性，这里可能需要自存一个指针与一个 mask
namespace skr
{
template <typename TBlock, typename Alloc>
class BitArray final
{
public:
    using SizeType = typename Alloc::SizeType;
    using Algo     = algo::BitAlgo<TBlock>;
    using It       = BitIt<TBlock, SizeType, false>;
    using CIt      = BitIt<TBlock, SizeType, true>;
    using TIt      = TrueBitIt<TBlock, SizeType, true>;

    // ctor & dtor
    BitArray(Alloc alloc = Alloc());
    BitArray(SizeType size, bool v, Alloc alloc = Alloc());
    ~BitArray();

    // copy & move ctor
    BitArray(const BitArray& other, Alloc alloc = Alloc());
    BitArray(BitArray&& other) noexcept;

    // copy & move assign
    BitArray& operator=(const BitArray& rhs);
    BitArray& operator=(BitArray&& rhs) noexcept;

    // compare
    bool operator==(const BitArray& rhs) const;
    bool operator!=(const BitArray& rhs) const;

    // getter
    TBlock*       data();
    const TBlock* data() const;
    SizeType      size() const;
    SizeType      capacity() const;
    Alloc&        allocator();
    const Alloc&  allocator() const;
    bool          empty();

    // validate
    bool isValidIndex(SizeType idx);

    // memory op
    void clear();
    void release(SizeType capacity = 0);
    void reserve(SizeType capacity);
    void resize(SizeType size, bool new_value);
    void resizeUnsafe(SizeType size);

    // add
    SizeType add(bool v);
    SizeType add(bool v, SizeType n);

    // remove
    void removeAt(SizeType start, SizeType n = 1);
    void removeAtSwap(SizeType start, SizeType n = 1);

    // modify
    BitRef<TBlock> operator[](SizeType idx);
    bool           operator[](SizeType idx) const;

    // find
    SizeType find(bool v) const;
    SizeType findLast(bool v) const;

    // contain
    bool contain(bool v) const;

    // set range
    void setRange(SizeType start, SizeType n, bool v);

    // support foreach
    It  begin();
    It  end();
    CIt begin() const;
    CIt end() const;

private:
    // helper
    void _grow(SizeType size);
    void _resizeMemory(SizeType new_capacity);

private:
    TBlock*  m_data;
    SizeType m_size;
    SizeType m_capacity;
    Alloc    m_alloc;
};
} // namespace skr

// BitArray impl
namespace skr
{
// helper
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::_grow(SizeType size)
{
    if (m_size + size > m_capacity)
    {
        // calc new capacity
        SizeType old_block_size     = Algo::numBlocks(m_size);
        SizeType old_block_capacity = Algo::numBlocks(m_capacity);
        SizeType new_block_size     = Algo::numBlocks(m_size + size);
        SizeType new_block_capacity = m_alloc.getGrow(new_block_size, old_block_capacity);

        // realloc
        m_data = m_alloc.resizeContainer(m_data, old_block_size, old_block_capacity, new_block_capacity);

        // update capacity
        m_capacity = new_block_capacity << Algo::PerBlockSizeLog2;
    }
    // update size
    m_size += size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::_resizeMemory(SizeType new_capacity)
{
    SizeType block_size         = Algo::numBlocks(m_size);
    SizeType old_block_capacity = Algo::numBlocks(m_capacity);
    SizeType new_block_capacity = Algo::numBlocks(new_capacity);

    if (new_block_capacity)
    {
        // realloc
        m_data = m_alloc.resizeContainer(m_data, block_size, old_block_capacity, new_block_capacity);

        // clean new memory
        if (new_block_capacity > old_block_capacity)
            std::memset(data() + old_block_capacity, 0, (new_block_capacity - old_block_capacity) * sizeof(TBlock));

        // update size and capacity
        m_size     = std::min(m_size, m_capacity);
        m_capacity = new_block_capacity * Algo::PerBlockSize;
    }
    else if (m_data)
    {
        // free
        m_alloc.free(m_data);
        m_data     = nullptr;
        m_size     = 0;
        m_capacity = 0;
    }
}

// ctor & dtor
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(Alloc alloc)
    : m_data(nullptr)
    , m_size(0)
    , m_capacity(0)
    , m_alloc(std::move(alloc))
{
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(SizeType size, bool v, Alloc alloc)
    : m_data(nullptr)
    , m_size(0)
    , m_capacity(0)
    , m_alloc(std::move(alloc))
{
    resize(size, v);
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::~BitArray() { release(); }

// copy & move ctor
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(const BitArray& other, Alloc alloc)
    : m_data(nullptr)
    , m_size(0)
    , m_capacity(0)
    , m_alloc(std::move(alloc))
{
    // resize
    _resizeMemory(other.m_capacity);

    // copy
    m_size = other.size();
    if (other.m_size)
        std::memcpy(m_data, other.m_data, Algo::numBlocks(m_size) * sizeof(TBlock));
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>::BitArray(BitArray&& other) noexcept
    : m_data(other.m_data)
    , m_size(other.m_size)
    , m_capacity(other.m_capacity)
    , m_alloc(std::move(other.m_alloc))
{
    other.m_data     = nullptr;
    other.m_size     = 0;
    other.m_capacity = 0;
}

// copy & move assign
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>& BitArray<TBlock, Alloc>::operator=(const BitArray& rhs)
{
    if (this != &rhs)
    {
        _resizeMemory(rhs.m_size);
        m_size = rhs.m_size;
        if (m_size)
            std::memcpy(m_data, rhs.m_data, Algo::numBlocks(m_size) * sizeof(TBlock));
    }

    return *this;
}
template <typename TBlock, typename Alloc>
SKR_INLINE BitArray<TBlock, Alloc>& BitArray<TBlock, Alloc>::operator=(BitArray&& rhs) noexcept
{
    if (this != &rhs)
    {
        // release
        release();

        // copy data
        m_data     = rhs.m_data;
        m_size     = rhs.m_size;
        m_capacity = rhs.m_capacity;
        m_alloc    = std::move(rhs.m_alloc);

        // invalidate rhs
        rhs.m_data     = nullptr;
        rhs.m_size     = 0;
        rhs.m_capacity = 0;
    }

    return *this;
}

// compare
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator==(const BitArray& rhs) const
{
    if (m_size != rhs.m_size)
        return false;

    auto block_count   = intDivFloor(m_size, (SizeType)Algo::PerBlockSize);
    auto last_mask     = Algo::lastBlockMask(m_size);
    bool memcmp_result = std::memcmp(m_data, rhs.m_data, block_count * sizeof(TBlock)) == 0;
    bool last_result   = last_mask == Algo::FullMask || (m_data[block_count] & last_mask) == (rhs.m_data[block_count] & last_mask);
    return memcmp_result && last_result;
}
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator!=(const BitArray& rhs) const { return !(*this == rhs); }

// getter
template <typename TBlock, typename Alloc>
SKR_INLINE TBlock* BitArray<TBlock, Alloc>::data() { return m_data; }
template <typename TBlock, typename Alloc>
SKR_INLINE const TBlock* BitArray<TBlock, Alloc>::data() const { return m_data; }
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::size() const
{
    return m_size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::capacity() const
{
    return m_capacity;
}
template <typename TBlock, typename Alloc>
SKR_INLINE Alloc& BitArray<TBlock, Alloc>::allocator() { return m_alloc; }
template <typename TBlock, typename Alloc>
SKR_INLINE const Alloc& BitArray<TBlock, Alloc>::allocator() const { return m_alloc; }
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::empty() { return m_size == 0; }

// validate
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::isValidIndex(SizeType idx) { return idx >= 0 && idx < m_size; }

// memory op
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::clear()
{
    Algo::setBlocks(m_data, SizeType(0), Algo::numBlocks(m_size), false);
    m_size = 0;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::release(SizeType capacity)
{
    m_size = 0;
    _resizeMemory(capacity);
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::reserve(SizeType capacity)
{
    if (capacity > m_capacity)
        _resizeMemory(capacity);
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::resize(SizeType size, bool new_value)
{
    // do resize
    if (size > m_capacity)
    {
        _resizeMemory(size);
    }

    // set size
    auto old_size = m_size;
    m_size        = size;

    // try init
    if (size > old_size)
    {
        setRange(old_size, size - old_size, new_value);
    }
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::resizeUnsafe(SizeType size)
{
    // do resize
    if (size > m_capacity)
    {
        _resizeMemory(size);
    }

    // set size
    m_size = size;
}

// add
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::add(bool v)
{
    // do grow
    auto old_size = m_size;
    _grow(1);

    // set value
    (*this)[old_size] = v;
    return old_size;
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::add(bool v, SizeType n)
{
    // do grow
    auto old_size = m_size;
    _grow(n);

    // set value
    setRange(old_size, n, v);
    return old_size;
}

// remove
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::removeAt(SizeType start, SizeType n)
{
    SKR_Assert(start >= 0 && n > 0 && start + n < m_size);
    if (start + n != m_size)
    {
        It write(data(), size(), start);
        It read(data(), size(), start + n);

        while (read)
        {
            *write = *read;
            ++write;
            ++read;
        }
    }
    m_size -= n;
}
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::removeAtSwap(SizeType start, SizeType n)
{
    SKR_Assert(start >= 0 && n > 0 && start + n < m_size);
    if (start + n != m_size)
    {
        // adjust n
        auto move_n = std::min(n, m_size - start - n);

        // copy items
        for (SizeType i = 0; i < move_n; ++i)
        {
            (*this)[start + i] = (*this)[m_size - move_n + i];
        }
    }
    m_size -= n;
}

// modify
template <typename TBlock, typename Alloc>
SKR_INLINE BitRef<TBlock> BitArray<TBlock, Alloc>::operator[](SizeType idx)
{
    SKR_Assert(isValidIndex(idx));
    return BitRef<TBlock>(m_data[idx >> Algo::PerBlockSizeLog2], TBlock(1) << (idx & Algo::PerBlockSizeMask));
}
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::operator[](SizeType idx) const
{
    SKR_Assert(isValidIndex(idx));
    return m_data[idx >> Algo::PerBlockSizeLog2] & (1 << (idx & Algo::PerBlockSizeMask));
}

// find
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::find(bool v) const
{
    return Algo::find(m_data, SizeType(0), m_size, v);
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::SizeType BitArray<TBlock, Alloc>::findLast(bool v) const
{
    return Algo::findLast(m_data, SizeType(0), m_size, v);
}

// contain
template <typename TBlock, typename Alloc>
SKR_INLINE bool BitArray<TBlock, Alloc>::contain(bool v) const { return find(v) != npos_of<SizeType>; }

// set range
template <typename TBlock, typename Alloc>
SKR_INLINE void BitArray<TBlock, Alloc>::setRange(SizeType start, SizeType n, bool v)
{
    SKR_Assert(start >= 0 && n > 0 && start + n <= m_size);
    Algo::setRange(m_data, start, n, v);
}

// support foreach
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::It BitArray<TBlock, Alloc>::begin()
{
    return It(data(), size());
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::It BitArray<TBlock, Alloc>::end()
{
    return It(data(), size(), m_size);
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::CIt BitArray<TBlock, Alloc>::begin() const
{
    return CIt(data(), size());
}
template <typename TBlock, typename Alloc>
SKR_INLINE typename BitArray<TBlock, Alloc>::CIt BitArray<TBlock, Alloc>::end() const
{
    return CIt(data(), size(), m_size);
}

} // namespace skr
