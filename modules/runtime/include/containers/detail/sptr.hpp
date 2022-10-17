#pragma once
#include "platform/memory.h"
#include "sptr_base.hpp"

namespace skr
{
template <typename T, bool EmbedRC = !std::is_base_of_v<SInterface, T>>
struct SPtr : public SPtrBase<T, EmbedRC>
{
    static_assert(std::is_base_of_v<SInterface, T> || EmbedRC, 
        "Such SPtr is not allowed, EmbedRC should be true or type T must be derived from SInterface");
    using this_type = SPtr<T, EmbedRC>;
public:
    SPtr() SKR_NOEXCEPT = default;
    SPtr(std::nullptr_t lp) SKR_NOEXCEPT;
    SPtr(T* lp) SKR_NOEXCEPT;
    SPtr(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtr(const SPtr<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT;
    template <typename U>
    SPtr(const SPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    SPtr(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtr(SPtr<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    ~SPtr() SKR_NOEXCEPT;

    template<typename...Args>
    static SPtr<T, EmbedRC> Create(Args&&... args) SKR_NOEXCEPT
    {
        return SPtr<T, EmbedRC>(SkrNew<T>(std::forward<Args>(args)...));
    }
    
    void swap(this_type& lp) SKR_NOEXCEPT;
    void release() SKR_NOEXCEPT;

    void reset() SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type
    reset(U* pValue) SKR_NOEXCEPT;

    // operatpr =
    this_type& operator=(T* lp) SKR_NOEXCEPT;
    this_type& operator=(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(const SPtr<U, EmbedRC>& lp) SKR_NOEXCEPT;
    this_type& operator=(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(SPtr<U, EmbedRC>&& lp) SKR_NOEXCEPT;

    explicit operator bool() const SKR_NOEXCEPT;

    template <typename U, bool B> friend struct SPtr;
    template <typename U, bool B> friend struct SWeakPtr;
};
}

// implement SPtr

// default ctors
template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::SPtr(std::nullptr_t lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::SPtr(T* lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::SPtr(const this_type& lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtr<T, EmbedRC>::SPtr(const SPtr<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(lp, pValue)
{

}

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtr<T, EmbedRC>::SPtr(const SPtr<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(lp)
{

}

template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::SPtr(this_type&& lp) SKR_NOEXCEPT  : SPtrBase<T, EmbedRC>(std::move(lp)) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtr<T, EmbedRC>::SPtr(SPtr<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(std::move(lp))
{
    
}

template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::~SPtr() SKR_NOEXCEPT
{
    if (this->p) release();
}

// methods

template <typename T, bool EmbedRC>
void skr::SPtr<T, EmbedRC>::swap(this_type& lp) SKR_NOEXCEPT
{
    lp.Swap(*this);
}

template <typename T, bool EmbedRC>
void skr::SPtr<T, EmbedRC>::release() SKR_NOEXCEPT
{
    T* pTemp = this->p;
    if (pTemp)
    {
        this->p = NULL;

        if SKR_CONSTEXPR (EmbedRC) 
        {
            if (this->block)
            {
                uint32_t rc = UINT32_MAX;
                this->block->release(&rc);
                if (rc <= 0)
                {
                    this->ActualDelete(pTemp);
                    pTemp = NULL;
                    return;
                }
            }
            else
            {
                this->ActualDelete(pTemp);
                pTemp = NULL;
                SKR_UNREACHABLE_CODE();
            }
        }

        if SKR_CONSTEXPR (std::is_base_of_v<SInterface, T>) 
        {
            auto rc = pTemp->release();
            if (rc <= 0)
            {
                this->ActualDelete(pTemp);
                pTemp = NULL;
                return;
            }
        }
    }
}

template <typename T, bool EmbedRC>
void skr::SPtr<T, EmbedRC>::reset() SKR_NOEXCEPT
{
    this_type().Swap(*this);
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type
skr::SPtr<T, EmbedRC>::reset(U* pValue) SKR_NOEXCEPT
{
    this_type(pValue).Swap(*this);
}

// operator =

// with ptr
template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>& skr::SPtr<T, EmbedRC>::operator=(T* lp) SKR_NOEXCEPT
{
    if(this->p != lp)
    {
        SPtr(lp).Swap(*this);
    }
    return *this;
}


// with reference
template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>& skr::SPtr<T, EmbedRC>::operator=(const this_type& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(lp).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SPtr<T, EmbedRC>&>::type
skr::SPtr<T, EmbedRC>::operator=(const skr::SPtr<U, EmbedRC>& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(lp).Swap(*this);
    }
    return *this;
}

// with rvalue
template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>& skr::SPtr<T, EmbedRC>::operator=(this_type&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(std::move(lp)).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SPtr<T, EmbedRC>&>::type
skr::SPtr<T, EmbedRC>::operator=(skr::SPtr<U, EmbedRC>&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtr(std::move(lp)).Swap(*this);
    }
    return *this;
}

// operator bool

template <typename T, bool EmbedRC>
skr::SPtr<T, EmbedRC>::operator bool() const SKR_NOEXCEPT
{
    return (this->p != NULL);
}

// cmps

namespace skr
{

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator==(const skr::SPtr<T, EmbedRC>& a, const skr::SPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() == b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator>(const skr::SPtr<T, EmbedRC>& a, const skr::SPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() > b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator<(const skr::SPtr<T, EmbedRC>& a, const skr::SPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() < b.get();
}

template <typename T, typename U, bool EmbedRC, bool B>
inline bool operator!=(const skr::SPtr<T, EmbedRC>& a, const skr::SPtr<U, B>& b) SKR_NOEXCEPT
{
    return a.get() != b.get();
}

// global cmps
template <typename T, bool EmbedRC>
inline bool operator==(const skr::SPtr<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return !a;
}

template <typename T, bool EmbedRC>
inline bool operator==(std::nullptr_t, const skr::SPtr<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return !b;
}

template <typename T, bool EmbedRC>
inline bool operator!=(const skr::SPtr<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return a;
}

template <typename T, bool EmbedRC>
inline bool operator!=(std::nullptr_t, const skr::SPtr<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return b;
}

}

// c++ STL functions equivalent
namespace skr
{

template <typename T, typename U, bool EmbedRC>
inline SPtr<T, EmbedRC> reinterpret_pointer_cast(SPtr<U, EmbedRC> const& r) SKR_NOEXCEPT
{
    return SPtr<T, EmbedRC>(r, reinterpret_cast<T*>(r.get()));
}

template<typename T, typename U, bool EmbedRC> 
SPtr<T, EmbedRC> static_pointer_cast( const SPtr<U, EmbedRC>& r ) SKR_NOEXCEPT
{
    return SPtr<T, EmbedRC>(r, static_cast<T*>(r.get()));
}


}