#pragma once
#include "dual.h"
#include "SmallVector.h"

namespace dual
{
template <class... Ts>
struct static_type_set_T {
    dual_type_set_t typeSet;
    template <size_t N, size_t... I>
    void Initialize(dual_type_index_t types[N], std::index_sequence<I...>)
    {
        static constexpr size_t length = sizeof...(Ts) + N;
        static dual_type_index_t storage[length];
        for(int i=0; i<N; ++i)
            storage[i] = types[i];
        int _[] = { (storage[I + N] = dual_id_of<Ts>::get(), 0)... };
        std::sort(storage, storage + length);
        typeSet.data = storage;
        typeSet.length = (SIndex)length;
    }
    template <size_t... I>
    void Initialize(std::index_sequence<I...>)
    {
        static constexpr size_t length = sizeof...(Ts);
        static dual_type_index_t storage[length];
        int _[] = { (storage[I] = dual_id_of<Ts>::get(), 0)... };
        std::sort(storage, storage + length);
        typeSet.data = storage;
        typeSet.length = (SIndex)length;
    }
    static_type_set_T()
    {
        Initialize(std::index_sequence_for<Ts...>{});
    }
    template<class... TArgs>
    static_type_set_T(TArgs... args)
    {
        dual_type_index_t types[] = {args...};
        Initialize<sizeof...(TArgs)>(types, std::index_sequence_for<Ts...>{});
    }
    const dual_type_set_t& get()
    {
        return typeSet;
    }
};

struct RUNTIME_API type_builder_t {
    type_builder_t() = default;
    ~type_builder_t() = default;
    type_builder_t& with(const dual_type_index_t* types, uint32_t length);
    type_builder_t& with(dual_type_index_t type)
    {
        return with(&type, 1);
    }
    template<class... T>
    type_builder_t& with()
    {
        dual_type_index_t types[] = {(dual_id_of<T>::get())...};
        return with(types, sizeof...(T));
    }
    void reserve(uint32_t size);
    dual_type_set_t build();
protected:
    llvm_vecsmall::SmallVector<dual_type_index_t, 8> indices;
};
} // namespace dual