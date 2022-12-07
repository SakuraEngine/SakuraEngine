#pragma once
#include "platform/configure.h"
#include "platform/memory.h"

namespace skr
{
template <class _Ty>
class RUNTIME_API skr_allocator
{
public:
    static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
                                         "because allocator<const T> is ill-formed.");

    using _From_primary = skr_allocator;

    using value_type = _Ty;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;

    constexpr skr_allocator() noexcept {}

    constexpr skr_allocator(const skr_allocator&) noexcept = default;
    template <class _Other>
    constexpr skr_allocator(const skr_allocator<_Other>&) noexcept {}
    ~skr_allocator() = default;
    skr_allocator& operator=(const skr_allocator&) = default;

    void deallocate(_Ty* const _Ptr, const size_t _Count)
    {
        // no overflow check on the following multiply; we assume _Allocate did that check
        sakura_free_aligned(_Ptr, alignof(_Ty));
    }

    _Ty* allocate(const size_t _Count)
    {
        return static_cast<_Ty*>(sakura_new_aligned(sizeof(_Ty) * _Count, alignof(_Ty)));
    }
};
} // namespace skr