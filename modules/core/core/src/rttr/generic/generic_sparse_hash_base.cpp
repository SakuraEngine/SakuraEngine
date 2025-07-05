#include "SkrContainers/set.hpp"
#include <SkrRTTR/generic/generic_sparase_hash_base.hpp>

namespace skr
{
// ctor & dtor
GenericSparseHashSetStorage::GenericSparseHashSetStorage(RC<IGenericBase> inner)
    : _inner(std::move(inner))
{
    if (_inner)
    {
        _inner_mem_traits = _inner->memory_traits_data();
        _inner_size       = _inner->size();
        _inner_alignment  = _inner->alignment();
    }
}
GenericSparseHashSetStorage::~GenericSparseHashSetStorage() = default;

//===> IGenericBase API
// get type info
EGenericKind GenericSparseHashSetStorage::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericSparseHashSetStorage::id() const
{
    return kSparseHashSetStorageGenericId;
}

// get utils
MemoryTraitsData GenericSparseHashSetStorage::memory_traits_data() const
{
    SKR_ASSERT(is_valid());
    return _inner_mem_traits;
}

// basic info
uint64_t GenericSparseHashSetStorage::size() const
{
    SKR_ASSERT(is_valid());
    auto alignment         = std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
    auto padded_inner_size = ::skr::memory::padded_size(_inner_size, alignment); // align inner size
    return padded_inner_size + sizeof(uint64_t) * 2 /* hash + next */;
}
uint64_t GenericSparseHashSetStorage::alignment() const
{
    SKR_ASSERT(is_valid());
    return std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
}

// operations, used for generic container algorithms
bool GenericSparseHashSetStorage::support(EGenericFeature feature) const
{
    SKR_ASSERT(is_valid());

    switch (feature)
    {
    case EGenericFeature::DefaultCtor:
    case EGenericFeature::Dtor:
    case EGenericFeature::Copy:
    case EGenericFeature::Move:
    case EGenericFeature::Assign:
    case EGenericFeature::MoveAssign:
    case EGenericFeature::Swap:
        return _inner->support(feature);
    case EGenericFeature::Equal:
    case EGenericFeature::Hash:
        return false;
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
void GenericSparseHashSetStorage::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_ctor)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);

            // call ctor
            _inner->default_ctor(p_dst, 1);

            // set hash and next
            item_hash(p_dst) = 0; // hash
            next(p_dst)      = 0; // next
        }
    }
    else
    {
        ::skr::memory::zero_memory(dst, storage_size * count);
    }
}
void GenericSparseHashSetStorage::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_dtor)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);

            // call dtor
            _inner->dtor(p_dst, 1);

            // reset hash and next
            item_hash(p_dst) = 0; // hash
            next(p_dst)      = 0; // next
        }
    }
    else
    {
        ::skr::memory::zero_memory(dst, storage_size * count);
    }
}
void GenericSparseHashSetStorage::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_copy)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);
            auto p_src = ::skr::memory::offset_bytes(src, storage_size);

            // copy item data
            _inner->copy(item_data(p_dst), item_data(p_src), 1);

            // copy hash and next
            item_hash(p_dst) = item_hash(p_src);
            next(p_dst)      = next(p_src);
        }
    }
    else
    {
        ::std::memcpy(dst, src, storage_size * count);
    }
}
void GenericSparseHashSetStorage::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_move)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);
            auto p_src = ::skr::memory::offset_bytes(src, storage_size);

            // move item data
            _inner->move(item_data(p_dst), item_data(p_src), 1);

            // move hash and next
            item_hash(p_dst) = item_hash(p_src);
            next(p_dst)      = next(p_src);

            // reset src
            item_hash(p_src) = 0;
            next(p_src)      = 0;
        }
    }
    else
    {
        ::std::memmove(dst, src, storage_size * count);
        ::skr::memory::zero_memory(src, storage_size * count);
    }
}
void GenericSparseHashSetStorage::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_assign)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);
            auto p_src = ::skr::memory::offset_bytes(src, storage_size);

            // assign item data
            _inner->assign(item_data(p_dst), item_data(p_src), 1);

            // assign hash and next
            item_hash(p_dst) = item_hash(p_src);
            next(p_dst)      = next(p_src);
        }
    }
    else
    {
        ::std::memcpy(dst, src, storage_size * count);
    }
}
void GenericSparseHashSetStorage::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    if (_inner_mem_traits.use_move_assign)
    {
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);
            auto p_src = ::skr::memory::offset_bytes(src, storage_size);

            // move assign item data
            _inner->move_assign(item_data(p_dst), item_data(p_src), 1);

            // move assign hash and next
            item_hash(p_dst) = item_hash(p_src);
            next(p_dst)      = next(p_src);

            // reset src
            item_hash(p_src) = 0;
            next(p_src)      = 0;
        }
    }
    else
    {
        ::std::memmove(dst, src, storage_size * count);
        ::skr::memory::zero_memory(src, storage_size * count);
    }
}
bool GenericSparseHashSetStorage::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(count > 0);

    SKR_ASSERT(false && "equal not support for GenericOptional, please check feature before call this function");
    return false;
}
size_t GenericSparseHashSetStorage::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(false && "hash not support for GenericOptional, please check feature before call this function");
    return 0;
}
void GenericSparseHashSetStorage::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto storage_size = size();

    for (uint64_t i = 0; i < count; ++i)
    {
        auto p_dst = ::skr::memory::offset_bytes(dst, storage_size);
        auto p_src = ::skr::memory::offset_bytes(src, storage_size);

        // swap item data
        _inner->swap(item_data(p_dst), item_data(p_src), 1);

        // swap hash
        auto temp_hash   = item_hash(p_dst);
        item_hash(p_dst) = item_hash(p_src);
        item_hash(p_src) = temp_hash;

        // swap next
        auto temp_next = next(p_dst);
        next(p_dst)    = next(p_src);
        next(p_src)    = temp_next;
    }
}
//===> IGenericBase API

// getter
bool             GenericSparseHashSetStorage::is_valid() const { return _inner != nullptr; }
RC<IGenericBase> GenericSparseHashSetStorage::inner() const { return _inner; }

// item getter
void* GenericSparseHashSetStorage::item_data(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return dst; // item data is at the beginning of the storage
}
const void* GenericSparseHashSetStorage::item_data(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    return dst; // item data is at the beginning of the storage
}
size_t GenericSparseHashSetStorage::item_hash(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto alignment         = std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
    auto padded_inner_size = ::skr::memory::padded_size(_inner_size, alignment); // align inner size
    auto hash_ptr          = ::skr::memory::offset_bytes(dst, padded_inner_size);
    return *static_cast<const size_t*>(hash_ptr);
}
size_t& GenericSparseHashSetStorage::item_hash(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto alignment         = std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
    auto padded_inner_size = ::skr::memory::padded_size(_inner_size, alignment); // align inner size
    auto hash_ptr          = ::skr::memory::offset_bytes(dst, padded_inner_size);
    return *static_cast<size_t*>(hash_ptr);
}
uint64_t GenericSparseHashSetStorage::next(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto alignment         = std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
    auto padded_inner_size = ::skr::memory::padded_size(_inner_size, alignment); // align inner size
    auto next_ptr          = ::skr::memory::offset_bytes(dst, padded_inner_size + sizeof(size_t));
    return *static_cast<const uint64_t*>(next_ptr);
}
uint64_t& GenericSparseHashSetStorage::next(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto alignment         = std::max(_inner_alignment, (uint64_t)alignof(uint64_t));
    auto padded_inner_size = ::skr::memory::padded_size(_inner_size, alignment); // align inner size
    auto next_ptr          = ::skr::memory::offset_bytes(dst, padded_inner_size + sizeof(size_t));
    return *static_cast<uint64_t*>(next_ptr);
}

// ctor & dtor
GenericSparseHashBase::GenericSparseHashBase(RC<IGenericBase> inner)
    : GenericSparseVector(RC<GenericSparseHashSetStorage>::New(inner))
    , _inner(inner)
{
    _inner_storage = GenericSparseVector::inner().cast_static<GenericSparseHashSetStorage>();
    if (_inner)
    {
        _inner_mem_traits = _inner->memory_traits_data();
        _inner_size       = _inner->size();
        _inner_alignment  = _inner->alignment();
    }
}
GenericSparseHashBase::~GenericSparseHashBase() = default;

//===> IGenericBase API
// get type info
EGenericKind GenericSparseHashBase::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericSparseHashBase::id() const
{
    //! NOTE. should be override by derived class
    SKR_UNREACHABLE_CODE();
    return {};
}

// get utils
MemoryTraitsData GenericSparseHashBase::memory_traits_data() const
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
uint64_t GenericSparseHashBase::size() const
{
    return sizeof(SetMemoryBase);
}
uint64_t GenericSparseHashBase::alignment() const
{
    return alignof(SetMemoryBase);
}

// operations, used for generic container algorithms
bool GenericSparseHashBase::support(EGenericFeature feature) const
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
void GenericSparseHashBase::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        dst_mem->_reset();
    }
}
void GenericSparseHashBase::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* p_dst = ::skr::memory::offset_item(
            dst,
            sizeof(SetMemoryBase),
            i
        );

        // call super
        Super::dtor(p_dst);

        // free bucket
        _free_bucket(p_dst);
    }
}
void GenericSparseHashBase::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        const auto* src_mem = reinterpret_cast<const SetMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SetMemoryBase), i)
        );

        // copy sparse vector
        Super::copy(dst_mem, src_mem);

        // build bucket
        _resize_bucket(dst_mem);
        _clean_bucket(dst_mem);
        _build_bucket(dst_mem);
    }
}
void GenericSparseHashBase::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SetMemoryBase), i)
        );

        // move data
        *dst_mem = std::move(*src_mem);

        // reset src
        src_mem->_reset();
    }
}
void GenericSparseHashBase::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        const auto* src_mem = reinterpret_cast<const SetMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SetMemoryBase), i)
        );

        // assign sparse vector
        Super::assign(dst_mem, src_mem);

        // build bucket
        _resize_bucket(dst_mem);
        _clean_bucket(dst_mem);
        _build_bucket(dst_mem);
    }
}
void GenericSparseHashBase::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SetMemoryBase), i)
        );

        // move assign sparse vector
        Super::move_assign(dst_mem, src_mem);

        // clean up bucket
        _free_bucket(dst_mem);

        // move bucket data
        dst_mem->_bucket      = src_mem->_bucket;
        dst_mem->_bucket_size = src_mem->_bucket_size;
        dst_mem->_bucket_mask = src_mem->_bucket_mask;

        // reset rhs
        src_mem->_reset();
    }
}
bool GenericSparseHashBase::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_UNREACHABLE_CODE(); // should impl by child
    return false;
}
size_t GenericSparseHashBase::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(src);
    SKR_ASSERT(false && "GenericSparseVector does not support hash operation, please check feature first");
    return 0;
}
void GenericSparseHashBase::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    for (uint64_t i = 0; i < count; ++i)
    {
        auto* dst_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(dst, sizeof(SetMemoryBase), i)
        );
        auto* src_mem = reinterpret_cast<SetMemoryBase*>(
            ::skr::memory::offset_item(src, sizeof(SetMemoryBase), i)
        );

        // swap data
        SetMemoryBase tmp = std::move(*dst_mem);
        *dst_mem          = std::move(*src_mem);
        *src_mem          = std::move(tmp);
    }
}
//===> IGenericBase API

// getter
bool GenericSparseHashBase::is_valid() const
{
    return _inner != nullptr;
}
RC<IGenericBase> GenericSparseHashBase::inner() const
{
    SKR_ASSERT(is_valid());
    return _inner;
}
RC<GenericSparseHashSetStorage> GenericSparseHashBase::inner_storage() const
{
    SKR_ASSERT(is_valid());
    return _inner_storage;
}

// sparse hash set getter
uint64_t* GenericSparseHashBase::bucket(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);
    return dst_mem->_bucket; // bucket is at the beginning of the storage
}
const uint64_t* GenericSparseHashBase::bucket(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);
    return dst_mem->_bucket; // bucket is at the beginning of the storage
}
uint64_t GenericSparseHashBase::bucket_size(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);
    return dst_mem->_bucket_size;
}
uint64_t& GenericSparseHashBase::bucket_size(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);
    return dst_mem->_bucket_size;
}
uint64_t GenericSparseHashBase::bucket_mask(const void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);
    return dst_mem->_bucket_mask;
}
uint64_t& GenericSparseHashBase::bucket_mask(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);
    return dst_mem->_bucket_mask;
}

// memory op
void GenericSparseHashBase::clear(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    Super::clear(dst);
    _clean_bucket(dst);
}
void GenericSparseHashBase::release(void* dst, uint64_t capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    Super::release(dst, capacity);
    _resize_bucket(dst);
    _clean_bucket(dst);
}
void GenericSparseHashBase::reserve(void* dst, uint64_t capacity) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    Super::reserve(dst, capacity);
    rehash_if_need(dst);
}
void GenericSparseHashBase::shrink(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    Super::shrink(dst);
    rehash_if_need(dst);
}
bool GenericSparseHashBase::compact(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (Super::compact(dst))
    {
        rehash(dst);
        return true;
    }
    else
    {
        return false;
    }
}
bool GenericSparseHashBase::compact_stable(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (Super::compact_stable(dst))
    {
        rehash(dst);
        return true;
    }
    else
    {
        return false;
    }
}
bool GenericSparseHashBase::compact_top(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    return Super::compact_top(dst);
}

// rehash
void GenericSparseHashBase::rehash(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    _resize_bucket(dst);
    _clean_bucket(dst);
    _build_bucket(dst);
}
bool GenericSparseHashBase::rehash_if_need(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (_resize_bucket(dst))
    {
        _clean_bucket(dst);
        _build_bucket(dst);
        return true;
    }
    return false;
}

// visitor
const void* GenericSparseHashBase::at(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    return _inner_storage->item_data(Super::at(dst, idx));
}
const void* GenericSparseHashBase::at_last(const void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    return _inner_storage->item_data(Super::at_last(dst, idx));
}

// remove
void GenericSparseHashBase::remove_at(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    _remove_from_bucket(dst, idx);
    Super::remove_at(dst, idx);
}
void GenericSparseHashBase::remove_at_unsafe(void* dst, uint64_t idx) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    _remove_from_bucket(dst, idx);
    Super::remove_at_unsafe(dst, idx);
}

// basic add/find/remove
GenericSparseHashSetDataRef GenericSparseHashBase::add_unsafe(void* dst, size_t hash) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto data_arr_ref                           = Super::add_unsafe(dst);
    _inner_storage->item_hash(data_arr_ref.ptr) = hash;
    _add_to_bucket(dst, data_arr_ref.ptr, data_arr_ref.index);
    return {
        _inner_storage->item_data(data_arr_ref.ptr),
        data_arr_ref.index,
        hash,
        false
    };
}
GenericSparseHashSetDataRef GenericSparseHashBase::find(const void* dst, size_t hash, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);

    if (!dst_mem->_bucket) return {};

    uint64_t search_index = dst_mem->_bucket[_bucket_index(dst_mem, hash)];
    while (search_index != npos)
    {
        auto* node      = Super::at(dst, search_index);
        auto  node_hash = _inner_storage->item_hash(node);
        auto* node_data = _inner_storage->item_data(node);
        if (node_hash == hash && pred(node_data))
        {
            return {
                const_cast<void*>(node_data),
                search_index,
                hash,
                false
            };
        }
        search_index = _inner_storage->next(node);
    }
    return {};
}
GenericSparseHashSetDataRef GenericSparseHashBase::find_next(const void* dst, GenericSparseHashSetDataRef ref, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);

    if (!dst_mem->_bucket || !ref.is_valid()) return {};

    uint64_t search_index = _inner_storage->next(Super::at(dst, ref.index));
    while (search_index != npos)
    {
        auto* node      = Super::at(dst, search_index);
        auto  node_hash = _inner_storage->item_hash(node);
        auto* node_data = _inner_storage->item_data(node);
        if (node_hash == ref.hash && pred(node_data))
        {
            return {
                const_cast<void*>(node_data),
                search_index,
                node_hash,
                false
            };
        }
        search_index = _inner_storage->next(node);
    }

    return {};
}
bool GenericSparseHashBase::remove(void* dst, size_t hash, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    if (!dst_mem->_bucket) return false;

    uint64_t* p_next_idx = dst_mem->_bucket + _bucket_index(dst_mem, hash);
    while (*p_next_idx != npos)
    {
        auto* next_node      = Super::at(dst, *p_next_idx);
        auto  next_node_hash = _inner_storage->item_hash(next_node);
        auto* next_node_data = _inner_storage->item_data(next_node);

        if (next_node_hash == hash && pred(next_node_data))
        {
            // update link
            uint64_t index = *p_next_idx;
            *p_next_idx    = _inner_storage->next(next_node);

            // remove item
            Super::remove_at(dst, index);
            return true;
        }
        else
        {
            p_next_idx = &_inner_storage->next(next_node);
        }
    }
    return false;
}
uint64_t GenericSparseHashBase::remove_all(void* dst, size_t hash, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    if (is_empty(dst)) return 0;

    uint64_t count = 0;

    uint64_t* p_next_idx = dst_mem->_bucket + _bucket_index(dst_mem, hash);

    while (*p_next_idx != npos)
    {
        auto* next_node      = Super::at(dst, *p_next_idx);
        auto  next_node_hash = _inner_storage->item_hash(next_node);
        auto* next_node_data = _inner_storage->item_data(next_node);

        if (next_node_hash == hash && pred(next_node_data))
        {
            // update link
            uint64_t index = *p_next_idx;
            *p_next_idx    = _inner_storage->next(next_node);

            // remove item
            Super::remove_at(dst, index);
            ++count;
        }
        else
        {
            // move to next node
            p_next_idx = &_inner_storage->next(next_node);
        }
    }

    return count;
}

// find if
GenericSparseHashSetDataRef GenericSparseHashBase::find_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (auto ref = Super::find_if(dst, [this, pred](const void* node) {
            return pred(_inner_storage->item_data(node));
        }))
    {
        return {
            _inner_storage->item_data(ref.ptr),
            ref.index,
            _inner_storage->item_hash(ref.ptr),
            false
        };
    }
    else
    {
        return {};
    }
}
GenericSparseHashSetDataRef GenericSparseHashBase::find_last_if(void* dst, PredType pred) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    if (auto ref = Super::find_last_if(dst, [this, pred](const void* node) {
            return pred(_inner_storage->item_data(node));
        }))
    {
        return {
            _inner_storage->item_data(ref.ptr),
            ref.index,
            _inner_storage->item_hash(ref.ptr),
            false
        };
    }
    else
    {
        return {};
    }
}

// helper
void GenericSparseHashBase::_free_bucket(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    if (dst_mem->_bucket)
    {
        SkrAllocator::free_raw(dst_mem->_bucket, alignof(uint64_t));
        dst_mem->_bucket      = nullptr;
        dst_mem->_bucket_size = 0;
        dst_mem->_bucket_mask = 0;
    }
}
bool GenericSparseHashBase::_resize_bucket(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    uint64_t new_bucket_size = container::sparse_hash_set_calc_bucket_size(dst_mem->_capacity);
    if (new_bucket_size != dst_mem->_bucket_size)
    {
        if (new_bucket_size)
        {
            if constexpr (::skr::memory::MemoryTraits<uint64_t>::use_realloc)
            {
                dst_mem->_bucket = (uint64_t*)SkrAllocator::realloc_raw(
                    dst_mem->_bucket,
                    new_bucket_size,
                    sizeof(uint64_t),
                    alignof(uint64_t)
                );
            }
            else
            {
                // alloc new memory
                uint64_t* new_memory = (uint64_t*)SkrAllocator::alloc_raw(
                    new_bucket_size,
                    sizeof(uint64_t),
                    alignof(uint64_t)
                );

                // release old memory
                SkrAllocator::free_raw(dst_mem->_bucket, alignof(uint64_t));

                dst_mem->_bucket = new_memory;
            }

            dst_mem->_bucket_size = new_bucket_size;
            dst_mem->_bucket_mask = new_bucket_size - 1;
        }
        else
        {
            _free_bucket(dst);
        }
        return true;
    }
    return false;
}
void GenericSparseHashBase::_clean_bucket(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    container::sparse_hash_set_clean_bucket(
        dst_mem->_bucket,
        dst_mem->_bucket_size
    );
}
void GenericSparseHashBase::_build_bucket(void* dst) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);

    if (Super::sparse_size(dst) - Super::hole_size(dst))
    {
        auto bit_cursor = Super::TrueBitCursor::Begin(
            Super::bit_data(dst),
            Super::sparse_size(dst)
        );

        while (!bit_cursor.reach_end())
        {
            void*     p_data       = Super::at(dst, bit_cursor.index());
            uint64_t* p_bucket_idx = dst_mem->_bucket +
                                     _bucket_index(
                                         dst_mem,
                                         _inner_storage->item_hash(p_data)
                                     );

            // link bucket node
            _inner_storage->next(p_data) = *p_bucket_idx;

            // replace bucket node
            *p_bucket_idx = bit_cursor.index();

            bit_cursor.move_next();
        }
    }
}
uint64_t GenericSparseHashBase::_bucket_index(const void* dst, uint64_t hash) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    const auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);

    // use hash to find bucket index
    return hash & dst_mem->_bucket_mask;
}
bool GenericSparseHashBase::_is_in_bucket(const void* dst, uint64_t index) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    auto* dst_mem = reinterpret_cast<const SetMemoryBase*>(dst);

    if (Super::has_data(dst, index))
    {
        const auto& node         = Super::at(dst, index);
        auto        node_hash    = _inner_storage->item_hash(node);
        auto        search_index = dst_mem->_bucket[_bucket_index(dst, node_hash)];

        while (search_index != npos)
        {
            if (search_index == index)
            {
                return true;
            }
            search_index = _inner_storage->next(Super::at(dst, search_index));
        }
        return false;
    }
    else
    {
        return false;
    }
}
void GenericSparseHashBase::_add_to_bucket(void* dst, void* item_data, uint64_t index) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(item_data);

    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);
    SKR_ASSERT(Super::has_data(dst, index));
    SKR_ASSERT(!dst_mem->_bucket || !_is_in_bucket(dst, index));

    if (!rehash_if_need(dst))
    {
        auto      item_hash             = _inner_storage->item_hash(item_data);
        uint64_t* p_bucket_idx          = dst_mem->_bucket + _bucket_index(dst_mem, item_hash);
        _inner_storage->next(item_data) = *p_bucket_idx;
        *p_bucket_idx                   = index; // link bucket node
    }
}
void GenericSparseHashBase::_remove_from_bucket(void* dst, uint64_t index) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto* dst_mem = reinterpret_cast<SetMemoryBase*>(dst);
    SKR_ASSERT(Super::has_data(dst, index));
    SKR_ASSERT(dst_mem->_bucket);

    auto*     p_in_node  = Super::at(dst, index);
    uint64_t* p_next_idx = dst_mem->_bucket +
                           _bucket_index(
                               dst,
                               _inner_storage->item_hash(p_in_node)
                           );

    while (*p_next_idx != npos)
    {
        auto* next_node = Super::at(dst, *p_next_idx);

        if (*p_next_idx == index)
        {
            // update link
            *p_next_idx = _inner_storage->next(next_node);
            break;
        }
        else
        {
            // move to next node
            p_next_idx = &_inner_storage->next(next_node);
        }
    }
}

} // namespace skr