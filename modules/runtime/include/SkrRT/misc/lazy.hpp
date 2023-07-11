#pragma once
#include <type_traits>
#include <new>

namespace skr
{
    template<class T>
    struct lazy_t
    {
    private:
        mutable bool initialized = false;
        std::aligned_storage_t<sizeof(T), alignof(T)> storage;

    public:
        explicit operator bool() const { return initialized; }

        bool is_initialized() const
        {
            return initialized;
        }
        void initialize() const { ::new ((void*)&storage) T(); initialized = true; }
        
        T& get()
        {
            if(!initialized)
                initialize();
            return *std::launder(reinterpret_cast<T*>(&storage));
        }

        const T& get() const
        {
            if(!initialized)
                initialize();
            return *std::launder(reinterpret_cast<const T*>(&storage));
        }

        template<class F>
        auto then(F&& f)
        {
            using R = decltype(f(get()));
            if(initialized)
                return f(*std::launder(reinterpret_cast<T*>(&storage)));
            if constexpr(!std::is_void_v<R>)
                return R{};
        }

        template<class F>
        auto then(F&& f) const
        {
            using R = decltype(f(get()));
            if(initialized)
                return f(*std::launder(reinterpret_cast<const T*>(&storage)));
            if constexpr(!std::is_void_v<R>)
                return R{};
        }
        
        T* operator->() { return &get(); }
        T& operator*() { return get(); }
        const T* operator->() const { return &get(); }
        const T& operator*() const { return get(); }
        ~lazy_t()
        {
            if(initialized)
               std::launder(reinterpret_cast<T*>(&storage))->~T();
               
        }
    };

    template<class F>
    auto lazy_construct(F&& f)
    {
        using T = std::invoke_result_t<F>;
        struct result_t
        {
            std::remove_reference_t<F> f;
            operator T()
            {
                return f();
            }
        };
        return result_t{std::forward<F>(f)};
    }
}