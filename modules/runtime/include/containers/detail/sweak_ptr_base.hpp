#pragma once
#include "sptr.hpp"

namespace skr
{
// SWeakPtrBase is a base class for SWeakPtr.
// We implement ctors, memory management and rc methods here.
template <typename T, bool EmbedRC = !std::is_base_of_v<SInterface, T>>
struct SWeakPtrBase : public SRCInst<EmbedRC>
{
    using this_type = SWeakPtrBase<T, EmbedRC>;
    // operators to T*

    T& operator*() const SKR_NOEXCEPT;
    T* operator->() const SKR_NOEXCEPT;
    bool operator!() const SKR_NOEXCEPT;

protected:
    SWeakPtrBase() SKR_NOEXCEPT;
    // copy constructor
    SWeakPtrBase(const SPtr<T, EmbedRC>& lp) SKR_NOEXCEPT;
    SWeakPtrBase(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(const SWeakPtrBase<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(const SWeakPtrBase<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    // move constructor
    SWeakPtrBase(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(SWeakPtrBase<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;

    void Swap(SWeakPtrBase& other);
    T* p;
};
} // namespace skr

// implement SWeakPtrBase

template <typename T, bool EmbedRC>
T& skr::SWeakPtrBase<T, EmbedRC>::operator*() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return *p;
}

template <typename T, bool EmbedRC>
T* skr::SWeakPtrBase<T, EmbedRC>::operator->() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return p;
}

template <typename T, bool EmbedRC>
bool skr::SWeakPtrBase<T, EmbedRC>::operator!() const SKR_NOEXCEPT
{
    return (p == NULL);
}

// protected SPtrBase

template <typename T, bool EmbedRC>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase() SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = NULL;
}

template <typename T, bool EmbedRC>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(const skr::SPtr<T, EmbedRC>& lp) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.get();

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T, bool EmbedRC>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(const this_type& lp) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.p;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(const SWeakPtrBase<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = pValue;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(const SWeakPtrBase<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SRCInst<EmbedRC>()
{
    p = lp.get();

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T, bool EmbedRC>
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(this_type&& lp) SKR_NOEXCEPT
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
skr::SWeakPtrBase<T, EmbedRC>::SWeakPtrBase(SWeakPtrBase<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
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
void skr::SWeakPtrBase<T, EmbedRC>::Swap(SWeakPtrBase& lp)
{
    T* pTemp = p;
    p = lp.p;
    lp.p = pTemp;

    if SKR_CONSTEXPR (EmbedRC) 
    {
        this->SwapRCBlock(lp);
    }
}