#pragma once
#include <type_traits>
#include "shared_rc.hpp"
#include "platform/configure.h"

namespace skr
{
// SPtrBase is a base class for SPtr.
// We implement ctors, memory management and rc methods here.
template <typename T, bool EmbedRC = !std::is_base_of_v<SInterface, T>>
struct SPtrBase : public SRCInst<EmbedRC>
{
    template<typename U> struct reference_type_helper { using type = U&;};
    template<> struct reference_type_helper<void> { using type = void;};

    using this_type = SPtrBase<T, EmbedRC>;
    using reference_type = typename reference_type_helper<T>::type;
    // operators to T*

    reference_type operator*() const SKR_NOEXCEPT;
    T* operator->() const SKR_NOEXCEPT;
    bool operator!() const SKR_NOEXCEPT;
    inline T* get() const SKR_NOEXCEPT { return this->p; }

    inline bool equivalent_ownership(const this_type& lp) const
    {
        return this->p == lp.get();
    }

    template <typename U>
    inline bool equivalent_ownership(const SPtrBase<U, EmbedRC>& lp) const
    {
        if SKR_CONSTEXPR (EmbedRC)
        {
            // We compare mpRefCount instead of mpValue, because it's feasible that there are two sets of shared_ptr 
            // objects that are unconnected to each other but happen to own the same value pointer. 
            return this->equivalent_rc_ownership(lp);
        }
        return (this->p == lp.get());
    }

protected:
    void ActualDelete(T*) SKR_NOEXCEPT;
    virtual ~SPtrBase() = default;
    SPtrBase() SKR_NOEXCEPT;
    SPtrBase(std::nullptr_t) SKR_NOEXCEPT;

    SPtrBase(T* lp) SKR_NOEXCEPT;
    template<typename Deleter>
    SPtrBase(T* lp, Deleter deleter) SKR_NOEXCEPT;
    // copy constructor
    SPtrBase(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtrBase(const SPtrBase<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT;
    template <typename U>
    SPtrBase(const SPtrBase<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    // move constructor
    SPtrBase(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtrBase(SPtrBase<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;

    void Swap(SPtrBase& other);
    T* p;
};
} // namespace skr

// implement SPtrBase

template <typename T, bool EmbedRC>
typename skr::SPtrBase<T, EmbedRC>::reference_type skr::SPtrBase<T, EmbedRC>::operator*() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return *p;
}

template <typename T, bool EmbedRC>
T* skr::SPtrBase<T, EmbedRC>::operator->() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return p;
}

template <typename T, bool EmbedRC>
bool skr::SPtrBase<T, EmbedRC>::operator!() const SKR_NOEXCEPT
{
    return (p == NULL);
}

// protected SPtrBase

template <typename T, bool EmbedRC>
void skr::SPtrBase<T, EmbedRC>::ActualDelete(T* ptr) SKR_NOEXCEPT
{
    static_assert(!EmbedRC, "Non-Intrusive ptrs should not call this method");
    auto obj = sobject_cast<SInterface*>(ptr);
    if (obj) SkrDelete(obj);
}

template <typename T, bool EmbedRC>
skr::SPtrBase<T, EmbedRC>::SPtrBase() SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = NULL;
}

template <typename T, bool EmbedRC>
skr::SPtrBase<T, EmbedRC>::SPtrBase(std::nullptr_t) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = NULL;
}

template <typename T, bool EmbedRC>
skr::SPtrBase<T, EmbedRC>::SPtrBase(T* lp) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp;
    if (p != NULL) 
    {
        if SKR_CONSTEXPR (EmbedRC) 
        {
            DefaultDeleter<T> deleter = {};
            this->template allocate_block<T>(lp, deleter);
        }
        else
        {
            p->add_refcount();
        }
    }
}

template <typename T, bool EmbedRC>
template<typename Deleter>
skr::SPtrBase<T, EmbedRC>::SPtrBase(T* lp, Deleter deleter) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp;
    if (p != NULL) 
    {
        if SKR_CONSTEXPR (EmbedRC) 
        {
            this->template allocate_block<T, Deleter>(lp, deleter);
        }
        else
        {
            p->add_refcount();
        }
    }
}

template <typename T, bool EmbedRC>
skr::SPtrBase<T, EmbedRC>::SPtrBase(const this_type& lp) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.p;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_refcount();
    }
    else
    {
        p->add_refcount();
    }
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrBase<T, EmbedRC>::SPtrBase(const SPtrBase<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = pValue;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_refcount();
    }
    else
    {
        p->add_refcount();
    }
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrBase<T, EmbedRC>::SPtrBase(const SPtrBase<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.get();

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_refcount();
    }
    else
    {
        auto object = sobject_cast<SInterface*>(p);
        object->add_refcount();
    }
}

template <typename T, bool EmbedRC>
skr::SPtrBase<T, EmbedRC>::SPtrBase(this_type&& lp) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.get();
    lp.p = nullptr;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->MoveRCBlockFrom(lp);
    }
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrBase<T, EmbedRC>::SPtrBase(SPtrBase<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.get();
    ((this_type*)&lp)->p = nullptr;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->MoveRCBlockFrom(lp);
    }
}

template <typename T, bool EmbedRC>
void skr::SPtrBase<T, EmbedRC>::Swap(SPtrBase& lp)
{
    T* pTemp = p;
    p = lp.p;
    lp.p = pTemp;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->SwapRCBlock(lp);
    }
}