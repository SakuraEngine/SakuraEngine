#include <SkrRTTR/generic/generic_optional.hpp>

namespace skr
{
// ctor & dtor
GenericOptional::GenericOptional(RC<IGenericBase> inner)
    : _inner(std::move(inner))
{
    if (_inner)
    {
        _inner_mem_traits = _inner->memory_traits_data();
        _inner_size       = _inner->size();
        _inner_alignment  = _inner->alignment();
    }
}
GenericOptional::~GenericOptional() = default;

//===> IGenericBase API
// get type info
EGenericKind GenericOptional::kind() const
{
    return EGenericKind::Generic;
}
GUID GenericOptional::id() const
{
    return kOptionalGenericId;
}

// get utils
MemoryTraitsData GenericOptional::memory_traits_data() const
{
    SKR_ASSERT(is_valid());
    MemoryTraitsData data = _inner_mem_traits;
    data.use_ctor         = true; // optional need default ctor
    data.use_compare      = true; // optional will compare has_value
    return data;
}

// basic info
uint64_t GenericOptional::size() const
{
    SKR_ASSERT(is_valid());
    auto alignment        = std::max(_inner_alignment, (uint64_t)alignof(bool));
    auto padded_bool_size = ::skr::memory::padded_size(sizeof(bool), alignment);

    return padded_bool_size + _inner_size;
}
uint64_t GenericOptional::alignment() const
{
    SKR_ASSERT(is_valid());
    return std::max((uint64_t)sizeof(bool), _inner_alignment);
}

// operations, used for generic container algorithms
bool GenericOptional::support(EGenericFeature feature) const
{
    SKR_ASSERT(is_valid());

    switch (feature)
    {
    case EGenericFeature::DefaultCtor:
        return true;
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
void GenericOptional::default_ctor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    ::skr::memory::zero_memory(dst, size() * count);
}
void GenericOptional::dtor(void* dst, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(count > 0);

    auto optional_size = size();

    if (_inner_mem_traits.use_dtor)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, optional_size, i);

            // call dtor if has value
            if (has_value(p_dst))
            {
                auto p_value = value_ptr(p_dst);
                _inner->dtor(p_value);
            }
        }
    }
    else
    {
        ::skr::memory::zero_memory(dst, optional_size * count);
    }
}
void GenericOptional::copy(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    auto optional_size = size();

    if (_inner_mem_traits.use_copy)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, optional_size, i);
            auto p_src = ::skr::memory::offset_item(src, optional_size, i);

            if (has_value(p_src))
            {
                has_value(p_dst) = true;
                _inner->copy(value_ptr(p_dst), value_ptr(p_src));
            }
            else
            {
                has_value(p_dst) = false;
            }
        }
    }
    else
    {
        ::std::memcpy(dst, src, optional_size * count);
    }
}
void GenericOptional::move(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    auto optional_size = size();

    if (_inner_mem_traits.use_move)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, optional_size, i);
            auto p_src = ::skr::memory::offset_item(src, optional_size, i);

            if (has_value(p_src))
            {
                has_value(p_dst) = true;
                _inner->move(value_ptr(p_dst), value_ptr(p_src));
                has_value(src) = false; // reset rhs
            }
            else
            {
                has_value(p_dst) = false;
            }
        }
    }
    else
    {
        ::std::memmove(dst, src, optional_size * count);
        ::skr::memory::zero_memory(src, optional_size * count);
    }
}
void GenericOptional::assign(void* dst, const void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    if (_inner_mem_traits.use_assign)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, size(), i);
            auto p_src = ::skr::memory::offset_item(src, size(), i);

            if (has_value(p_src))
            {
                assign_value(p_dst, value_ptr(p_src));
            }
            else
            {
                reset(p_dst);
            }
        }
    }
    else
    {
        ::std::memcpy(dst, src, size() * count);
    }
}
void GenericOptional::move_assign(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);
    auto optional_size = size();

    if (_inner_mem_traits.use_move_assign)
    {
        // each items
        for (uint64_t i = 0; i < count; ++i)
        {
            auto p_dst = ::skr::memory::offset_item(dst, optional_size, i);
            auto p_src = ::skr::memory::offset_item(src, optional_size, i);

            if (has_value(p_src))
            {
                assign_value_move(p_dst, value_ptr(p_src));
                has_value(src) = false; // reset rhs
            }
            else
            {
                reset(p_dst);
            }
        }
    }
    else
    {
        ::std::memmove(dst, src, optional_size * count);
        ::skr::memory::zero_memory(src, optional_size * count);
    }
}
bool GenericOptional::equal(const void* lhs, const void* rhs, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(lhs);
    SKR_ASSERT(rhs);
    SKR_ASSERT(count > 0);

    SKR_ASSERT(false && "equal not support for GenericOptional, please check feature before call this function");
    return false;
}
size_t GenericOptional::hash(const void* src) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(false && "hash not support for GenericOptional, please check feature before call this function");
    return 0;
}
void GenericOptional::swap(void* dst, void* src, uint64_t count) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(dst);
    SKR_ASSERT(src);
    SKR_ASSERT(count > 0);

    auto optional_size = size();

    for (uint64_t i = 0; i < count; ++i)
    {
        auto p_dst = ::skr::memory::offset_item(dst, optional_size, i);
        auto p_src = ::skr::memory::offset_item(src, optional_size, i);

        if (has_value(p_dst) && has_value(p_src))
        {
            // swap value
            _inner->swap(value_ptr(p_dst), value_ptr(p_src));
        }
        else if (has_value(p_dst) && !has_value(p_src))
        {
            // move value from dst to src
            auto p_value = value_ptr(p_dst);
            _inner->move(value_ptr(p_src), p_value);

            // reset dst
            has_value(dst) = false;
        }
        else if (!has_value(p_dst) && has_value(p_src))
        {
            // move value from src to dst
            auto p_value = value_ptr(p_src);
            _inner->move(value_ptr(p_dst), p_value);

            // reset src
            has_value(src) = false;
        }
    }
}
//===> IGenericBase API

// getter
bool             GenericOptional::is_valid() const { return _inner != nullptr; }
RC<IGenericBase> GenericOptional::inner() const
{
    SKR_ASSERT(is_valid());
    return _inner;
}

// has value
bool GenericOptional::has_value(const void* memory) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(memory);
    return *reinterpret_cast<const bool*>(memory);
}
bool& GenericOptional::has_value(void* memory) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(memory);
    return *reinterpret_cast<bool*>(memory);
}

// value pointer
void* GenericOptional::value_ptr(void* memory) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(memory);
    auto alignment        = std::max(_inner_alignment, (uint64_t)alignof(bool));
    auto padded_bool_size = ::skr::memory::padded_size(sizeof(bool), alignment);
    return ::skr::memory::offset_bytes(
        memory,
        padded_bool_size
    );
}
const void* GenericOptional::value_ptr(const void* memory) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(memory);
    auto alignment        = std::max(_inner_alignment, (uint64_t)alignof(bool));
    auto padded_bool_size = ::skr::memory::padded_size(sizeof(bool), alignment);
    return ::skr::memory::offset_bytes(
        memory,
        padded_bool_size
    );
}

// reset
void GenericOptional::reset(void* memory) const
{
    SKR_ASSERT(is_valid());
    SKR_ASSERT(memory);
    if (has_value(memory))
    {
        auto p_value = this->value_ptr(memory);
        _inner->dtor(p_value);
        has_value(memory) = false;
    }
}

// assign
void GenericOptional::assign_value(void* dst, const void* v) const
{
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    if (has_value(dst))
    {
        auto p_value = this->value_ptr(dst);
        _inner->assign(p_value, v);
    }
    else
    {
        has_value(dst) = true;
        auto p_value   = this->value_ptr(dst);
        _inner->copy(p_value, v);
    }
}
void GenericOptional::assign_value_move(void* dst, void* v) const
{
    SKR_ASSERT(dst);
    SKR_ASSERT(v);

    if (has_value(dst))
    {
        auto p_value = this->value_ptr(dst);
        _inner->move(p_value, v);
    }
    else
    {
        has_value(dst) = true;
        auto p_value   = this->value_ptr(dst);
        _inner->move(p_value, v);
    }
}
void GenericOptional::assign_default(void* dst)
{
    SKR_ASSERT(dst);
    reset(dst);
    has_value(dst) = false;
    auto p_value   = this->value_ptr(dst);
    return _inner->default_ctor(p_value);
}
} // namespace skr