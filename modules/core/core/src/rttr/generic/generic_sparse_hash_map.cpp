#include <SkrRTTR/generic/generic_sparse_hash_map.hpp>

namespace skr
{
// ctor & dtor
GenericKVPair::GenericKVPair(RC<IGenericBase> inner_key, RC<IGenericBase> inner_value)
    : _inner_key(std::move(inner_key))
    , _inner_value(std::move(inner_value))
{
    if (_inner_key)
    {
        _inner_key_mem_traits = _inner_key->memory_traits_data();
        _inner_key_size       = _inner_key->size();
        _inner_key_alignment  = _inner_key->alignment();
    }

    if (_inner_value)
    {
        _inner_value_mem_traits = _inner_value->memory_traits_data();
        _inner_value_size       = _inner_value->size();
        _inner_value_alignment  = _inner_value->alignment();
    }
}
GenericKVPair::~GenericKVPair() = default;

// get type info
EGenericKind GenericKVPair::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericKVPair::id() const
{
    return kKVPairGenericId;
}

// get utils
MemoryTraitsData GenericKVPair::memory_traits_data() const
{
    SKR_ASSERT(is_valid());

    MemoryTraitsData data;
    data.use_ctor             = _inner_key_mem_traits.use_ctor || _inner_value_mem_traits.use_ctor;
    data.use_dtor             = _inner_key_mem_traits.use_dtor || _inner_value_mem_traits.use_dtor;
    data.use_move             = _inner_key_mem_traits.use_move || _inner_value_mem_traits.use_move;
    data.use_assign           = _inner_key_mem_traits.use_assign || _inner_value_mem_traits.use_assign;
    data.use_move_assign      = _inner_key_mem_traits.use_move_assign || _inner_value_mem_traits.use_move_assign;
    data.need_dtor_after_move = _inner_key_mem_traits.need_dtor_after_move || _inner_value_mem_traits.need_dtor_after_move;
    data.use_realloc          = _inner_key_mem_traits.use_realloc && _inner_value_mem_traits.use_realloc;
    data.use_compare          = _inner_key_mem_traits.use_compare || _inner_value_mem_traits.use_compare;
    return data;
}

// basic info
uint64_t GenericKVPair::size() const
{
    auto alignment = std::max(_inner_key_alignment, _inner_value_alignment);
    return ::skr::memory::padded_size(_inner_key_size, alignment) + ::skr::memory::padded_size(_inner_value_size, alignment);
}
uint64_t GenericKVPair::alignment() const
{
    return std::max(_inner_key_alignment, _inner_value_alignment);
}

// operations, used for generic container algorithms
bool GenericKVPair::support(EGenericFeature feature) const
{
    switch (feature)
    {
    case EGenericFeature::DefaultCtor:
    case EGenericFeature::Dtor:
    case EGenericFeature::Copy:
    case EGenericFeature::Move:
    case EGenericFeature::Assign:
    case EGenericFeature::MoveAssign:
    case EGenericFeature::Equal:
    case EGenericFeature::Swap:
        return _inner_key->support(feature) && _inner_value->support(feature);
    case EGenericFeature::Hash:
        return _inner_key->support(feature);
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
void GenericKVPair::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    for (uint64_t i = 0; i < count; ++i)
    {
        auto p_dst = ::skr::memory::offset_item(dst, size, i);
        _inner_key->default_ctor(key(p_dst));
        _inner_value->default_ctor(value(p_dst));
    }
}
void GenericKVPair::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    for (uint64_t i = 0; i < count; ++i)
    {
        auto p_dst = ::skr::memory::offset_item(dst, size, i);
        _inner_key->dtor(key(p_dst));
        _inner_value->dtor(value(p_dst));
    }
}
void GenericKVPair::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    if (_inner_key_mem_traits.use_copy && _inner_value_mem_traits.use_copy)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, size, i);
            auto p_src = ::skr::memory::offset_item(src, size, i);
            _inner_key->copy(key(p_dst), key(p_src));
            _inner_value->copy(value(p_dst), value(p_src));
        }
    }
    else
    {
        ::std::memcpy(dst, src, size * count);
    }
}
void GenericKVPair::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    if (_inner_key_mem_traits.use_move && _inner_value_mem_traits.use_move)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, size, i);
            auto p_src = ::skr::memory::offset_item(src, size, i);
            _inner_key->move(key(p_dst), key(p_src));
            _inner_value->move(value(p_dst), value(p_src));
        }
    }
    else
    {
        ::std::memcpy(dst, src, size * count);
    }
}
void GenericKVPair::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    if (_inner_key_mem_traits.use_assign && _inner_value_mem_traits.use_assign)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, size, i);
            auto p_src = ::skr::memory::offset_item(src, size, i);
            _inner_key->assign(key(p_dst), key(p_src));
            _inner_value->assign(value(p_dst), value(p_src));
        }
    }
    else
    {
        ::std::memcpy(dst, src, size * count);
    }
}
void GenericKVPair::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    if (_inner_key_mem_traits.use_move_assign && _inner_value_mem_traits.use_move_assign)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, size, i);
            auto p_src = ::skr::memory::offset_item(src, size, i);
            _inner_key->move_assign(key(p_dst), key(p_src));
            _inner_value->move_assign(value(p_dst), value(p_src));
        }
    }
    else
    {
        ::std::memcpy(dst, src, size * count);
        ::skr::memory::zero_memory(src, size * count); // reset src
    }
}
bool GenericKVPair::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(count > 0);

    if (_inner_key_mem_traits.use_compare && _inner_value_mem_traits.use_compare)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_lhs = ::skr::memory::offset_item(lhs, size(), i);
            auto p_rhs = ::skr::memory::offset_item(rhs, size(), i);
            if (!_inner_key->equal(key(p_lhs), key(p_rhs)) ||
                !_inner_value->equal(value(p_lhs), value(p_rhs)))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        SKR_ASSERT(false && "equal not support for GenericKVPair, please check feature before call this function");
        return false;
    }
}
size_t GenericKVPair::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(src);
    SKR_ASSERT(_inner_key->support(EGenericFeature::Hash));

    // hash key
    return _inner_key->hash(key(src));
}
void GenericKVPair::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto size = this->size();

    for (uint64_t i = 0; i < count; ++i)
    {
        auto p_dst = ::skr::memory::offset_item(dst, size, i);
        auto p_src = ::skr::memory::offset_item(src, size, i);
        _inner_key->swap(key(p_dst), key(p_src));
        _inner_value->swap(value(p_dst), value(p_src));
    }
}

// getter
bool GenericKVPair::is_valid() const
{
    return _inner_key && _inner_value;
}
RC<IGenericBase> GenericKVPair::inner_key() const
{
    SKR_ASSERT(is_valid());
    return _inner_key;
}
RC<IGenericBase> GenericKVPair::inner_value() const
{
    SKR_ASSERT(is_valid());
    return _inner_value;
}

// get key & value
void* GenericKVPair::key(void* dst) const
{
    return dst;
}
void* GenericKVPair::value(void* dst) const
{
    auto alignment       = std::max(_inner_key_alignment, _inner_value_alignment);
    auto padded_key_size = ::skr::memory::padded_size(_inner_key_size, alignment);
    return ::skr::memory::offset_bytes(dst, padded_key_size);
}
const void* GenericKVPair::key(const void* dst) const
{
    return key(const_cast<void*>(dst));
}
const void* GenericKVPair::value(const void* dst) const
{
    return value(const_cast<void*>(dst));
}

// ctor & dtor
GenericSparseHashMap::GenericSparseHashMap(RC<IGenericBase> inner_key, RC<IGenericBase> inner_value)
    : GenericSparseHashBase(RC<GenericKVPair>::New(std::move(inner_key), std::move(inner_value)))
{
    _inner_kv_pair = GenericSparseHashBase::inner().cast_static<GenericKVPair>();
}
GenericSparseHashMap::~GenericSparseHashMap() = default;

// get type info
EGenericKind GenericSparseHashMap::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericSparseHashMap::id() const
{
    return kMapGenericId;
}

// get utils
MemoryTraitsData GenericSparseHashMap::memory_traits_data() const
{
    return Super::memory_traits_data();
}

// basic info
uint64_t GenericSparseHashMap::size() const
{
    return sizeof(MapMemoryBase);
}
uint64_t GenericSparseHashMap::alignment() const
{
    return alignof(MapMemoryBase);
}

// operations, used for generic container algorithms
bool GenericSparseHashMap::support(EGenericFeature feature) const
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
        return _inner && _inner->support(feature);
    case EGenericFeature::Equal:
        return false;
    case EGenericFeature::Hash:
        return false;
    case EGenericFeature::Swap:
        return true;
    default:
        SKR_UNREACHABLE_CODE()
        return false;
    }
}
void GenericSparseHashMap::default_ctor(void* dst, uint64_t count) const
{
    Super::default_ctor(dst, count);
}
void GenericSparseHashMap::dtor(void* dst, uint64_t count) const
{
    Super::dtor(dst, count);
}
void GenericSparseHashMap::copy(void* dst, const void* src, uint64_t count) const
{
    Super::copy(dst, src, count);
}
void GenericSparseHashMap::move(void* dst, void* src, uint64_t count) const
{
    Super::move(dst, src, count);
}
void GenericSparseHashMap::assign(void* dst, const void* src, uint64_t count) const
{
    Super::assign(dst, src, count);
}
void GenericSparseHashMap::move_assign(void* dst, void* src, uint64_t count) const
{
    Super::move_assign(dst, src, count);
}
bool GenericSparseHashMap::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(false && "GenericSparseHashSet not support equal operation, please check feature first");
    return 0;
}
size_t GenericSparseHashMap::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(src);
    SKR_ASSERT(false && "GenericSparseHashSet does not support hash operation, please check feature first");
    return 0;
}
void GenericSparseHashMap::swap(void* dst, void* src, uint64_t count) const
{
    Super::swap(dst, src, count);
}

// getter
bool GenericSparseHashMap::is_valid() const
{
    return Super::is_valid();
}
RC<IGenericBase> GenericSparseHashMap::inner_key() const
{
    SKR_ASSERT(is_valid());
    return _inner_kv_pair->inner_key();
}
RC<IGenericBase> GenericSparseHashMap::inner_value() const
{
    SKR_ASSERT(is_valid());
    return _inner_kv_pair->inner_value();
}
RC<GenericKVPair> GenericSparseHashMap::inner_kv_pair() const
{
    SKR_ASSERT(is_valid());
    return _inner_kv_pair;
}

// sparse hash map getter
uint64_t GenericSparseHashMap::size(const void* dst) const
{
    return Super::size(dst);
}
uint64_t GenericSparseHashMap::capacity(const void* dst) const
{
    return Super::capacity(dst);
}
uint64_t GenericSparseHashMap::slack(const void* dst) const
{
    return Super::slack(dst);
}
uint64_t GenericSparseHashMap::sparse_size(const void* dst) const
{
    return Super::sparse_size(dst);
}
uint64_t GenericSparseHashMap::hole_size(const void* dst) const
{
    return Super::hole_size(dst);
}
uint64_t GenericSparseHashMap::bit_size(const void* dst) const
{
    return Super::bit_size(dst);
}
uint64_t GenericSparseHashMap::freelist_head(const void* dst) const
{
    return Super::freelist_head(dst);
}
bool GenericSparseHashMap::is_compact(const void* dst) const
{
    return Super::is_compact(dst);
}
bool GenericSparseHashMap::is_empty(const void* dst) const
{
    return Super::is_empty(dst);
}
void* GenericSparseHashMap::storage(void* dst) const
{
    return Super::storage(dst);
}
const void* GenericSparseHashMap::storage(const void* dst) const
{
    return Super::storage(dst);
}
uint64_t* GenericSparseHashMap::bit_data(void* dst) const
{
    return Super::bit_data(dst);
}
const uint64_t* GenericSparseHashMap::bit_data(const void* dst) const
{
    return Super::bit_data(dst);
}
uint64_t* GenericSparseHashMap::bucket(void* dst) const
{
    return Super::bucket(dst);
}
const uint64_t* GenericSparseHashMap::bucket(const void* dst) const
{
    return Super::bucket(dst);
}
uint64_t GenericSparseHashMap::bucket_size(const void* dst) const
{
    return Super::bucket_size(dst);
}
uint64_t& GenericSparseHashMap::bucket_size(void* dst) const
{
    return Super::bucket_size(dst);
}
uint64_t GenericSparseHashMap::bucket_mask(const void* dst) const
{
    return Super::bucket_mask(dst);
}
uint64_t& GenericSparseHashMap::bucket_mask(void* dst) const
{
    return Super::bucket_mask(dst);
}

// add
GenericSparseHashMapDataRef GenericSparseHashMap::add(void* dst, const void* key, const void* value)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(key);
    SKR_ASSERT(value);

    size_t hash = _inner_kv_pair->inner_key()->hash(key);
    auto   ref  = add_ex_unsafe(dst, hash, [this, key](const void* k) {
        return _inner_kv_pair->inner_key()->equal(key, k);
    });
    if (ref.already_exist)
    { // assign case
        _inner_kv_pair->inner_key()->assign(_inner_kv_pair->key(ref.ptr), key, 1);
        _inner_kv_pair->inner_value()->assign(_inner_kv_pair->value(ref.ptr), value, 1);
    }
    else
    { // copy case
        _inner_kv_pair->inner_key()->copy(_inner_kv_pair->key(ref.ptr), key, 1);
        _inner_kv_pair->inner_value()->copy(_inner_kv_pair->value(ref.ptr), value, 1);
    }
    return ref;
}
GenericSparseHashMapDataRef GenericSparseHashMap::add_move(void* dst, void* key, void* value)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(key);
    SKR_ASSERT(value);

    size_t hash = _inner_kv_pair->inner_key()->hash(key);
    auto   ref  = add_ex_unsafe(dst, hash, [this, key](const void* k) {
        return _inner_kv_pair->inner_key()->equal(key, k);
    });
    if (ref.already_exist)
    { // assign case
        _inner_kv_pair->inner_key()->move_assign(_inner_kv_pair->key(ref.ptr), key, 1);
        _inner_kv_pair->inner_value()->move_assign(_inner_kv_pair->value(ref.ptr), value, 1);
    }
    else
    { // copy case
        _inner_kv_pair->inner_key()->move(_inner_kv_pair->key(ref.ptr), key, 1);
        _inner_kv_pair->inner_value()->move(_inner_kv_pair->value(ref.ptr), value, 1);
    }
    return ref;
}
GenericSparseHashMapDataRef GenericSparseHashMap::add_ex_unsafe(void* dst, size_t hash, PredType pred)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);

    auto ref = Super::find(dst, hash, [this, pred](const void* pair) {
        return pred(_inner_kv_pair->key(pair));
    });
    if (ref)
    {
        ref.already_exist = true;
        return ref;
    }
    else
    {
        return Super::add_unsafe(dst, hash);
    }
}

// try add (key only add)
GenericSparseHashMapDataRef GenericSparseHashMap::GenericSparseHashMap::try_add_unsafe(void* dst, const void* key)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(key);

    size_t hash = _inner_kv_pair->inner_key()->hash(key);
    auto   ref  = add_ex_unsafe(dst, hash, [this, key](const void* k) {
        return _inner_kv_pair->inner_key()->equal(key, k);
    });
    if (!ref.already_exist)
    {
        _inner_kv_pair->inner_key()->copy(_inner_kv_pair->key(ref.ptr), key, 1);
    }
    return ref;
}
GenericSparseHashMapDataRef GenericSparseHashMap::GenericSparseHashMap::try_add_move_unsafe(void* dst, void* key)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(key);

    size_t hash = _inner_kv_pair->inner_key()->hash(key);
    auto   ref  = add_ex_unsafe(dst, hash, [this, key](const void* k) {
        return _inner_kv_pair->inner_key()->equal(key, k);
    });
    if (!ref.already_exist)
    {
        _inner_kv_pair->inner_key()->move(_inner_kv_pair->key(ref.ptr), key, 1);
    }
    return ref;
}

// append
void GenericSparseHashMap::append(void* dst, const void* src)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);

    auto bit_cursor = Super::CTrueBitCursor::Begin(
        bit_data(src),
        sparse_size(src)
    );

    // fill slack
    uint64_t count = 0;
    while (slack(dst) > 0 && !bit_cursor.reach_end())
    {
        auto* src_data  = at(src, bit_cursor.index());
        auto* src_key   = _inner_kv_pair->key(src_data);
        auto* src_value = _inner_kv_pair->value(src_data);
        add(dst, src_key, src_value);
        bit_cursor.move_next();
        ++count;
    }

    // reserve and add
    if (!bit_cursor.reach_end())
    {
        auto new_capacity = capacity(dst) + (size(src) - count);
        reserve(dst, new_capacity);

        while (!bit_cursor.reach_end())
        {
            auto* src_data  = at(src, bit_cursor.index());
            auto* src_key   = _inner_kv_pair->key(src_data);
            auto* src_value = _inner_kv_pair->value(src_data);
            add(dst, src_key, src_value);
            bit_cursor.move_next();
        }
    }
}

// remove
bool GenericSparseHashMap::remove(void* dst, const void* v)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    size_t hash = _inner_kv_pair->inner_key()->hash(v);
    return Super::remove(dst, hash, [this, v](const void* pair) {
        return _inner_kv_pair->inner_key()->equal(v, _inner_kv_pair->key(pair));
    });
}

// find
GenericSparseHashMapDataRef GenericSparseHashMap::find(void* dst, const void* key) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(key);

    size_t hash = _inner_kv_pair->inner_key()->hash(key);
    return Super::find(dst, hash, [this, key](const void* pair) {
        return _inner_kv_pair->inner_key()->equal(key, _inner_kv_pair->key(pair));
    });
}
CGenericSparseHashMapDataRef GenericSparseHashMap::find(const void* dst, const void* key) const
{
    return find(const_cast<void*>(dst), key);
}

// find if
GenericSparseHashMapDataRef GenericSparseHashMap::find_if(void* dst, PredType pred) const
{
    return Super::find_if(dst, pred);
}
GenericSparseHashMapDataRef GenericSparseHashMap::find_last_if(void* dst, PredType pred) const
{
    return Super::find_if(dst, pred);
}
CGenericSparseHashMapDataRef GenericSparseHashMap::find_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}
CGenericSparseHashMapDataRef GenericSparseHashMap::find_last_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}

// contains
bool GenericSparseHashMap::contains(const void* dst, const void* key) const
{
    return (bool)find(dst, key);
}

// contains if
bool GenericSparseHashMap::contains_if(const void* dst, PredType pred) const
{
    return (bool)find_if(dst, pred);
}

// visitor
const void* GenericSparseHashMap::at(const void* dst, uint64_t idx) const
{
    return Super::at(dst, idx);
}
const void* GenericSparseHashMap::at_last(const void* dst, uint64_t idx) const
{
    return Super::at_last(dst, idx);
}
} // namespace skr