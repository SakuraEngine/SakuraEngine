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
    type_builder_t();
    ~type_builder_t();
    type_builder_t(const type_builder_t&);
    type_builder_t(type_builder_t&&);
    type_builder_t& operator=(const type_builder_t&);
    type_builder_t& operator=(type_builder_t&&);
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
    bool empty() const { return indices.empty(); }
protected:
    llvm_vecsmall::SmallVector<dual_type_index_t, 8> indices;
};

template<class... T>
struct entity_spawner_T
{
    type_builder_t builder;
    dual_entity_type_t type;
    entity_spawner_T()
    {
        type.type = builder.with<T...>().build();
        type.meta = {nullptr, 0};
    }
    struct View
    {
        dual_chunk_view_t* view;
        std::tuple<T*...> components;
        std::tuple<T*...> unpack() { return components; }
        uint32_t count() const { return view->count; }
    };
    template<class F>
    void operator()(dual_storage_t* storage, uint32_t count, F&& f)
    {
        auto trampoline = +[](void* u, dual_chunk_view_t* v)
        {
            auto& f = *(F*)u;
            View view = {v, std::make_tuple(((T*)dualV_get_owned_ro(v, dual_id_of<T>::get()))...)};
            f(view);
        };
        dualS_allocate_type(storage, &type, count, trampoline, &f);
    }
};

} // namespace dual