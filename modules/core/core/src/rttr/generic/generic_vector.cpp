#include <SkrRTTR/generic/generic_vector.hpp>

namespace skr
{
// ctor & dtor
GenericVector::GenericVector(RC<IGenericBase> inner)
    : _inner(inner)
{
    if (_inner)
    {
        _inner_mem_traits = _inner->memory_traits_data();
        _inner_size       = _inner->size();
        _inner_alignment  = _inner->alignment();
    }
}
GenericVector::~GenericVector() = default;

// get type info
EGenericKind GenericVector::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericVector::id() const
{
    return kVectorGenericId;
}

// get utils
MemoryTraitsData GenericVector::memory_traits_data() const
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
uint64_t GenericVector::size() const
{
    return sizeof(VectorMemoryBase);
}
uint64_t GenericVector::alignment() const
{
    return alignof(VectorMemoryBase);
}

// operations, used for generic container algorithms
bool GenericVector::support(EGenericFeature feature) const
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
void GenericVector::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    std::memset(dst, 0, sizeof(VectorMemoryBase) * count);
}
void GenericVector::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* p_dst = VectorMemoryBase::_memory_at(dst, i);
        release(p_dst);
    }
}
void GenericVector::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto*       dst_mem = VectorMemoryBase::_memory_at(dst, i);
        const auto* src_mem = VectorMemoryBase::_memory_at(src, i);

        if (src_mem->_size)
        {
            _realloc(dst_mem, src_mem->_size);
            _inner->copy(
                dst_mem->_data,
                src_mem->_data,
                src_mem->_size
            );
            dst_mem->_size = src_mem->_size;
        }
    }
}
void GenericVector::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    if (dst == src) { return; }

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = VectorMemoryBase::_memory_at(dst, i);
        auto* src_mem = VectorMemoryBase::_memory_at(src, i);

        // move data
        dst_mem->_size     = src_mem->_size;
        dst_mem->_capacity = src_mem->_capacity;
        dst_mem->_data     = src_mem->_data;

        // clean up src
        src_mem->_data     = nullptr;
        src_mem->_capacity = 0;
        src_mem->_size     = 0;
    }
}
void GenericVector::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    if (dst == src) { return; }

    for (uint64_t i = 0; i < count; ++i)
    {
        auto*       dst_mem = VectorMemoryBase::_memory_at(dst, i);
        const auto* src_mem = VectorMemoryBase::_memory_at(src, i);

        // clean up dst
        clear(dst_mem);

        // copy data
        if (src_mem->_size)
        {
            // reserve memory
            if (dst_mem->capacity() < src_mem->_size)
            {
                _realloc(dst_mem, src_mem->_size);
            }

            // copy data
            _inner->copy(
                dst_mem->_data,
                src_mem->_data,
                src_mem->_size
            );
            dst_mem->_size = src_mem->_size;
        }
    }
}
void GenericVector::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = VectorMemoryBase::_memory_at(dst, i);
        auto* src_mem = VectorMemoryBase::_memory_at(src, i);

        // clean up dst
        clear(dst_mem);
        free(dst_mem);

        // move data
        dst_mem->_size     = src_mem->_size;
        dst_mem->_capacity = src_mem->_capacity;
        dst_mem->_data     = src_mem->_data;

        // clean up src
        src_mem->_size     = 0;
        src_mem->_capacity = 0;
        src_mem->_data     = nullptr;
    }
}
bool GenericVector::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto lhs_mem = VectorMemoryBase::_memory_at(lhs, i);
        auto rhs_mem = VectorMemoryBase::_memory_at(rhs, i);

        // check size
        if (lhs_mem->_size != rhs_mem->_size)
        {
            return false;
        }

        // check data
        if (!_inner->equal(
                lhs_mem->_data,
                rhs_mem->_data,
                lhs_mem->_size
            ))
        {
            return false;
        }
    }

    return true;
}
size_t GenericVector::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(src);
    SKR_ASSERT(false && "GenericVector does not support hash operation, please check feature first");
    return 0;
}
void GenericVector::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = VectorMemoryBase::_memory_at(dst, i);
        auto* src_mem = VectorMemoryBase::_memory_at(src, i);

        // swap data
        VectorMemoryBase tmp = std::move(*dst_mem);
        *dst_mem             = std::move(*src_mem);
        *src_mem             = std::move(tmp);
    }
}

// getter
bool GenericVector::is_valid() const
{
    return _inner != nullptr;
}
RC<IGenericBase> GenericVector::inner() const
{
    SKR_ASSERT(is_valid());
    return _inner;
}

// vector getter
uint64_t GenericVector::size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return reinterpret_cast<const VectorMemoryBase*>(dst)->_size;
}
uint64_t GenericVector::capacity(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return reinterpret_cast<const VectorMemoryBase*>(dst)->capacity();
}
uint64_t GenericVector::slack(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<const VectorMemoryBase*>(dst);
    return dst_mem->capacity() - dst_mem->_size;
}
bool GenericVector::is_empty(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return reinterpret_cast<const VectorMemoryBase*>(dst)->_size == 0;
}
const void* GenericVector::data(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return reinterpret_cast<const VectorMemoryBase*>(dst)->_data;
}
void* GenericVector::data(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return reinterpret_cast<VectorMemoryBase*>(dst)->_data;
}

// validate
bool GenericVector::is_valid_index(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto* dst_mem = reinterpret_cast<const VectorMemoryBase*>(dst);
    return idx >= 0 && idx < dst_mem->_size;
}

// memory op
void GenericVector::clear(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (dst_mem->_size)
    {
        if (_inner_mem_traits.use_dtor)
        {
            _inner->dtor(
                dst_mem->_data,
                dst_mem->_size
            );
        }
        dst_mem->_size = 0;
    }
}
void GenericVector::release(void* dst, uint64_t reserve_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    clear(dst);

    if (reserve_capacity)
    {
        if (reserve_capacity != dst_mem->capacity())
        {
            _realloc(dst, reserve_capacity);
        }
    }
    else
    {
        _free(dst);
    }
}
void GenericVector::reserve(void* dst, uint64_t expect_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (expect_capacity > dst_mem->capacity())
    {
        _realloc(dst, expect_capacity);
    }
}
void GenericVector::shrink(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    auto new_capacity = container::default_get_shrink<uint64_t>(
        dst_mem->_size,
        dst_mem->capacity()
    );
    SKR_ASSERT(new_capacity >= dst_mem->_size);
    if (new_capacity < dst_mem->capacity())
    {
        if (new_capacity)
        {
            _realloc(dst, new_capacity);
        }
        else
        {
            _free(dst);
        }
    }
}
void GenericVector::resize(void* dst, uint64_t expect_size, void* new_value) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    // realloc memory if need
    if (expect_size > dst_mem->capacity())
    {
        _realloc(dst, expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > dst_mem->_size)
    {
        for (uint64_t i = dst_mem->_size; i < expect_size; ++i)
        {
            void* p_copy_data = dst_mem->_item_at(i, _inner_size);
            _inner->copy(
                p_copy_data,
                new_value
            );
        }
    }
    else if (expect_size < dst_mem->_size)
    {
        void* p_destruct_data = dst_mem->_item_at(expect_size, _inner_size);
        _inner->dtor(
            p_destruct_data,
            dst_mem->_size - expect_size
        );
    }

    // set size
    dst_mem->_size = expect_size;
}
void GenericVector::resize_unsafe(void* dst, uint64_t expect_size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    // realloc memory if need
    if (expect_size > dst_mem->capacity())
    {
        _realloc(dst, expect_size);
    }

    // construct item or destruct item if need
    if (expect_size < dst_mem->_size)
    {
        void* p_destruct_data = dst_mem->_item_at(expect_size, _inner_size);
        _inner->dtor(
            p_destruct_data,
            dst_mem->_size - expect_size
        );
    }

    // set size
    dst_mem->_size = expect_size;
}
void GenericVector::resize_default(void* dst, uint64_t expect_size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    // realloc memory if need
    if (expect_size > dst_mem->capacity())
    {
        _realloc(dst, expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > dst_mem->_size)
    {
        void* p_construct_data = dst_mem->_item_at(dst_mem->_size, _inner_size);
        _inner->default_ctor(
            p_construct_data,
            expect_size - dst_mem->_size
        );
    }
    else if (expect_size < dst_mem->_size)
    {
        void* p_destruct_data = dst_mem->_item_at(expect_size, _inner_size);
        _inner->dtor(
            p_destruct_data,
            dst_mem->_size - expect_size
        );
    }

    // set size
    dst_mem->_size = expect_size;
}
void GenericVector::resize_zeroed(void* dst, uint64_t expect_size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    // realloc memory if need
    if (expect_size > dst_mem->capacity())
    {
        _realloc(dst, expect_size);
    }

    // construct item or destruct item if need
    if (expect_size > dst_mem->_size)
    {
        void* p_zero_data = dst_mem->_item_at(dst_mem->_size, _inner_size);
        std::memset(p_zero_data, 0, (expect_size - dst_mem->_size) * _inner_size);
    }
    else if (expect_size < dst_mem->_size)
    {
        void* p_destruct_data = dst_mem->_item_at(expect_size, _inner_size);
        _inner->dtor(
            p_destruct_data,
            dst_mem->_size - expect_size
        );
    }

    // set size
    dst_mem->_size = expect_size;
}

// add
GenericVectorDataRef GenericVector::add(void* dst, const void* v, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto result = add_unsafe(dst, n);
    for (uint64_t i = 0; i < n; ++i)
    {
        _inner->copy(
            result.offset(i, _inner_size),
            v
        );
    }
    return result;
}
GenericVectorDataRef GenericVector::add_move(void* dst, void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto result = add_unsafe(dst, 1);
    _inner->move(result.ptr, v, 1);
    return result;
}
GenericVectorDataRef GenericVector::add_unsafe(void* dst, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    auto old_size = _grow(dst, n);
    return {
        dst_mem->_item_at(old_size, _inner_size),
        old_size
    };
}
GenericVectorDataRef GenericVector::add_default(void* dst, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto result = add_unsafe(dst, n);
    _inner->default_ctor(
        result.ptr,
        n
    );
    return result;
}
GenericVectorDataRef GenericVector::add_zeroed(void* dst, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto result = add_unsafe(dst, n);
    ::skr::memory::zero_memory(result.ptr, n * _inner_size);
    return result;
}
GenericVectorDataRef GenericVector::add_unique(void* dst, const void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (auto ref = find(dst, v))
    {
        return ref;
    }
    else
    {
        return add(dst, v);
    }

    return {};
}
GenericVectorDataRef GenericVector::add_unique_move(void* dst, void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (auto ref = find(dst, v))
    {
        return ref;
    }
    else
    {
        return add_move(dst, v);
    }

    return {};
}

// add at
void GenericVector::add_at(void* dst, uint64_t idx, const void* v, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx, n);
    for (uint64_t i = 0; i < n; ++i)
    {
        _inner->copy(
            dst_mem->_item_at(idx + i, _inner_size),
            v
        );
    }
}
void GenericVector::add_at_move(void* dst, uint64_t idx, void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx, 1);
    for (uint64_t i = 0; i < dst_mem->_size; ++i)
    {
        _inner->move(
            dst_mem->_item_at(idx + i, _inner_size),
            v
        );
    }
}
void GenericVector::add_at_unsafe(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT((is_empty(dst) && idx == 0) || is_valid_index(dst, idx));
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    auto move_n = dst_mem->_size - idx;
    add_unsafe(dst, n);
    _inner->move(
        dst_mem->_item_at(idx + n, _inner_size),
        dst_mem->_item_at(idx, _inner_size),
        move_n
    );
}
void GenericVector::add_at_default(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx, n);
    _inner->default_ctor(
        dst_mem->_item_at(idx, _inner_size),
        n
    );
}
void GenericVector::add_at_zeroed(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    add_at_unsafe(dst, idx, n);
    ::skr::memory::zero_memory(
        dst_mem->_item_at(idx, _inner_size),
        n
    );
}

// append
GenericVectorDataRef GenericVector::append(void* dst, const void* other) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(other);
    auto*       dst_mem   = reinterpret_cast<VectorMemoryBase*>(dst);
    const auto* other_mem = reinterpret_cast<const VectorMemoryBase*>(other);

    auto old_size   = dst_mem->_size;
    auto other_size = other_mem->_size;
    if (other_size)
    {
        auto result = add_unsafe(dst, other_size);
        _inner->copy(
            result.ptr,
            other_mem->_data,
            other_size
        );
        return result;
    }
    return {
        dst_mem->_item_at(old_size, _inner_size),
        old_size
    };
}

// append at
void GenericVector::append_at(void* dst, uint64_t idx, const void* other) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(other);
    auto*       dst_mem   = reinterpret_cast<VectorMemoryBase*>(dst);
    const auto* other_mem = reinterpret_cast<const VectorMemoryBase*>(other);

    auto other_size = other_mem->_size;
    if (other_size)
    {
        add_at_unsafe(dst, idx, other_size);
        _inner->copy(
            dst_mem->_item_at(idx, _inner_size),
            other_mem->_data,
            other_size
        );
    }
}

// remove
void GenericVector::remove_at(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);
    SKR_ASSERT(is_valid_index(dst, idx) && (dst_mem->_size - idx >= n));

    if (n)
    {
        // calc move size
        auto move_n = dst_mem->_size - idx - n;

        // destruct remove items
        _inner->dtor(
            dst_mem->_item_at(idx, _inner_size),
            n
        );

        // move data
        if (move_n)
        {
            _inner->move(
                dst_mem->_item_at(idx, _inner_size),
                dst_mem->_item_at(idx + n, _inner_size),
                move_n
            );
        }

        // update size
        dst_mem->_size = dst_mem->_size - n;
    }
}
void GenericVector::remove_at_swap(void* dst, uint64_t idx, uint64_t n) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);
    SKR_ASSERT(is_valid_index(dst, idx) && (dst_mem->_size - idx >= n));

    if (n)
    {
        // calc move size
        auto move_n = std::min(dst_mem->_size - idx - n, n);

        // destruct remove items
        _inner->dtor(
            dst_mem->_item_at(idx, _inner_size),
            n
        );

        // move data
        if (move_n)
        {
            _inner->move(
                dst_mem->_item_at(idx, _inner_size),
                dst_mem->_item_at(dst_mem->_size - move_n, _inner_size),
                move_n
            );
        }

        // update size
        dst_mem->_size = dst_mem->_size - n;
    }
}
bool GenericVector::remove(void* dst, const void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    if (auto ref = find(dst, v))
    {
        remove_at(dst, ref.index);
        return true;
    }
    return false;
}
bool GenericVector::remove_swap(void* dst, const void* v) const
{
    return remove_swap_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
bool GenericVector::remove_last(void* dst, const void* v) const
{
    return remove_last_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
bool GenericVector::remove_last_swap(void* dst, const void* v) const
{
    return remove_last_swap_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
uint64_t GenericVector::remove_all(void* dst, const void* v) const
{
    return remove_all_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
uint64_t GenericVector::remove_all_swap(void* dst, const void* v) const
{
    return remove_all_swap_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}

// remove if
bool GenericVector::remove_if(void* dst, PredType pred) const
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
bool GenericVector::remove_swap_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    if (auto ref = find_if(dst, pred))
    {
        remove_at_swap(dst, ref.index);
        return true;
    }
    return false;
}
bool GenericVector::remove_last_if(void* dst, PredType pred) const
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
bool GenericVector::remove_last_swap_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    if (auto ref = find_last_if(dst, pred))
    {
        remove_at_swap(dst, ref.index);
        return true;
    }
    return false;
}
uint64_t GenericVector::remove_all_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (is_empty(dst))
    {
        return 0;
    }
    else
    {
        void* begin     = dst_mem->_data;
        void* end       = ::skr::memory::offset_item(begin, _inner_size, dst_mem->_size);
        void* write     = begin;
        void* read      = begin;
        bool  do_remove = pred(read);

        // remove loop
        do
        {
            auto run_start = read;
            read           = ::skr::memory::offset_item(read, _inner_size, 1);

            // collect run scope
            while (read < end && do_remove == pred(read))
            {
                read = ::skr::memory::offset_item(read, _inner_size, 1);
            }
            uint64_t run_len = ::skr::memory::distance_item(run_start, read, _inner_size);
            SKR_ASSERT(run_len > 0);

            // do scope op
            if (do_remove)
            {
                _inner->dtor(
                    run_start,
                    run_len
                );
            }
            else
            {
                if (write != run_start)
                {
                    _inner->move(
                        write,
                        run_start,
                        run_len
                    );
                }
                write = ::skr::memory::offset_item(write, _inner_size, run_len);
            }

            // update flag
            do_remove = !do_remove;
        } while (read < end);

        // get remove count
        uint64_t remove_count = ::skr::memory::distance_item(write, end, _inner_size);

        // update size
        if (remove_count)
        {
            dst_mem->_size = dst_mem->_size - remove_count;
        }
        return remove_count;
    }
}
uint64_t GenericVector::remove_all_swap_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (is_empty(dst))
    {
        return 0;
    }
    else
    {
        void* begin     = dst_mem->_data;
        void* end       = ::skr::memory::offset_item(begin, _inner_size, dst_mem->_size);
        void* cache_end = end;

        // for swap
        end = ::skr::memory::offset_item(end, _inner_size, -1);

        // remove loop
        while (true)
        {
            // skip items that needn't remove on header
            while (true)
            {
                if (begin > end)
                {
                    goto LoopEnd;
                }
                if (pred(begin))
                {
                    break;
                }
                begin = ::skr::memory::offset_item(begin, _inner_size, 1);
            }

            // skip items that needn't remove on tail
            while (true)
            {
                if (begin > end)
                {
                    goto LoopEnd;
                }
                if (!pred(end))
                {
                    break;
                }
                _inner->dtor(
                    end,
                    1
                );
                end = ::skr::memory::offset_item(end, _inner_size, -1);
            }

            // swap items at bad pos
            _inner->dtor(
                begin,
                1
            );
            _inner->move(
                begin,
                end,
                1
            );

            // update iterator
            begin = ::skr::memory::offset_item(begin, _inner_size, 1);
            end   = ::skr::memory::offset_item(end, _inner_size, -1);
        }
    LoopEnd:

        // get remove count
        uint64_t remove_count = ::skr::memory::distance_item(begin, cache_end, _inner_size);

        // update size
        if (remove_count)
        {
            dst_mem->_size = dst_mem->_size - remove_count;
        }
        return remove_count;
    }
}

// access
void* GenericVector::at(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(!is_empty(dst) && is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);
    return dst_mem->_item_at(idx, _inner_size);
}
void* GenericVector::at_last(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(!is_empty(dst) && is_valid_index(dst, idx));
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);
    return dst_mem->_item_at(dst_mem->_size - 1 - idx, _inner_size);
}
const void* GenericVector::at(const void* dst, uint64_t idx) const
{
    return at(const_cast<void*>(dst), idx);
}
const void* GenericVector::at_last(const void* dst, uint64_t idx) const
{
    return at_last(const_cast<void*>(dst), idx);
}

// find
GenericVectorDataRef GenericVector::find(void* dst, const void* v) const
{
    return find_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
GenericVectorDataRef GenericVector::find_last(void* dst, const void* v) const
{
    return find_last_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
CGenericVectorDataRef GenericVector::find(const void* dst, const void* v) const
{
    return find(const_cast<void*>(dst), v);
}
CGenericVectorDataRef GenericVector::find_last(const void* dst, const void* v) const
{
    return find_last(const_cast<void*>(dst), v);
}

// find if
GenericVectorDataRef GenericVector::find_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (is_empty(dst))
    {
        return {};
    }
    else
    {
        void* begin = dst_mem->_data;
        void* end   = ::skr::memory::offset_item(begin, _inner_size, dst_mem->_size);

        while (begin < end)
        {
            if (pred(begin))
            {
                return {
                    begin,
                    (uint64_t)::skr::memory::distance_item(
                        dst_mem->_data,
                        begin,
                        _inner_size
                    )
                };
            }
            begin = ::skr::memory::offset_item(begin, _inner_size, 1);
        }
        return {};
    }
}
GenericVectorDataRef GenericVector::find_last_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (is_empty(dst))
    {
        return {};
    }
    else
    {
        void* begin = dst_mem->_data;
        void* end   = ::skr::memory::offset_item(begin, _inner_size, dst_mem->_size);

        end = ::skr::memory::offset_item(end, _inner_size, -1);

        while (end >= begin)
        {
            if (pred(end))
            {
                return {
                    end,
                    (uint64_t)::skr::memory::distance_item(
                        dst_mem->_data,
                        end,
                        _inner_size
                    )
                };
            }
            end = ::skr::memory::offset_item(end, _inner_size, -1);
        }
        return {};
    }
}
CGenericVectorDataRef GenericVector::find_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}
CGenericVectorDataRef GenericVector::find_last_if(const void* dst, PredType pred) const
{
    return find_last_if(const_cast<void*>(dst), pred);
}

// contains & count
bool GenericVector::contains(const void* dst, const void* v) const
{
    return contains_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}
uint64_t GenericVector::count(const void* dst, const void* v) const
{
    return count_if(dst, [this, v](const void* item) {
        return _inner->equal(item, v);
    });
}

// contains if & count if
bool GenericVector::contains_if(const void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);

    return (bool)find_if(dst, pred);
}
uint64_t GenericVector::count_if(const void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(pred);
    auto* dst_mem = reinterpret_cast<const VectorMemoryBase*>(dst);

    if (is_empty(dst))
    {
        return 0;
    }
    else
    {
        uint64_t    count = 0;
        const void* begin = dst_mem->_data;
        const void* end   = ::skr::memory::offset_item(begin, _inner_size, dst_mem->_size);

        while (begin < end)
        {
            if (pred(begin))
            {
                ++count;
            }
            begin = ::skr::memory::offset_item(begin, _inner_size, 1);
        }
        return count;
    }
}

// helper
void GenericVector::_realloc(void* dst, uint64_t new_capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    SKR_ASSERT(new_capacity != dst_mem->capacity());
    SKR_ASSERT(new_capacity > 0);
    SKR_ASSERT(dst_mem->_size <= new_capacity);
    SKR_ASSERT((dst_mem->capacity() > 0 && dst_mem->_data != nullptr) || (dst_mem->capacity() == 0 && dst_mem->_data == nullptr));

    if (_inner_mem_traits.use_realloc)
    {
        dst_mem->_data = SkrAllocator::realloc_raw(
            dst_mem->_data,
            new_capacity,
            _inner_size,
            _inner_alignment
        );
    }
    else
    {
        // alloc new memory
        void* new_memory = SkrAllocator::alloc_raw(
            new_capacity,
            _inner_size,
            _inner_alignment
        );

        // move items
        if (dst_mem->_size)
        {
            _inner->move(
                new_memory,
                dst_mem->_data,
                dst_mem->_size
            );
        }

        // release old memory
        SkrAllocator::free_raw(
            dst_mem->_data,
            _inner_alignment
        );

        // update data
        dst_mem->_data = new_memory;
    }

    // update capacity
    dst_mem->_capacity = new_capacity;
}
void GenericVector::_free(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    if (dst_mem->_data)
    {
        SkrAllocator::free_raw(
            dst_mem->_data,
            _inner_alignment
        );
        dst_mem->_data     = nullptr;
        dst_mem->_capacity = 0;
    }
}
uint64_t GenericVector::_grow(void* dst, uint64_t grow_size) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto dst_mem = reinterpret_cast<VectorMemoryBase*>(dst);

    uint64_t old_size = dst_mem->_size;
    uint64_t new_size = old_size + grow_size;

    if (new_size > dst_mem->capacity())
    {
        uint64_t new_capacity = container::default_get_grow<uint64_t>(
            new_size,
            dst_mem->capacity()
        );
        SKR_ASSERT(new_capacity >= dst_mem->capacity());
        if (new_capacity >= dst_mem->capacity())
        {
            _realloc(dst, new_capacity);
        }
    }

    dst_mem->_size = new_size;
    return old_size;
}
} // namespace skr
