#pragma once
#include "dual.h"

namespace dual
{
template <class... Ts>
struct static_type_set_T {
    dual_type_index_t storage[sizeof...(Ts)];
    dual_type_set_t typeSet;
    template <size_t... I>
    void Initialize(std::index_sequence<I...>)
    {
        int _[] = { (storage[I] = dual_id_of<Ts>::get(), 0)... };
    }
    static_type_set_T()
    {
        Initialize(std::index_sequence_for<Ts...>{});
        typeSet.data = storage;
        typeSet.length = (SIndex)sizeof...(Ts);
    }
    const dual_type_set_t& get()
    {
        return typeSet;
    }
};

struct RUNTIME_API type_builder_t {
    dual_type_index_t* data;
    SIndex length;

    type_builder_t();
    ~type_builder_t();
    type_builder_t& with(dual_type_index_t* types, uint32_t length);
    template<class... T>
    type_builder_t& with()
    {
        dual_type_index_t types[] = {(dual_id_of<T>::get())...};
        return with(types, sizeof...(T));
    }
    dual_type_set_t build();
};
} // namespace dual