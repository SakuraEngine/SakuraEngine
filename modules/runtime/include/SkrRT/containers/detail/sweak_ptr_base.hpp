#pragma once
#include "sptr.hpp"

namespace skr
{
// SWeakPtrBase is a base class for SWeakPtr.
// We implement ctors, memory management and rc methods here.
template <typename T>
struct SWeakPtrBase : public SRCInst<true>
{
    using this_type = SWeakPtrBase<T>;
    // operators to T*

    T& operator*() const SKR_NOEXCEPT;
    T* operator->() const SKR_NOEXCEPT;
    bool operator!() const SKR_NOEXCEPT;

protected:
    SWeakPtrBase() SKR_NOEXCEPT;
    // copy constructor
    SWeakPtrBase(const SPtr<T>& lp) SKR_NOEXCEPT;
    SWeakPtrBase(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(const SWeakPtrBase<U>& lp, T* pValue) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(const SWeakPtrBase<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    // move constructor
    SWeakPtrBase(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtrBase(SWeakPtrBase<U>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;

    void Swap(SWeakPtrBase& other);
    T* p;
};
} // namespace skr

// implement SWeakPtrBase

template <typename T>
T& skr::SWeakPtrBase<T>::operator*() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return *p;
}

template <typename T>
T* skr::SWeakPtrBase<T>::operator->() const SKR_NOEXCEPT
{
    SKR_ASSERT(p != NULL);
    return p;
}

template <typename T>
bool skr::SWeakPtrBase<T>::operator!() const SKR_NOEXCEPT
{
    return (p == NULL);
}

// protected SPtrBase

template <typename T>
skr::SWeakPtrBase<T>::SWeakPtrBase() SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = NULL;
}

template <typename T>
skr::SWeakPtrBase<T>::SWeakPtrBase(const skr::SPtr<T>& lp) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = lp.get();

    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T>
skr::SWeakPtrBase<T>::SWeakPtrBase(const this_type& lp) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = lp.p;

    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T>
template <typename U>
skr::SWeakPtrBase<T>::SWeakPtrBase(const SWeakPtrBase<U>& lp, T* pValue) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = pValue;

    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T>
template <typename U>
skr::SWeakPtrBase<T>::SWeakPtrBase(const SWeakPtrBase<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = lp.get();

    {
        this->CopyRCBlockFrom(lp);
        if (this->block) this->block->add_weak_refcount();
    }
}

template <typename T>
skr::SWeakPtrBase<T>::SWeakPtrBase(this_type&& lp) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = lp.get();
    lp.p = nullptr;

    {
        this->MoveRCBlockFrom(lp);
    }
}

template <typename T>
template <typename U>
skr::SWeakPtrBase<T>::SWeakPtrBase(SWeakPtrBase<U>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SRCInst<true>()
{
    p = lp.get();
    ((this_type*)&lp)->p = nullptr;

    {
        this->MoveRCBlockFrom(lp);
    }
}

template <typename T>
void skr::SWeakPtrBase<T>::Swap(SWeakPtrBase& lp)
{
    T* pTemp = p;
    p = lp.p;
    lp.p = pTemp;

    {
        this->SwapRCBlock(lp);
    }
}