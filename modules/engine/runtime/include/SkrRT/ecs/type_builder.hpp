#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"

namespace sugoi
{
template <typename... Ts>
struct StaticTypeSet {
public:
    StaticTypeSet()
    {
        initialize(std::index_sequence_for<Ts...>{});
    }
    template<typename... TArgs>
    StaticTypeSet(TArgs... args)
    {
        sugoi_type_index_t types[] = {args...};
        initialize<sizeof...(TArgs)>(types, std::index_sequence_for<Ts...>{});
    }
    const sugoi_type_set_t& get()
    {
        return typeSet;
    }
protected: 
    template <size_t N, size_t... I>
    void initialize(sugoi_type_index_t types[N], std::index_sequence<I...>)
    {
        static constexpr size_t length = sizeof...(Ts) + N;
        static sugoi_type_index_t storage[length];
        for(int i=0; i<N; ++i)
            storage[i] = types[i];
        int _[] = { (storage[I + N] = sugoi_id_of<Ts>::get(), 0)... };
        std::sort(storage, storage + length);
        typeSet.data = storage;
        typeSet.length = (SIndex)length;
    }
    template <size_t... I>
    void initialize(std::index_sequence<I...>)
    {
        static constexpr size_t length = sizeof...(Ts);
        static sugoi_type_index_t storage[length];
        int _[] = { (storage[I] = sugoi_id_of<Ts>::get(), 0)... };
        std::sort(storage, storage + length);
        typeSet.data = storage;
        typeSet.length = (SIndex)length;
    }
    sugoi_type_set_t typeSet;
};

struct SKR_RUNTIME_API TypeSetBuilder {
    TypeSetBuilder();
    ~TypeSetBuilder();
    TypeSetBuilder(const TypeSetBuilder&);
    TypeSetBuilder(TypeSetBuilder&&);
    TypeSetBuilder& operator=(const TypeSetBuilder&);
    TypeSetBuilder& operator=(TypeSetBuilder&&);
    TypeSetBuilder& with(const sugoi_type_index_t* types, uint32_t length);
    TypeSetBuilder& with(sugoi_type_index_t type)
    {
        return with(&type, 1);
    }
    template<typename... T>
    TypeSetBuilder& with()
    {
        sugoi_type_index_t types[] = {(sugoi_id_of<T>::get())...};
        return with(types, sizeof...(T));
    }
    void reserve(uint32_t size);
    sugoi_type_set_t build();
    bool empty() const { return indices.empty(); }
protected:
    sugoi::array_comp_T<sugoi_type_index_t, 8> indices;
};

template<typename... T>
struct EntitySpawner
{
    TypeSetBuilder builder;
    sugoi_entity_type_t type;
    EntitySpawner()
    {
        type.type = builder.with<T...>().build();
        type.meta = {nullptr, 0};
    }
    struct View
    {
        sugoi_chunk_view_t* view;
        std::tuple<T*...> components;
        std::tuple<T*...> unpack() { return components; }
        uint32_t count() const { return view->count; }
    };
    template<typename F>
    void operator()(sugoi_storage_t* storage, uint32_t count, F&& f)
    {
        auto trampoline = +[](void* u, sugoi_chunk_view_t* v)
        {
            auto& f = *(F*)u;
            View view = {v, std::make_tuple(((T*)sugoiV_get_owned_ro(v, sugoi_id_of<T>::get()))...)};
            f(view);
        };
        sugoiS_allocate_type(storage, &type, count, trampoline, &f);
    }
    View operator()(sugoi_storage_t* storage)
    {
        View view;
        auto trampoline = +[](void* u, sugoi_chunk_view_t* v)
        {
            auto& view = *(View*)u;
            view = {v, std::make_tuple(((T*)sugoiV_get_owned_ro(v, sugoi_id_of<T>::get()))...)};
        };
        sugoiS_allocate_type(storage, &type, 1, trampoline, &view);
        return view;
    }
};

} // namespace sugoi