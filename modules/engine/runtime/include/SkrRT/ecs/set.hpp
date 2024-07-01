#pragma once
#include "SkrContainers/bitset.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/hash.hpp"

namespace sugoi
{
    using bitset32 = skr::Bitset<32>;

    template<class T>
    struct set_utils
    {
        template<class S>
        static size_t hash(const S& s, size_t basis = _FNV_offset_basis)
        {
            return hash_append(basis, reinterpret_cast<const unsigned char*>(s.data), s.length * sizeof(T));
        }

        template<class S>
        static S merge(const S& lhs, const S& rhs, void* d)
        {
            SIndex i = 0, j = 0, k = 0;
            T* dst = (T*)d;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                    dst[k++] = rhs.data[j++];
                else if (lhs.data[i] < rhs.data[j])
                    dst[k++] = lhs.data[i++];
                else
                    dst[k++] = lhs.data[(j++, i++)];
            }
            while (i < lhs.length)
                dst[k++] = lhs.data[i++];
            while (j < rhs.length)
                dst[k++] = rhs.data[j++];
            return  { dst, k };
        }

        template<class S>
        static S merge_with_mask(const S& lhs, const S& rhs, sugoi::bitset32 mask, void* d)
        {
            SIndex i = 0, j = 0, k = 0;
            T* dst = (T*)d;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                {
                    if(mask[i])
                        dst[k++] = rhs.data[j++];
                    else
                        j++;
                } 
                else if (lhs.data[i] < rhs.data[j])
                    dst[k++] = lhs.data[i++];
                else
                    dst[k++] = lhs.data[(j++, i++)];
            }
            while (i < lhs.length)
                dst[k++] = lhs.data[i++];
            while (j < rhs.length)
            {
                if(mask[i])
                    dst[k++] = rhs.data[j++];
                else
                    j++;
            }
            return  { dst, k };
        }

        template<class S>
        static S substract(const S& lhs, const S& rhs, void* d)
        {
            SIndex i = 0, j = 0, k = 0;
            T* dst = (T*)d;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                    j++;
                else if (lhs.data[i] < rhs.data[j])
                    dst[k++] = lhs.data[i++];
                else
                    (j++, i++);
            }
            while (i < lhs.length)
                dst[k++] = lhs.data[i++];
            return  { dst, k };
        }

        template<class S>
        static S intersect(const S& lhs, const S& rhs, void* d)
        {
            SIndex i = 0, j = 0, k = 0;
            T* dst = (T*)d;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                    j++;
                else if (lhs.data[i] < rhs.data[j])
                    i++;
                else
                    dst[k++] = lhs.data[(j++, i++)];
            }
            return  { dst, k };
        }

        template<class S>
        static bool any(const S& lhs, const S& rhs)
        {
            SIndex i = 0, j = 0;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                    j++;
                else if (lhs.data[i] < rhs.data[j])
                    i++;
                else
                    return true;
            }
            return false;
        }

        template<class S>
        static bool all(const S& lhs, const S& rhs)
        {
            SIndex i = 0, j = 0;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                    return false;
                else if (lhs.data[i] < rhs.data[j])
                    i++;
                else
                    (j++, i++);
            }
            return j == rhs.length;
        }
    };
    SKR_RUNTIME_API void sort(const sugoi_type_set_t& value);
    SKR_RUNTIME_API void sort(const sugoi_entity_set_t& value);
    SKR_RUNTIME_API bool equal(const sugoi_type_set_t& a, const sugoi_type_set_t& b);
    SKR_RUNTIME_API bool equal(const sugoi_entity_set_t& a, const sugoi_entity_set_t& b);
    SKR_RUNTIME_API bool equal(const sugoi_entity_type_t& a, const sugoi_entity_type_t& b);

    SKR_RUNTIME_API bool ordered(const sugoi_type_set_t& value);
    SKR_RUNTIME_API bool ordered(const sugoi_entity_set_t& value);
    SKR_RUNTIME_API bool ordered(const sugoi_entity_type_t& value);
    SKR_RUNTIME_API bool ordered(const sugoi_filter_t& value);
    SKR_RUNTIME_API bool ordered(const sugoi_meta_filter_t& value);
    SKR_RUNTIME_API bool ordered(const sugoi_delta_type_t& value);

    SKR_RUNTIME_API size_t hash(const sugoi_type_set_t& value, size_t basis = _FNV_offset_basis);
    SKR_RUNTIME_API size_t hash(const sugoi_entity_set_t& value, size_t basis = _FNV_offset_basis);
    SKR_RUNTIME_API size_t hash(const sugoi_filter_t& value, size_t basis = _FNV_offset_basis);
    SKR_RUNTIME_API size_t hash(const sugoi_meta_filter_t& value, size_t basis = _FNV_offset_basis);
    SKR_RUNTIME_API size_t hash(const sugoi_entity_type_t& value, size_t basis = _FNV_offset_basis);

    SKR_RUNTIME_API size_t data_size(const sugoi_type_set_t& value);
    SKR_RUNTIME_API size_t data_size(const sugoi_entity_set_t& value);
    SKR_RUNTIME_API size_t data_size(const sugoi_filter_t& value);
    SKR_RUNTIME_API size_t data_size(const sugoi_meta_filter_t& value);
    SKR_RUNTIME_API size_t data_size(const sugoi_parameters_t& value);
    SKR_RUNTIME_API size_t data_size(const sugoi_entity_type_t& value);

    SKR_RUNTIME_API sugoi_type_set_t clone(const sugoi_type_set_t& value, char*& buffer);
    SKR_RUNTIME_API sugoi_entity_set_t clone(const sugoi_entity_set_t& value, char*& buffer);
    SKR_RUNTIME_API sugoi_filter_t clone(const sugoi_filter_t& value, char*& buffer);
    SKR_RUNTIME_API sugoi_meta_filter_t clone(const sugoi_meta_filter_t& value, char*& buffer);
    SKR_RUNTIME_API sugoi_parameters_t clone(const sugoi_parameters_t& value, char*& buffer);
    SKR_RUNTIME_API sugoi_entity_type_t clone(const sugoi_entity_type_t& value, char*& buffer);
    SKR_RUNTIME_API bool match(const sugoi_entity_type_t& type, const sugoi_filter_t& value);
}

SUGOI_FORCEINLINE const sugoi_type_index_t* begin(sugoi_type_set_t& value)
{
    return value.data;
}
SUGOI_FORCEINLINE const sugoi_type_index_t* end(sugoi_type_set_t& value)
{
    return value.data + value.length;
}

SUGOI_FORCEINLINE const sugoi_type_index_t* begin(const sugoi_type_set_t& value)
{
    return value.data;
}
SUGOI_FORCEINLINE const sugoi_type_index_t* end(const sugoi_type_set_t& value)
{
    return value.data + value.length;
}
SUGOI_FORCEINLINE const sugoi_entity_t* begin(sugoi_entity_set_t& value)
{
    return value.data;
}
SUGOI_FORCEINLINE const sugoi_entity_t* end(sugoi_entity_set_t& value)
{
    return value.data + value.length;
}
SUGOI_FORCEINLINE const sugoi_entity_t* begin(const sugoi_entity_set_t& value)
{
    return value.data;
}
SUGOI_FORCEINLINE const sugoi_entity_t* end(const sugoi_entity_set_t& value)
{
    return value.data + value.length;
}