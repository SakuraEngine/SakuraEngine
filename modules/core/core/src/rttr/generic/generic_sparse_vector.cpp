#include <SkrRTTR/generic/generic_sparse_vector.hpp>

namespace skr
{
// ctor & dtor
GenericSparseVector::GenericSparseVector(RC<IGenericBase> inner)
    : _inner(inner)
{
    if (_inner)
    {
        _inner_mem_traits = _inner->memory_traits_data();
        _inner_size       = _inner->size();
        _inner_alignment  = _inner->alignment();
    }
}
GenericSparseVector::~GenericSparseVector() = default;

// get type info
EGenericKind GenericSparseVector::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericSparseVector::id() const
{
    return kSparseVectorGenericId;
}

// get utils
MemoryTraitsData GenericSparseVector::memory_traits_data() const
{
    SKR_ASSERT(is_valid());
    MemoryTraitsData data     = _inner_mem_traits;
    data.use_ctor             = true;
    data.use_dtor             = true;
    data.use_move             = true;
    data.use_assign           = true;
    data.use_move_assign      = true;
    data.need_dtor_after_move = false;
    data.use_realloc          = false;
    data.use_compare          = true;
    return data;
}

// basic info
uint64_t GenericSparseVector::size() const
{
    return sizeof(SparseVectorMemoryBase);
}
uint64_t GenericSparseVector::alignment() const
{
    return alignof(SparseVectorMemoryBase);
}

// operations, used for generic container algorithms
bool GenericSparseVector::support(EGenericFeature feature) const
{
    switch (feature)
    {
    case EGenericFeature::DefaultCtor:
        return true;
    case EGenericFeature::Dtor:
    case EGenericFeature::Copy:
    case EGenericFeature::Move:
    case EGenericFeature::Assign:
    case EGenericFeature::MoveAssign:
    case EGenericFeature::Equal:
        return _inner && _inner->support(feature);
    case EGenericFeature::Hash:
        return false;
    case EGenericFeature::Swap:
        return true;
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
void GenericSparseVector::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        dst_mem->_reset();
    }
}
void GenericSparseVector::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* p_dst = ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i);
        release(p_dst);
    }
}
void GenericSparseVector::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        const auto* src_mem = reinterpret_cast<const SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SparseVectorMemoryBase), i)
        );

        if (src_mem->sparse_size())
        {
            // reserve data
            _realloc(dst_mem, src_mem->sparse_size());

            // copy data
            _copy_sparse_vector_data(
                dst_mem->_data,
                src_mem->_data,
                src_mem->_data,
                src_mem->sparse_size()
            );
            _copy_sparse_vector_bit_data(
                dst_mem->_bit_data,
                src_mem->_bit_data,
                src_mem->sparse_size()
            );
            dst_mem->set_sparse_size(src_mem->sparse_size());
            dst_mem->set_freelist_head(src_mem->freelist_head());
            dst_mem->set_hole_size(src_mem->hole_size());
        }
    }
}
void GenericSparseVector::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SparseVectorMemoryBase), i)
        );

        *dst_mem = std::move(*src_mem);
        src_mem->_reset();
    }
}
void GenericSparseVector::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    if (dst == src) { return; }

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        const auto* src_mem = reinterpret_cast<const SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SparseVectorMemoryBase), i)
        );

        // clean up self
        clear(dst_mem);

        // copy data
        if ((src_mem->sparse_size() - src_mem->hole_size()) > 0)
        {
            // reserve data
            if (dst_mem->capacity() < src_mem->sparse_size())
            {
                _realloc(dst_mem, src_mem->sparse_size());
            }

            // copy data
            _copy_sparse_vector_data(
                dst_mem->_data,
                src_mem->_data,
                src_mem->_bit_data,
                src_mem->sparse_size()
            );
            _copy_sparse_vector_bit_data(
                dst_mem->_bit_data,
                src_mem->_bit_data,
                src_mem->sparse_size()
            );
            dst_mem->set_sparse_size(src_mem->sparse_size());
            dst_mem->set_freelist_head(src_mem->freelist_head());
            dst_mem->set_hole_size(src_mem->hole_size());
        }
    }
}
void GenericSparseVector::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    if (dst == src) { return; }

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SparseVectorMemoryBase), i)
        );

        // clean up dst
        clear(dst_mem);

        // free dst
        _free(dst_mem);

        // move data
        *dst_mem = std::move(*src_mem);

        // reset src
        src_mem->_reset();
    }
}
bool GenericSparseVector::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        const auto* lhs_mem = reinterpret_cast<const SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(lhs, sizeof(SparseVectorMemoryBase), i)
        );
        const auto* rhs_mem = reinterpret_cast<const SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(rhs, sizeof(SparseVectorMemoryBase), i)
        );

        // compare size
        if (lhs_mem->sparse_size() != rhs_mem->sparse_size())
        {
            return false;
        }

        // compare data
        for (uint64_t cmp_idx = 0; cmp_idx < lhs_mem->sparse_size(); ++cmp_idx)
        {
            bool lhs_has_data = has_data(lhs_mem, cmp_idx);
            bool rhs_has_data = has_data(rhs_mem, cmp_idx);

            // compare data existence
            if (lhs_has_data != rhs_has_data)
            {
                return false;
            }

            // compare data
            if (lhs_has_data)
            {
                if (!_inner->equal(
                        lhs_mem->_storage_at(cmp_idx, _inner_size),
                        rhs_mem->_storage_at(cmp_idx, _inner_size),
                        1
                    ))
                {
                    return false;
                }
            }
        }
    }
    return true;
}
size_t GenericSparseVector::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(src);
    SKR_ASSERT(false && "GenericSparseVector does not support hash operation, please check feature first");
    return 0;
}
void GenericSparseVector::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SparseVectorMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SparseVectorMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SparseVectorMemoryBase), i)
        );

        // swap data
        SparseVectorMemoryBase tmp = std::move(*dst_mem);
        *dst_mem                   = std::move(*src_mem);
        *src_mem                   = std::move(tmp);
    }
}

// getter
bool GenericSparseVector::is_valid() const
{
    return _inner != nullptr;
}
RC<IGenericBase> GenericSparseVector::inner() const
{
    SKR_ASSERT(is_valid());
    return _inner;
}

// sparse vector getter
uint64_t GenericSparseVector::size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_sparse_size;
}
uint64_t GenericSparseVector::capacity(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_capacity;
}
uint64_t GenericSparseVector::slack(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_capacity - dst_mem->_sparse_size + dst_mem->_hole_size;
}
uint64_t GenericSparseVector::sparse_size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_sparse_size;
}
uint64_t GenericSparseVector::hole_size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_hole_size;
}
uint64_t GenericSparseVector::bit_size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return BitAlgo::num_blocks(dst_mem->_sparse_size) * BitAlgo::PerBlockSize;
}
uint64_t GenericSparseVector::freelist_head(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_freelist_head;
}
bool GenericSparseVector::is_compact(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_hole_size == 0;
}
bool GenericSparseVector::is_empty(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return (dst_mem->_sparse_size - dst_mem->_hole_size) == 0;
}
void* GenericSparseVector::storage(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    return dst_mem->_data;
}
const void* GenericSparseVector::storage(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_data;
}
uint64_t* GenericSparseVector::bit_data(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    return dst_mem->_typed_bit_data();
}
const uint64_t* GenericSparseVector::bit_data(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return dst_mem->_typed_bit_data();
}

// validate
bool GenericSparseVector::has_data(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(const_cast<void*>(dst));
    return BitAlgo::get(dst_mem->_typed_bit_data(), idx);
}
bool GenericSparseVector::is_hole(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(const_cast<void*>(dst));
    return !BitAlgo::get(dst_mem->_typed_bit_data(), idx);
}
bool GenericSparseVector::is_valid_index(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);
    return idx >= 0 && idx < dst_mem->_sparse_size;
}

// memory op
void GenericSparseVector::clear(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (dst_mem->_sparse_size)
    {
        // destruct items
        _destruct_sparse_vector_data(dst_mem->_data, dst_mem->_typed_bit_data(), dst_mem->_sparse_size);

        // clean up bit data
        if (dst_mem->_bit_data)
        {
            BitAlgo::set_blocks(
                dst_mem->_typed_bit_data(),
                (uint64_t)0,
                BitAlgo::num_blocks(dst_mem->_sparse_size),
                false
            );
        }

        // clean up data
        dst_mem->_hole_size     = 0;
        dst_mem->_sparse_size   = 0;
        dst_mem->_freelist_head = npos;
    }
}
void GenericSparseVector::release(void* dst, uint64_t reserve_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    // clean up self
    clear(dst_mem);

    // do reserve or free
    if (reserve_capacity)
    {
        if (reserve_capacity != dst_mem->_capacity)
        {
            _realloc(dst_mem, reserve_capacity);
        }
    }
    else
    {
        _free(dst_mem);
    }
}
void GenericSparseVector::reserve(void* dst, uint64_t expect_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (expect_capacity > dst_mem->_capacity)
    {
        _realloc(dst_mem, expect_capacity);
    }
}
void GenericSparseVector::shrink(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    compact_top(dst_mem);
    auto new_capacity = container::default_get_shrink<uint64_t>(dst_mem->_sparse_size, dst_mem->_capacity);
    SKR_ASSERT(new_capacity >= dst_mem->_sparse_size);
    if (new_capacity < dst_mem->_capacity)
    {
        if (new_capacity)
        {
            _realloc(dst_mem, new_capacity);
        }
        else
        {
            _free(dst_mem);
        }
    }
}
bool GenericSparseVector::compact(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (!is_empty(dst) && !is_compact(dst))
    {
        // fill hole
        auto compacted_index = dst_mem->_sparse_size - dst_mem->_hole_size;
        auto search_index    = dst_mem->_sparse_size;
        auto free_node       = dst_mem->_freelist_head;
        while (free_node != npos)
        {
            auto next_index = dst_mem->_freelist_node_at(free_node, _inner_size)->next;
            if (free_node < compacted_index)
            {
                // find last allocated element
                do
                {
                    --search_index;
                } while (!has_data(dst_mem, search_index));

                // move element to the hole
                _inner->move(
                    dst_mem->_storage_at(free_node, _inner_size),
                    dst_mem->_storage_at(search_index, _inner_size)
                );
            }
            free_node = next_index;
        }

        // setup bit data
        BitAlgo::set_range(
            dst_mem->_typed_bit_data(),
            compacted_index,
            dst_mem->_hole_size,
            false
        );
        BitAlgo::set_range(
            dst_mem->_typed_bit_data(),
            (uint64_t)0,
            compacted_index,
            true
        );

        // setup data
        dst_mem->_hole_size     = 0;
        dst_mem->_freelist_head = npos;
        dst_mem->_sparse_size   = compacted_index;

        return true;
    }
    else
    {
        return false;
    }
}
bool GenericSparseVector::compact_stable(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (!is_empty(dst) && !is_compact(dst))
    {
        auto compacted_index = dst_mem->_sparse_size - dst_mem->_hole_size;
        auto read_index      = dst_mem->_sparse_size;
        auto write_index     = 0;

        // skip first compacted range
        while (has_data(dst_mem, write_index) && write_index != dst_mem->_sparse_size)
            ++write_index;
        read_index = write_index + 1;

        // copy items
        while (read_index < dst_mem->_sparse_size)
        {
            // skip hole
            while (read_index < dst_mem->_sparse_size && !has_data(dst_mem, read_index))
                ++read_index;

            // move items
            while (read_index < dst_mem->_sparse_size && has_data(dst_mem, read_index))
            {
                _inner->move(
                    dst_mem->_storage_at(write_index, _inner_size),
                    dst_mem->_storage_at(read_index, _inner_size)
                );
                ++write_index;
                ++read_index;
            }
        }

        // setup bit data
        BitAlgo::set_range(
            dst_mem->_typed_bit_data(),
            compacted_index,
            dst_mem->_hole_size,
            false
        );
        BitAlgo::set_range(
            dst_mem->_typed_bit_data(),
            (uint64_t)0,
            compacted_index,
            true
        );

        // reset data
        dst_mem->_hole_size     = 0;
        dst_mem->_freelist_head = npos;
        dst_mem->_sparse_size   = compacted_index;

        return true;
    }
    else
    {
        return false;
    }
}
bool GenericSparseVector::compact_top(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (!is_empty(dst) && !is_compact(dst))
    {
        bool has_changes = false;
        for (uint64_t i = dst_mem->_sparse_size; i; --i)
        {
            if (is_hole(dst, i - 1))
            {
                // remove from freelist
                dst_mem->_hole_size--;
                _break_freelist_at(dst, i - 1);

                // update size
                dst_mem->_sparse_size--;

                has_changes = true;
            }
            else
            {
                break;
            }
        }
        return has_changes;
    }
    else
    {
        return false;
    }
}

// add
GenericSparseVectorDataRef GenericSparseVector::add(void* dst, const void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    auto info = add_unsafe(dst);
    _inner->copy(
        info.ptr,
        v
    );
    return info;
}

GenericSparseVectorDataRef GenericSparseVector::add_move(void* dst, void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    auto info = add_unsafe(dst);
    _inner->move(
        info.ptr,
        v
    );
    return info;
}
GenericSparseVectorDataRef GenericSparseVector::add_unsafe(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    uint64_t index;

    if (dst_mem->_hole_size)
    {
        // remove and use first index from freelist
        index                   = dst_mem->_freelist_head;
        dst_mem->_freelist_head = dst_mem->_freelist_node_at(index, _inner_size)->next;
        dst_mem->_hole_size--;

        // break prev link
        if (dst_mem->_hole_size)
        {
            dst_mem->_freelist_node_at(dst_mem->_freelist_head, _inner_size)->prev = npos;
        }
    }
    else
    {
        index = _grow(dst, 1);
    }

    // setup bit
    BitAlgo::set(dst_mem->_typed_bit_data(), index, true);

    return {
        dst_mem->_storage_at(index, _inner_size),
        index
    };
}
GenericSparseVectorDataRef GenericSparseVector::add_default(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto info = add_unsafe(dst);
    _inner->default_ctor(info.ptr, 1);
    return info;
}
GenericSparseVectorDataRef GenericSparseVector::add_zeroed(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto info = add_unsafe(dst);
    ::skr::memory::zero_memory(info.ptr, _inner_size);
    return info;
}

// add at
void GenericSparseVector::add_at(void* dst, uint64_t idx, const void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx);
    _inner->copy(
        dst_mem->_storage_at(idx, _inner_size),
        v
    );
}
void GenericSparseVector::add_at_move(void* dst, uint64_t idx, void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx);
    _inner->move(
        dst_mem->_storage_at(idx, _inner_size),
        v
    );
}
void GenericSparseVector::add_at_unsafe(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_hole(dst, idx));
    SKR_ASSERT(is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    // remove from freelist
    dst_mem->_hole_size--;
    _break_freelist_at(dst, idx);

    // setup bit
    BitAlgo::set(dst_mem->_typed_bit_data(), idx, true);
}
void GenericSparseVector::add_at_default(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx);
    _inner->default_ctor(
        dst_mem->_storage_at(idx, _inner_size)
    );
}
void GenericSparseVector::add_at_zeroed(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx);
    ::skr::memory::zero_memory(
        dst_mem->_storage_at(idx, _inner_size),
        _inner_size
    );
}

// append
void GenericSparseVector::append(void* dst, const void* other) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(other);
    auto*       dst_mem   = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    const auto* other_mem = reinterpret_cast<const SparseVectorMemoryBase*>(other);

    const uint64_t other_size = other_mem->sparse_size() - other_mem->hole_size();

    auto bit_cursor = CTrueBitCursor::Begin(
        other_mem->_typed_bit_data(),
        other_mem->sparse_size()
    );

    // fill holes
    uint64_t count = 0;
    while (dst_mem->hole_size() > 0 && !bit_cursor.reach_end())
    {
        add(dst_mem, other_mem->_storage_at(bit_cursor.index(), _inner_size));
        bit_cursor.move_next();
        ++count;
    }

    // grow and copy
    if (!bit_cursor.reach_end())
    {
        auto write_idx  = dst_mem->sparse_size();
        auto grow_count = other_size - count;
        _grow(dst, grow_count);
        BitAlgo::set_range(dst_mem->_typed_bit_data(), write_idx, grow_count, true);
        while (!bit_cursor.reach_end())
        {
            _inner->copy(
                dst_mem->_storage_at(write_idx, _inner_size),
                other_mem->_storage_at(bit_cursor.index(), _inner_size)
            );
            ++write_idx;
            bit_cursor.move_next();
        }
        SKR_ASSERT(write_idx == sparse_size(dst));
    }
}

// remove
void GenericSparseVector::remove_at(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    SKR_ASSERT(is_valid_index(dst, idx + n - 1));
    SKR_ASSERT(n > 0);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    // destruct items
    if (_inner_mem_traits.use_dtor)
    {
        for (uint64_t i = 0; i < n; ++i)
        {
            SKR_ASSERT(has_data(dst, idx + i));
            _inner->dtor(dst_mem->_storage_at(idx + i, _inner_size), 1);
        }
    }

    // remove at unsafe
    remove_at_unsafe(dst, idx, n);
}
void GenericSparseVector::remove_at_unsafe(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    SKR_ASSERT(is_valid_index(dst, idx + n - 1));
    SKR_ASSERT(n > 0);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    for (; n; --n)
    {
        SKR_ASSERT(has_data(dst, idx));

        // link to free list
        auto* node = dst_mem->_freelist_node_at(idx, _inner_size);
        if (dst_mem->_hole_size)
        { // link head
            auto* old_head_node = dst_mem->_freelist_node_at(dst_mem->_freelist_head, _inner_size);
            old_head_node->prev = idx;
        }
        node->prev              = npos;
        node->next              = dst_mem->_freelist_head;
        dst_mem->_freelist_head = idx;
        dst_mem->_hole_size++;

        // set flag
        BitAlgo::set(dst_mem->_typed_bit_data(), idx, false);

        // update index
        ++idx;
    }
}
bool GenericSparseVector::remove(void* dst, const void* v) const
{
    return remove_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}
bool GenericSparseVector::remove_last(void* dst, const void* v) const
{
    return remove_last_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}
uint64_t GenericSparseVector::remove_all(void* dst, const void* v) const
{
    return remove_all_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}

// remove if
bool GenericSparseVector::remove_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    if (auto ref = find_if(dst, pred))
    {
        remove_at(dst, ref.index);
        return true;
    }
    return false;
}
bool GenericSparseVector::remove_last_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    if (auto ref = find_last_if(dst, pred))
    {
        remove_at(dst, ref.index);
        return true;
    }
    return false;
}
uint64_t GenericSparseVector::remove_all_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    uint64_t count      = 0;
    auto     bit_cursor = CTrueBitCursor::Begin(
        dst_mem->_typed_bit_data(),
        dst_mem->_sparse_size
    );

    while (!bit_cursor.reach_end())
    {
        if (pred(dst_mem->_storage_at(
                bit_cursor.index(),
                _inner_size
            )))
        {
            // remove at
            remove_at(dst, bit_cursor.index(), 1);
            ++count;
        }

        bit_cursor.move_next();
    }

    return count;
}

// access
void* GenericSparseVector::at(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    SKR_ASSERT(has_data(dst, idx));
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    return dst_mem->_storage_at(idx, _inner_size);
}
void* GenericSparseVector::at_last(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    idx           = dst_mem->_sparse_size - idx - 1;
    SKR_ASSERT(is_valid_index(dst, idx));
    SKR_ASSERT(has_data(dst, idx));
    return dst_mem->_storage_at(idx, _inner_size);
}
const void* GenericSparseVector::at(const void* dst, uint64_t idx) const
{
    return at(const_cast<void*>(dst), idx);
}
const void* GenericSparseVector::at_last(const void* dst, uint64_t idx) const
{
    return at_last(const_cast<void*>(dst), idx);
}

// find
GenericSparseVectorDataRef GenericSparseVector::find(void* dst, const void* v) const
{
    return find_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}
GenericSparseVectorDataRef GenericSparseVector::find_last(void* dst, const void* v) const
{
    return find_last_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}
CGenericSparseVectorDataRef GenericSparseVector::find(const void* dst, const void* v) const
{
    return find(const_cast<void*>(dst), v);
}
CGenericSparseVectorDataRef GenericSparseVector::find_last(const void* dst, const void* v) const
{
    return find_last(const_cast<void*>(dst), v);
}

// find if
GenericSparseVectorDataRef GenericSparseVector::find_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    auto bit_cursor = CTrueBitCursor::Begin(
        dst_mem->_typed_bit_data(),
        dst_mem->_sparse_size
    );

    while (!bit_cursor.reach_end())
    {
        if (pred(dst_mem->_storage_at(bit_cursor.index(), _inner_size)))
        {
            return {
                dst_mem->_storage_at(bit_cursor.index(), _inner_size),
                bit_cursor.index()
            };
        }
        bit_cursor.move_next();
    }
    return {};
}
GenericSparseVectorDataRef GenericSparseVector::find_last_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    auto bit_cursor = CTrueBitCursor::End(
        dst_mem->_typed_bit_data(),
        dst_mem->_sparse_size
    );

    while (!bit_cursor.reach_begin())
    {
        if (pred(dst_mem->_storage_at(bit_cursor.index(), _inner_size)))
        {
            return {
                dst_mem->_storage_at(bit_cursor.index(), _inner_size),
                bit_cursor.index()
            };
        }
        bit_cursor.move_prev();
    }
    return {};
}
CGenericSparseVectorDataRef GenericSparseVector::find_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}
CGenericSparseVectorDataRef GenericSparseVector::find_last_if(const void* dst, PredType pred) const
{
    return find_last_if(const_cast<void*>(dst), pred);
}

// contains & count
bool GenericSparseVector::contains(const void* dst, const void* v) const
{
    return contains_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}
uint64_t GenericSparseVector::count(const void* dst, const void* v) const
{
    return count_if(dst, [this, v](const void* data) {
        return _inner->equal(data, v, 1);
    });
}

// contains if & count if
bool GenericSparseVector::contains_if(const void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    return (bool)find_if(dst, pred);
}
uint64_t GenericSparseVector::count_if(const void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<const SparseVectorMemoryBase*>(dst);

    uint64_t count      = 0;
    auto     bit_cursor = CTrueBitCursor::Begin(
        dst_mem->_typed_bit_data(),
        dst_mem->sparse_size()
    );

    while (!bit_cursor.reach_end())
    {
        if (pred(dst_mem->_storage_at(bit_cursor.index(), _inner_size)))
        {
            ++count;
        }
        bit_cursor.move_next();
    }
    return count;
}

// helper
void GenericSparseVector::_realloc(void* dst, uint64_t new_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);
    SKR_ASSERT(new_capacity != dst_mem->_capacity);
    SKR_ASSERT(new_capacity > 0);
    SKR_ASSERT((dst_mem->_capacity > 0 && dst_mem->_data != nullptr) || (dst_mem->_capacity == 0 && dst_mem->_data == nullptr));

    auto storage_size = SparseVectorMemoryBase::_storage_size(_inner_size);

    // realloc bit data
    uint64_t new_block_size = BitAlgo::num_blocks(new_capacity);
    uint64_t old_block_size = BitAlgo::num_blocks(dst_mem->_capacity);
    if (new_block_size != old_block_size)
    {
        if constexpr (::skr::memory::MemoryTraits<uint64_t>::use_realloc)
        {
            dst_mem->_bit_data = SkrAllocator::realloc_raw(
                dst_mem->_bit_data,
                new_block_size,
                sizeof(uint64_t),
                alignof(uint64_t)
            );
        }
        else
        {
            // alloc new memory
            void* new_memory = SkrAllocator::alloc_raw(
                new_block_size,
                sizeof(uint64_t),
                alignof(uint64_t)
            );

            // move data
            if (old_block_size)
            {
                ::skr::memory::move(
                    (uint64_t*)new_memory,
                    (uint64_t*)dst_mem->_bit_data,
                    std::min(old_block_size, new_block_size)
                );
            }

            // release old memory
            SkrAllocator::free_raw(dst_mem->_bit_data, alignof(uint64_t));

            // updata data
            dst_mem->_bit_data = new_memory;
        }
    }

    // realloc data
    if (_inner_mem_traits.use_realloc)
    {
        dst_mem->_data = SkrAllocator::realloc_raw(
            dst_mem->_data,
            new_capacity,
            storage_size,
            _inner_alignment
        );
    }
    else
    {
        // alloc new memory
        void* new_memory = SkrAllocator::alloc_raw(
            new_capacity,
            storage_size,
            _inner_alignment
        );

        // move items
        if (dst_mem->_sparse_size)
        {
            _move_sparse_vector_data(
                new_memory,
                dst_mem->_data,
                dst_mem->_bit_data,
                dst_mem->_sparse_size
            );
        }

        // release old memory
        SkrAllocator::free_raw(dst_mem->_data, _inner_alignment);

        // update data
        dst_mem->_data = new_memory;
    }

    // update capacity
    dst_mem->_capacity = new_capacity;
}
void GenericSparseVector::_free(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    if (dst_mem->_data)
    {
        // release memory
        SkrAllocator::free_raw(
            dst_mem->_data,
            _inner_alignment
        );
        SkrAllocator::free_raw(
            dst_mem->_bit_data,
            alignof(uint64_t)
        );

        // reset data
        dst_mem->_data     = nullptr;
        dst_mem->_bit_data = nullptr;
        dst_mem->_capacity = 0;
    }
}
uint64_t GenericSparseVector::_grow(void* dst, uint64_t grow_size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(grow_size > 0);
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    auto old_sparse_size = dst_mem->_sparse_size;
    auto new_sparse_size = old_sparse_size + grow_size;

    if (new_sparse_size > dst_mem->_capacity)
    {
        auto new_capacity = container::default_get_grow<uint64_t>(
            new_sparse_size,
            dst_mem->_capacity
        );
        SKR_ASSERT(new_capacity >= new_sparse_size);
        if (new_capacity > dst_mem->_capacity)
        {
            _realloc(dst_mem, new_capacity);
        }
    }

    dst_mem->_sparse_size = new_sparse_size;
    return old_sparse_size;
}
void GenericSparseVector::_copy_sparse_vector_data(void* dst, const void* src, const void* src_bit_data, uint64_t size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(src_bit_data);
    SKR_ASSERT(size > 0);
    auto storage_size = SparseVectorMemoryBase::_storage_size(_inner_size);

    if (_inner_mem_traits.use_copy)
    {
        for (uint64_t i = 0; i < size; ++i)
        {
            auto* p_dst_data = ::skr::memory::offset_item(
                dst,
                storage_size,
                i
            );
            auto* p_src_data = ::skr::memory::offset_item(
                src,
                storage_size,
                i
            );

            if (BitAlgo::get((const uint64_t*)src_bit_data, i))
            {
                _inner->copy(p_dst_data, p_src_data, 1);
            }
            else
            {
                auto* dst_node = reinterpret_cast<SparseVectorFreeListNode*>(p_dst_data);
                auto* src_node = reinterpret_cast<const SparseVectorFreeListNode*>(p_src_data);

                dst_node->next = src_node->next;
                dst_node->prev = src_node->prev;
            }
        }
    }
    else
    {
        std::memcpy(dst, src, storage_size * size);
    }
}
void GenericSparseVector::_move_sparse_vector_data(void* dst, void* src, const void* src_bit_data, uint64_t size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(src_bit_data);
    SKR_ASSERT(size > 0);
    auto storage_size = SparseVectorMemoryBase::_storage_size(_inner_size);

    if (_inner_mem_traits.use_move)
    {
        for (uint64_t i = 0; i < size; ++i)
        {
            auto* p_dst_data = ::skr::memory::offset_item(
                dst,
                storage_size,
                i
            );
            auto* p_src_data = ::skr::memory::offset_item(
                src,
                storage_size,
                i
            );

            if (BitAlgo::get((const uint64_t*)src_bit_data, i))
            {
                _inner->move(p_dst_data, p_src_data);
            }
            else
            {
                auto* dst_node = reinterpret_cast<SparseVectorFreeListNode*>(p_dst_data);
                auto* src_node = reinterpret_cast<SparseVectorFreeListNode*>(p_src_data);

                dst_node->next = src_node->next;
                dst_node->prev = src_node->prev;
            }
        }
    }
    else
    {
        std::memmove(dst, src, storage_size * size);
    }
}
void GenericSparseVector::_copy_sparse_vector_bit_data(void* dst, const void* src, uint64_t size) const
{
    std::memcpy(
        dst,
        src,
        BitAlgo::num_blocks(size) * sizeof(uint64_t)
    );
}
void GenericSparseVector::_move_sparse_vector_bit_data(void* dst, void* src, uint64_t size) const
{
    std::memcpy(
        dst,
        src,
        BitAlgo::num_blocks(size) * sizeof(uint64_t)
    );
}
void GenericSparseVector::_destruct_sparse_vector_data(void* dst, const void* bit_data, uint64_t size) const
{
    if (_inner_mem_traits.use_dtor)
    {
        auto bit_cursor = CTrueBitCursor::Begin(
            (const uint64_t*)bit_data,
            size
        );
        auto storage_size = SparseVectorMemoryBase::_storage_size(_inner_size);

        while (!bit_cursor.reach_end())
        {
            auto* p_dst_data = ::skr::memory::offset_item(
                dst,
                storage_size,
                bit_cursor.index()
            );

            _inner->dtor(p_dst_data);
            bit_cursor.move_next();
        }
    }
}
void GenericSparseVector::_break_freelist_at(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<SparseVectorMemoryBase*>(dst);

    auto* node = dst_mem->_freelist_node_at(idx, _inner_size);

    if (dst_mem->_freelist_head == idx)
    {
        dst_mem->_freelist_head = node->next;
    }
    if (node->next != npos)
    {
        auto* next_node = dst_mem->_freelist_node_at(node->next, _inner_size);
        next_node->prev = node->prev;
    }
    if (node->prev != npos)
    {
        auto* prev_node = dst_mem->_freelist_node_at(node->prev, _inner_size);
        prev_node->next = node->next;
    }
}
} // namespace skr