#pragma once
#include "SkrBase/types/expected.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_index.hpp"

namespace sugoi
{
static constexpr type_index_t kDisableComponent = type_index_t(0, false, false, true, false);
static constexpr type_index_t kDeadComponent = type_index_t(1, false, false, true, false);
static constexpr type_index_t kLinkComponent = type_index_t(2, false, true, false, false);
static constexpr type_index_t kMaskComponent = type_index_t(3, false, false, false, false);
static constexpr type_index_t kGuidComponent = type_index_t(4, false, false, false, false);
static constexpr type_index_t kDirtyComponent = type_index_t(5, false, false, false, false);

using type_description_t = sugoi_type_description_t;

struct SKR_RUNTIME_API TypeRegistry {
    struct Impl;
    TypeRegistry(Impl& impl);
    TypeRegistry(const TypeRegistry&) = delete;

    static TypeRegistry& get();
    
    type_index_t register_type(const sugoi_type_description_t& desc);
    type_index_t get_type(const guid_t& guid);
    type_index_t get_type(skr::StringView name);
    const sugoi_type_description_t* get_type_desc(sugoi_type_index_t idx);
    void foreach_types(sugoi_type_callback_t callback, void* u);

    // TODO: remove this
    intptr_t map_entity_field(intptr_t p);

    guid_t make_guid();

    enum class BuilderType
    {
        Plain,
        Array,
        Tag
    };
    template <BuilderType Type, size_t N = 0>
    struct TypeBuilder;
    
    template <typename T>
    auto new_type() SKR_NOEXCEPT;

    template <typename T, size_t N>
    auto new_array() SKR_NOEXCEPT;
    
    auto new_tag() SKR_NOEXCEPT;

protected:
    Impl& impl;
};

enum class TypeRegisterError
{
    UnknownError,
    ZeroSize,
    ZeroArrayLength,
    InvalidName,
    InvalidGUID,
    GUIDAlreadyExists,
    NameAlreadyExists,
};

using TypeRegisterResult = skr::Expected<TypeRegisterError, type_index_t>;

template <TypeRegistry::BuilderType Type, size_t N>
struct TypeRegistry::TypeBuilder {
public:
    TypeBuilder(TypeRegistry& registry) SKR_NOEXCEPT
        : registry(registry)
    {
    }

    TypeBuilder& align(uint32_t align) SKR_NOEXCEPT requires(Type != BuilderType::Tag)
    {
        desc.alignment = align;
        return *this;
    }

    TypeBuilder& size(uint32_t size) SKR_NOEXCEPT requires(Type == BuilderType::Plain)
    {
        desc.size = size;
        return *this;
    }

    TypeBuilder& element_size(uint32_t size) SKR_NOEXCEPT requires(Type == BuilderType::Array)
    {
        desc.elementSize = size;
        return *this;
    }

    TypeBuilder& element_count(uint32_t cnt) SKR_NOEXCEPT requires(Type == BuilderType::Array)
    {
        _element_count = cnt;
        return *this;
    }

    TypeBuilder& guid(const guid_t& guid) SKR_NOEXCEPT
    {
        desc.guid = guid;
        return *this;
    }

    TypeBuilder& name(const char8_t* name) SKR_NOEXCEPT
    {
        desc.name = name;
        return *this;
    }

#define ERROR_CASE(cond, err) if (cond) return err

    TypeRegisterResult commit() SKR_NOEXCEPT
    {
        if constexpr (Type == BuilderType::Plain)
        {
            ERROR_CASE(desc.size == 0, TypeRegisterError::ZeroSize);
        }
        else if constexpr (Type == BuilderType::Array)
        {
            ERROR_CASE(desc.size != 0, TypeRegisterError::UnknownError);
            ERROR_CASE(desc.elementSize == 0, TypeRegisterError::ZeroSize);
            ERROR_CASE(_element_count == 0, TypeRegisterError::ZeroArrayLength);
            const uint16_t alignment = std::max((uint16_t)alignof(sugoi_array_comp_t), desc.alignment);
            desc.size = desc.elementSize * _element_count + sizeof(sugoi_array_comp_t);
            desc.size = static_cast<uint16_t>((static_cast<uint16_t>(desc.size) + (alignment - 1)) & ~(alignment - 1));
        }
        ERROR_CASE(desc.guid.is_zero(), TypeRegisterError::InvalidGUID);
        ERROR_CASE(desc.name == nullptr, TypeRegisterError::InvalidName);
        ERROR_CASE(registry.get_type(desc.guid) != kInvalidTypeIndex, TypeRegisterError::GUIDAlreadyExists);
        ERROR_CASE(registry.get_type(desc.name) != kInvalidTypeIndex, TypeRegisterError::NameAlreadyExists);
        return registry.register_type(desc);
    }

#undef ERROR_CASE

protected:
    TypeRegistry& registry;
    sugoi_type_description_t desc = {};
    size_t _element_count = N;
};

template<typename T>
inline auto TypeRegistry::new_type() SKR_NOEXCEPT {
    return TypeBuilder<TypeRegistry::BuilderType::Plain>(*this)
                .align(alignof(T))
                .size(sizeof(T));
}

template<typename T, size_t N>
inline auto TypeRegistry::new_array() SKR_NOEXCEPT {
    using ArrayType = sugoi::ArrayComponent<T, N>;
    return TypeBuilder<TypeRegistry::BuilderType::Array, N>(*this)
                .align(alignof(ArrayType))
                .element_size(sizeof(T))
                .element_count(N);
}

inline auto TypeRegistry::new_tag() SKR_NOEXCEPT {
    return TypeBuilder<TypeRegistry::BuilderType::Tag>(*this);
}

} // namespace sugoi