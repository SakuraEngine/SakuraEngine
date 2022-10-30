#pragma once
#include "platform/memory.h"
#include "sptr_base.hpp"

namespace skr
{
template <typename T, bool EmbedRC = true>
struct SPtrHelper : public SPtrBase<T, EmbedRC>
{
    //static_assert(std::is_base_of_v<SInterface, T> || EmbedRC, 
    //    "Such SPtrHelper is not allowed, EmbedRC should be true or type T must be derived from SInterface");
    using this_type = SPtrHelper<T, EmbedRC>;
public:
    SPtrHelper() SKR_NOEXCEPT = default;
    SPtrHelper(std::nullptr_t lp) SKR_NOEXCEPT;
    SPtrHelper(T* lp) SKR_NOEXCEPT;
    SPtrHelper(const this_type& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtrHelper(const SPtrHelper<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT;
    template <typename U>
    SPtrHelper(const SPtrHelper<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    SPtrHelper(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    SPtrHelper(SPtrHelper<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type* = 0) SKR_NOEXCEPT;
    ~SPtrHelper() SKR_NOEXCEPT;

    template<typename...Args>
    static SPtrHelper<T, EmbedRC> Create(Args&&... args) SKR_NOEXCEPT
    {
        return SPtrHelper<T, EmbedRC>(SkrNew<T>(std::forward<Args>(args)...));
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
    operator=(const SPtrHelper<U, EmbedRC>& lp) SKR_NOEXCEPT;
    this_type& operator=(this_type&& lp) SKR_NOEXCEPT;
    template <typename U>
    typename std::enable_if<std::is_convertible<U*, T*>::value, this_type&>::type
    operator=(SPtrHelper<U, EmbedRC>&& lp) SKR_NOEXCEPT;

    explicit operator bool() const SKR_NOEXCEPT;

    template <typename U, bool B> friend struct SPtrHelper;
    template <typename U> friend struct SWeakPtr;
};

template <typename T>
using SPtr = SPtrHelper<T, true>;

template <typename T>
using SObjectPtr = SPtrHelper<T, false>;
}

// implement SPtrHelper

// default ctors
template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(std::nullptr_t lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(T* lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(const this_type& lp) SKR_NOEXCEPT : SPtrBase<T, EmbedRC>(lp) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(const SPtrHelper<U, EmbedRC>& lp, T* pValue) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(lp, pValue)
{

}

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(const SPtrHelper<U, EmbedRC>& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(lp)
{

}

template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(this_type&& lp) SKR_NOEXCEPT  : SPtrBase<T, EmbedRC>(std::move(lp)) { }

template <typename T, bool EmbedRC>
template <typename U>
skr::SPtrHelper<T, EmbedRC>::SPtrHelper(SPtrHelper<U, EmbedRC>&& lp, typename std::enable_if<std::is_convertible<U*, T*>::value>::type*) SKR_NOEXCEPT
    : SPtrBase<T, EmbedRC>(std::move(lp))
{
    
}

template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::~SPtrHelper() SKR_NOEXCEPT
{
    if (this->p) release();
}

// methods

template <typename T, bool EmbedRC>
void skr::SPtrHelper<T, EmbedRC>::swap(this_type& lp) SKR_NOEXCEPT
{
    lp.Swap(*this);
}

template <typename T, bool EmbedRC>
void skr::SPtrHelper<T, EmbedRC>::release() SKR_NOEXCEPT
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
                    pTemp = NULL;
                    return;
                }
            }
            else
            {
                SKR_UNREACHABLE_CODE();
            }
        }
        else
        {
            auto object = sobject_cast<SInterface*>(pTemp);
            auto rc = object->release();
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
void skr::SPtrHelper<T, EmbedRC>::reset() SKR_NOEXCEPT
{
    this_type().Swap(*this);
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, void>::type
skr::SPtrHelper<T, EmbedRC>::reset(U* pValue) SKR_NOEXCEPT
{
    this_type(pValue).Swap(*this);
}

// operator =

// with ptr
template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>& skr::SPtrHelper<T, EmbedRC>::operator=(T* lp) SKR_NOEXCEPT
{
    if(this->p != lp)
    {
        SPtrHelper(lp).Swap(*this);
    }
    return *this;
}


// with reference
template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>& skr::SPtrHelper<T, EmbedRC>::operator=(const this_type& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtrHelper(lp).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SPtrHelper<T, EmbedRC>&>::type
skr::SPtrHelper<T, EmbedRC>::operator=(const skr::SPtrHelper<U, EmbedRC>& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtrHelper(lp).Swap(*this);
    }
    return *this;
}

// with rvalue
template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>& skr::SPtrHelper<T, EmbedRC>::operator=(this_type&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtrHelper(std::move(lp)).Swap(*this);
    }
    return *this;
}

template <typename T, bool EmbedRC>
template <typename U>
typename std::enable_if<std::is_convertible<U*, T*>::value, skr::SPtrHelper<T, EmbedRC>&>::type
skr::SPtrHelper<T, EmbedRC>::operator=(skr::SPtrHelper<U, EmbedRC>&& lp) SKR_NOEXCEPT
{
    if (!this->equivalent_ownership(lp))
    {
        SPtrHelper(std::move(lp)).Swap(*this);
    }
    return *this;
}

// operator bool

template <typename T, bool EmbedRC>
skr::SPtrHelper<T, EmbedRC>::operator bool() const SKR_NOEXCEPT
{
    return (this->p != NULL);
}

// cmps

namespace skr
{

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator==(const skr::SPtrHelper<T, EmbedRC>& a, const skr::SPtrHelper<U, B>& b) SKR_NOEXCEPT
{
    return a.get() == b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator>(const skr::SPtrHelper<T, EmbedRC>& a, const skr::SPtrHelper<U, B>& b) SKR_NOEXCEPT
{
    return a.get() > b.get();
}

template <typename T, bool EmbedRC, typename U, bool B>
inline bool operator<(const skr::SPtrHelper<T, EmbedRC>& a, const skr::SPtrHelper<U, B>& b) SKR_NOEXCEPT
{
    return a.get() < b.get();
}

template <typename T, typename U, bool EmbedRC, bool B>
inline bool operator!=(const skr::SPtrHelper<T, EmbedRC>& a, const skr::SPtrHelper<U, B>& b) SKR_NOEXCEPT
{
    return a.get() != b.get();
}

// global cmps
template <typename T, bool EmbedRC>
inline bool operator==(const skr::SPtrHelper<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return !a;
}

template <typename T, bool EmbedRC>
inline bool operator==(std::nullptr_t, const skr::SPtrHelper<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return !b;
}

template <typename T, bool EmbedRC>
inline bool operator!=(const skr::SPtrHelper<T, EmbedRC>& a, std::nullptr_t) SKR_NOEXCEPT
{
    return a;
}

template <typename T, bool EmbedRC>
inline bool operator!=(std::nullptr_t, const skr::SPtrHelper<T, EmbedRC>& b) SKR_NOEXCEPT
{
    return b;
}

}

// c++ STL functions equivalent
namespace skr
{

template <typename T, typename U, bool EmbedRC>
inline SPtrHelper<T, EmbedRC> reinterpret_pointer_cast(SPtrHelper<U, EmbedRC> const& r) SKR_NOEXCEPT
{
    return SPtrHelper<T, EmbedRC>(r, reinterpret_cast<T*>(r.get()));
}

template<typename T, typename U, bool EmbedRC> 
SPtrHelper<T, EmbedRC> static_pointer_cast( const SPtrHelper<U, EmbedRC>& r ) SKR_NOEXCEPT
{
    return SPtrHelper<T, EmbedRC>(r, static_cast<T*>(r.get()));
}


}