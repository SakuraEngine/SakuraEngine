#include <SkrRTTR/generic/generic_sparse_hash_set.hpp>

namespace skr
{
// ctor & dtor
GenericSparseHashSet::GenericSparseHashSet(RC<IGenericBase> inner)
    : GenericSparseHashBase(std::move(inner))
{
}
GenericSparseHashSet::~GenericSparseHashSet() = default;

//===> IGenericBase API
// get type info
EGenericKind GenericSparseHashSet::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericSparseHashSet::id() const
{
    return kSetGenericId;
}

// get utils
MemoryTraitsData GenericSparseHashSet::memory_traits_data() const
{
    return Super::memory_traits_data();
}

// basic info
uint64_t GenericSparseHashSet::size() const
{
    return sizeof(SetMemoryBase);
}
uint64_t GenericSparseHashSet::alignment() const
{
    return alignof(SetMemoryBase);
}

// operations, used for generic container algorithms
bool GenericSparseHashSet::support(EGenericFeature feature) const
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
void GenericSparseHashSet::default_ctor(void* dst, uint64_t count) const
{
    Super::default_ctor(dst, count);
}
void GenericSparseHashSet::dtor(void* dst, uint64_t count) const
{
    Super::dtor(dst, count);
}
void GenericSparseHashSet::copy(void* dst, const void* src, uint64_t count) const
{
    Super::copy(dst, src, count);
}
void GenericSparseHashSet::move(void* dst, void* src, uint64_t count) const
{
    Super::move(dst, src, count);
}
void GenericSparseHashSet::assign(void* dst, const void* src, uint64_t count) const
{
    Super::assign(dst, src, count);
}
void GenericSparseHashSet::move_assign(void* dst, void* src, uint64_t count) const
{
    Super::move_assign(dst, src, count);
}
bool GenericSparseHashSet::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);

    if (size(lhs) == size(rhs))
    {
        return is_sub_set_of(lhs, rhs);
    }
    else
    {
        return false;
    }
}
size_t GenericSparseHashSet::hash(const void* src) const
{
    return Super::hash(src);
}
void GenericSparseHashSet::swap(void* dst, void* src, uint64_t count) const
{
    Super::swap(dst, src, count);
}
//===> IGenericBase API

// getter
bool GenericSparseHashSet::is_valid() const
{
    return Super::is_valid();
}
RC<IGenericBase> GenericSparseHashSet::inner() const
{
    return Super::inner();
}
RC<GenericSparseHashSetStorage> GenericSparseHashSet::inner_storage() const
{
    return Super::inner_storage();
}

// sparse hash set getter
uint64_t GenericSparseHashSet::size(const void* dst) const
{
    return Super::size(dst);
}
uint64_t GenericSparseHashSet::capacity(const void* dst) const
{
    return Super::capacity(dst);
}
uint64_t GenericSparseHashSet::slack(const void* dst) const
{
    return Super::slack(dst);
}
uint64_t GenericSparseHashSet::sparse_size(const void* dst) const
{
    return Super::sparse_size(dst);
}
uint64_t GenericSparseHashSet::hole_size(const void* dst) const
{
    return Super::hole_size(dst);
}
uint64_t GenericSparseHashSet::bit_size(const void* dst) const
{
    return Super::bit_size(dst);
}
uint64_t GenericSparseHashSet::freelist_head(const void* dst) const
{
    return Super::freelist_head(dst);
}
bool GenericSparseHashSet::is_compact(const void* dst) const
{
    return Super::is_compact(dst);
}
bool GenericSparseHashSet::is_empty(const void* dst) const
{
    return Super::is_empty(dst);
}
void* GenericSparseHashSet::storage(void* dst) const
{
    return Super::storage(dst);
}
const void* GenericSparseHashSet::storage(const void* dst) const
{
    return Super::storage(dst);
}
uint64_t* GenericSparseHashSet::bit_data(void* dst) const
{
    return Super::bit_data(dst);
}
const uint64_t* GenericSparseHashSet::bit_data(const void* dst) const
{
    return Super::bit_data(dst);
}
uint64_t* GenericSparseHashSet::bucket(void* dst) const
{
    return Super::bucket(dst);
}
const uint64_t* GenericSparseHashSet::bucket(const void* dst) const
{
    return Super::bucket(dst);
}
uint64_t GenericSparseHashSet::bucket_size(const void* dst) const
{
    return Super::bucket_size(dst);
}
uint64_t& GenericSparseHashSet::bucket_size(void* dst) const
{
    return Super::bucket_size(dst);
}
uint64_t GenericSparseHashSet::bucket_mask(const void* dst) const
{
    return Super::bucket_mask(dst);
}
uint64_t& GenericSparseHashSet::bucket_mask(void* dst) const
{
    return Super::bucket_mask(dst);
}

// add
GenericSparseHashSetDataRef GenericSparseHashSet::add(void* dst, const void* v)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    uint64_t hash = _inner->hash(v);
    if (auto ref = Super::find(dst, hash, [this, v](const void* set_v) {
            return _inner->equal(v, set_v);
        }))
    { // assign case
        ref.already_exist = true;
        _inner->assign(ref.ptr, v, 1);
        return ref;
    }
    else
    { // add case
        auto add_ref = Super::add_unsafe(dst, hash);
        _inner->copy(add_ref.ptr, v, 1);
        return add_ref;
    }
}
GenericSparseHashSetDataRef GenericSparseHashSet::add_move(void* dst, void* v)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    uint64_t hash = _inner->hash(v);
    if (auto ref = Super::find(dst, hash, [this, v](const void* set_v) {
            return _inner->equal(v, set_v);
        }))
    { // assign case
        ref.already_exist = true;
        _inner->move_assign(ref.ptr, v, 1);
        return ref;
    }
    else
    { // add case
        auto add_ref = Super::add_unsafe(dst, hash);
        _inner->move(add_ref.ptr, v, 1);
        return add_ref;
    }
}

// append
void GenericSparseHashSet::append(void* dst, const void* src)
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
        add(dst, at(src, bit_cursor.index()));
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
            add(dst, at(src, bit_cursor.index()));
            bit_cursor.move_next();
        }
    }
}

// remove
bool GenericSparseHashSet::remove(void* dst, const void* v)
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    uint64_t hash = _inner->hash(v);
    return Super::remove(dst, hash, [this, v](const void* set_v) {
        return _inner->equal(v, set_v);
    });
}

// find
GenericSparseHashSetDataRef GenericSparseHashSet::find(void* dst, const void* v) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    uint64_t hash = _inner->hash(v);
    return Super::find(dst, hash, [this, v](const void* set_v) {
        return _inner->equal(v, set_v);
    });
}
CGenericSparseHashSetDataRef GenericSparseHashSet::find(const void* dst, const void* v) const
{
    return find(const_cast<void*>(dst), v);
}

// find if
GenericSparseHashSetDataRef GenericSparseHashSet::find_if(void* dst, PredType pred) const
{
    return Super::find_if(dst, pred);
}
GenericSparseHashSetDataRef GenericSparseHashSet::find_last_if(void* dst, PredType pred) const
{
    return Super::find_if(dst, pred);
}
CGenericSparseHashSetDataRef GenericSparseHashSet::find_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}
CGenericSparseHashSetDataRef GenericSparseHashSet::find_last_if(const void* dst, PredType pred) const
{
    return find_if(const_cast<void*>(dst), pred);
}

// contains
bool GenericSparseHashSet::contains(const void* dst, const void* v) const
{
    return (bool)find(dst, v);
}

// contains if
bool GenericSparseHashSet::contains_if(const void* dst, PredType pred) const
{
    return (bool)find_if(const_cast<void*>(dst), pred);
}

// visitor
const void* GenericSparseHashSet::at(const void* dst, uint64_t idx) const
{
    return Super::at(dst, idx);
}
const void* GenericSparseHashSet::at_last(const void* dst, uint64_t idx) const
{
    return Super::at_last(dst, idx);
}

// set ops
bool GenericSparseHashSet::is_sub_set_of(const void* dst, const void* rhs) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(rhs);

    if (size(rhs) >= size(dst))
    {
        auto bit_cursor = Super::CTrueBitCursor::Begin(
            bit_data(dst),
            sparse_size(dst)
        );

        while (!bit_cursor.reach_end())
        {
            if (!contains(rhs, at(dst, bit_cursor.index())))
            {
                return false; // if any element in dst is not in rhs, return false
            }

            bit_cursor.move_next();
        }
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace skr