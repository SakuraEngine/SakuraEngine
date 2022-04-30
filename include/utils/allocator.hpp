#pragma once
#include "mimalloc.h"

namespace skr
{
template <class _Ty>
class mi_allocator
{
public:
    static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
                                         "because allocator<const T> is ill-formed.");

    using _From_primary = mi_allocator;

    using value_type = _Ty;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;

    constexpr mi_allocator() noexcept {}

    constexpr mi_allocator(const mi_allocator&) noexcept = default;
    template <class _Other>
    constexpr mi_allocator(const mi_allocator<_Other>&) noexcept {}
    ~mi_allocator() = default;
    mi_allocator& operator=(const mi_allocator&) = default;

    void deallocate(_Ty* const _Ptr, const size_t _Count)
    {
        // no overflow check on the following multiply; we assume _Allocate did that check
        ::mi_free_size_aligned(_Ptr, sizeof(_Ty) * _Count, alignof(_Ty));
    }

    __declspec(allocator) _Ty* allocate(const size_t _Count)
    {
        return static_cast<_Ty*>(::mi_new_aligned(sizeof(_Ty) * _Count, alignof(_Ty)));
    }
};
} // namespace skr