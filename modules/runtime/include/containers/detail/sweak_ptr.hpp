#pragma once
#include "sptr.hpp"
#include "sweak_ptr_base.hpp"

namespace skr
{
template <typename T>
struct SWeakPtr : public SWeakPtrBase<T>
{
    using this_type = SWeakPtr<T>;
public:
    SWeakPtr() SKR_NOEXCEPT = default;
    SWeakPtr(const this_type& lp) SKR_NOEXCEPT;
    SWeakPtr(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(const SWeakPtr<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(SWeakPtr<U>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(const SPtr<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    ~SWeakPtr() SKR_NOEXCEPT;

    void release() SKR_NOEXCEPT;
    inline SPtr<T> lock() const SKR_NOEXCEPT
    {
        // We can't just return shared_ptr<T>(*this), as the object may go stale while we are doing this.
        SPtr<T> temp;
        {
            temp.block = this->block ? this->block->lock() : this->block; // mpRefCount->lock() addref's the return value for us.
            if (temp.block) temp.p = this->p;
        }
        return temp;
    }

    this_type& operator=(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(const SWeakPtr<U>& lp) SKR_NOEXCEPT;
    this_type& operator=(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(SWeakPtr<U>&& lp) SKR_NOEXCEPT;
    template <typename U>

    // assigns to a weak_ptr from a shared_ptr
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(const SPtr<U>& lp) SKR_NOEXCEPT;

    explicit operator bool() const SKR_NOEXCEPT;

    template <typename U> friend struct SPtrHelper;
    template <typename U> friend struct SWeakPtr;
};  
}

// implement SWeakPtr

// default ctors
template <typename T>
skr::SWeakPtr<T>::SWeakPtr(const this_type& lp) SKR_NOEXCEPT : SWeakPtrBase<T>(lp) { }

template <typename T>
template <typename U>
skr::SWeakPtr<T>::SWeakPtr(const SWeakPtr<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T>(lp)
{

}

template <typename T>
skr::SWeakPtr<T>::SWeakPtr(this_type&& lp) SKR_NOEXCEPT  : SWeakPtrBase<T>(std::move(lp)) { }

template <typename T>
template <typename U>
skr::SWeakPtr<T>::SWeakPtr(SWeakPtr<U>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T>(std::move(lp))
{
    
}

template <typename T>
template <typename U>
skr::SWeakPtr<T>::SWeakPtr(const SPtr<U>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T>(lp)
{

}

template <typename T>
skr::SWeakPtr<T>::~SWeakPtr() SKR_NOEXCEPT
{
    if (this->p) this->release();
}

template <typename T>
void skr::SWeakPtr<T>::release() SKR_NOEXCEPT
{
    T* pTemp = this->p;
    if (pTemp)
    {
        this->p = NULL;

        {
            if (this->block)
            {
                this->block->weak_release();
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
    }
}

// operator =

// with reference
template <typename T>
skr::SWeakPtr<T>& skr::SWeakPtr<T>::operator=(const this_type& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr<T>(lp).Swap(*this);
    }
    return *this;
}

template <typename T>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T>&>::type
skr::SWeakPtr<T>::operator=(const skr::SWeakPtr<U>& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr<T>(lp).Swap(*this);
    }
    return *this;
}

// with rvalue
template <typename T>
skr::SWeakPtr<T>& skr::SWeakPtr<T>::operator=(this_type&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr<T>(std::move(lp)).Swap(*this);
    }
    return *this;
}

template <typename T>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T>&>::type
skr::SWeakPtr<T>::operator=(skr::SWeakPtr<U>&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr<T>(std::move(lp)).Swap(*this);
    }
    return *this;
}

// assigns to a weak_ptr from a shared_ptr

template <typename T>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T>&>::type
skr::SWeakPtr<T>::operator=(const SPtr<U>& lp) SKR_NOEXCEPT
{
    this->p = lp.get();

    {
        if (!this->equivalent_rc_ownership(lp)) // This check encompasses assignment to self.
        {
            // Release old reference
            {
                if(this->block)
                    this->block->weak_release();
            }
            this->CopyRCBlockFrom(lp);
            {
                if(this->block)
                    this->block->add_weak_refcount();
            }
        }
    }
    return *this;
}

// operator bool

template <typename T>
skr::SWeakPtr<T>::operator bool() const SKR_NOEXCEPT
{
    return (this->p != NULL);
}

// cmps

namespace skr
{

template <typename T, typename U>
inline bool operator==(const skr::SWeakPtr<T>& a, const skr::SWeakPtr<U>& b) SKR_NOEXCEPT
{
    return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator>(const skr::SWeakPtr<T>& a, const skr::SWeakPtr<U>& b) SKR_NOEXCEPT
{
    return a.get() > b.get();
}

template <typename T, typename U>
inline bool operator<(const skr::SWeakPtr<T>& a, const skr::SWeakPtr<U>& b) SKR_NOEXCEPT
{
    return a.get() < b.get();
}

template <typename T, typename U>
inline bool operator!=(const skr::SWeakPtr<T>& a, const skr::SWeakPtr<U>& b) SKR_NOEXCEPT
{
    return a.get() != b.get();
}

// global cmps
template <typename T>
inline bool operator==(const skr::SWeakPtr<T>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return !a;
}

template <typename T>
inline bool operator==(std::nullptr_t, const skr::SWeakPtr<T>& b) SKR_NOEXCEPT
{
    return !b;
}

template <typename T>
inline bool operator!=(const skr::SWeakPtr<T>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return a;
}

template <typename T>
inline bool operator!=(std::nullptr_t, const skr::SWeakPtr<T>& b) SKR_NOEXCEPT
{
    return b;
}

}

// c++ STL functions equivalent
namespace skr
{

template <typename T, typename U>
inline SWeakPtr<T> reinterpret_pointer_cast(SWeakPtr<U> const& r) SKR_NOEXCEPT
{
    return SWeakPtr<T>(r, reinterpret_cast<T*>(r.get()));
}

template<typename T, typename U> 
SWeakPtr<T> static_pointer_cast( const SWeakPtr<U>& r ) SKR_NOEXCEPT
{
    return SWeakPtr<T>(r, static_cast<T*>(r.get()));
}


}