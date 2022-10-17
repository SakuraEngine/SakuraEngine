#pragma once
#include "sptr.hpp"
#include "sweak_ptr_base.hpp"

namespace skr
{
template <typename T, bool EmbedRC = !std::is_base_of_v<SInterface, T>>
struct SWeakPtr : public SWeakPtrBase<T, EmbedRC>
{
    using this_type = SWeakPtr<T, EmbedRC>;
public:
    SWeakPtr() SKR_NOEXCEPT = default;
    SWeakPtr(const this_type& lp) SKR_NOEXCEPT;
    SWeakPtr(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(const SWeakPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(SWeakPtr<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    template <typename U>
    SWeakPtr(const SPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    ~SWeakPtr() SKR_NOEXCEPT;

    void release() SKR_NOEXCEPT;
    inline SPtr<T, EmbedRC> lock() const SKR_NOEXCEPT
    {
        // We can't just return shared_ptr<T>(*this), as the object may go stale while we are doing this.
        SPtr<T, EmbedRC> temp;
        if SKR_CONSTEXPR (EmbedRC) 
        {
            temp.block = this->block ? this->block->lock() : this->block; // mpRefCount->lock() addref's the return value for us.
            if (temp.block) temp.p = this->p;
        }
        if SKR_CONSTEXPR (std::is_base_of_v<SInterface, T>)
        {
            temp.p = this->p;
            temp.p->add_refcount();
        }
        return temp;
    }

    this_type& operator=(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(const SWeakPtr<U, EmbedRC>& lp) SKR_NOEXCEPT;
    this_type& operator=(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(SWeakPtr<U, EmbedRC>&& lp) SKR_NOEXCEPT;
    template <typename U>

    // assigns to a weak_ptr from a shared_ptr
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(const SPtr<U, EmbedRC>& lp) SKR_NOEXCEPT;

    explicit operator bool() const SKR_NOEXCEPT;

    template <typename U, bool B> friend struct SPtr;
    template <typename U, bool B> friend struct SWeakPtr;
};  
}

// implement SWeakPtr

// default ctors
template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>::SWeakPtr(const this_type& lp) SKR_NOEXCEPT : SWeakPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SWeakPtr<T, EmbedRC>::SWeakPtr(const SWeakPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T, EmbedRC>(lp)
{

}

template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>::SWeakPtr(this_type&& lp) SKR_NOEXCEPT  : SWeakPtrBase<T>(std::move(lp)) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SWeakPtr<T, EmbedRC>::SWeakPtr(SWeakPtr<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T>(std::move(lp))
{
    
}

template <typename T, bool EmbedRC>
template <typename U>
skr::SWeakPtr<T, EmbedRC>::SWeakPtr(const SPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SWeakPtrBase<T, EmbedRC>(lp)
{

}

template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>::~SWeakPtr() SKR_NOEXCEPT
{
    if (this->p) this->release();
}

template <typename T, bool EmbedRC>
void skr::SWeakPtr<T, EmbedRC>::release() SKR_NOEXCEPT
{
    T* pTemp = this->p;
    if (pTemp)
    {
        this->p = NULL;

        if SKR_CONSTEXPR (EmbedRC)
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
template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>& skr::SWeakPtr<T, EmbedRC>::operator=(const this_type& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(lp).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T, EmbedRC>&>::type
skr::SWeakPtr<T, EmbedRC>::operator=(const skr::SWeakPtr<U, EmbedRC>& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(lp).Swap(*this);
    }
    return *this;
}

// with rvalue
template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>& skr::SWeakPtr<T, EmbedRC>::operator=(this_type&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(std::move(lp)).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T, EmbedRC>&>::type
skr::SWeakPtr<T, EmbedRC>::operator=(skr::SWeakPtr<U, EmbedRC>&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(std::move(lp)).Swap(*this);
    }
    return *this;
}

// assigns to a weak_ptr from a shared_ptr

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SWeakPtr<T, EmbedRC>&>::type
skr::SWeakPtr<T, EmbedRC>::operator=(const SPtr<U, EmbedRC>& lp) SKR_NOEXCEPT
{
    this->p = lp.get();
    if SKR_CONSTEXPR (EmbedRC) 
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

template <typename T, bool EmbedRC>
skr::SWeakPtr<T, EmbedRC>::operator bool() const SKR_NOEXCEPT
{
    return (this->p != NULL);
}

// cmps

namespace skr
{

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator==(const skr::SWeakPtr<T, EmbedRC>& a, const skr::SWeakPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() == b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator>(const skr::SWeakPtr<T, EmbedRC>& a, const skr::SWeakPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() > b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator<(const skr::SWeakPtr<T, EmbedRC>& a, const skr::SWeakPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() < b.get();
}

template <typename T, typename U, bool EmbedRC, bool B>
inline bool operator!=(const skr::SWeakPtr<T, EmbedRC>& a, const skr::SWeakPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() != b.get();
}

// global cmps
template <typename T, bool EmbedRC>
inline bool operator==(const skr::SWeakPtr<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return !a;
}

template <typename T, bool EmbedRC>
inline bool operator==(std::nullptr_t, const skr::SWeakPtr<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return !b;
}

template <typename T, bool EmbedRC>
inline bool operator!=(const skr::SWeakPtr<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return a;
}

template <typename T, bool EmbedRC>
inline bool operator!=(std::nullptr_t, const skr::SWeakPtr<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return b;
}

}

// c++ STL functions equivalent
namespace skr
{

template <typename T, typename U, bool EmbedRC>
inline SWeakPtr<T, EmbedRC> reinterpret_pointer_cast(SWeakPtr<U, EmbedRC> const& r) SKR_NOEXCEPT
{
    return SWeakPtr<T, EmbedRC>(r, reinterpret_cast<T*>(r.get()));
}

template<typename T, typename U, bool EmbedRC> 
SWeakPtr<T, EmbedRC> static_pointer_cast( const SWeakPtr<U, EmbedRC>& r ) SKR_NOEXCEPT
{
    return SWeakPtr<T, EmbedRC>(r, static_cast<T*>(r.get()));
}


}