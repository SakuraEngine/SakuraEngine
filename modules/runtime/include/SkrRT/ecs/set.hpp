#pragma once
#include "ecs/dual.h"
#include "hash.hpp"
#include <EASTL/algorithm.h>
#include <EASTL/bitset.h>

namespace dual
{
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
        static S merge_with_mask(const S& lhs, const S& rhs, eastl::bitset<32> mask, void* d)
        {
            SIndex i = 0, j = 0, k = 0;
            T* dst = (T*)d;
            while (i < lhs.length && j < rhs.length)
            {
                if (lhs.data[i] > rhs.data[j])
                {
                    if(mask.test(i))
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
                if(mask.test(i))
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
    void sort(const dual_type_set_t& value);
    void sort(const dual_entity_set_t& value);
    bool equal(const dual_type_set_t& a, const dual_type_set_t& b);
    bool equal(const dual_entity_set_t& a, const dual_entity_set_t& b);
    bool equal(const dual_entity_type_t& a, const dual_entity_type_t& b);

    bool ordered(const dual_type_set_t& value);
    bool ordered(const dual_entity_set_t& value);
    bool ordered(const dual_entity_type_t& value);
    bool ordered(const dual_filter_t& value);
    bool ordered(const dual_meta_filter_t& value);
    bool ordered(const dual_delta_type_t& value);

    size_t hash(const dual_type_set_t& value, size_t basis = _FNV_offset_basis);
    size_t hash(const dual_entity_set_t& value, size_t basis = _FNV_offset_basis);
    size_t hash(const dual_filter_t& value, size_t basis = _FNV_offset_basis);
    size_t hash(const dual_meta_filter_t& value, size_t basis = _FNV_offset_basis);
    size_t hash(const dual_entity_type_t& value, size_t basis = _FNV_offset_basis);

    size_t data_size(const dual_type_set_t& value);
    size_t data_size(const dual_entity_set_t& value);
    size_t data_size(const dual_filter_t& value);
    size_t data_size(const dual_meta_filter_t& value);
    size_t data_size(const dual_parameters_t& value);
    size_t data_size(const dual_entity_type_t& value);

    dual_type_set_t clone(const dual_type_set_t& value, char*& buffer);
    dual_entity_set_t clone(const dual_entity_set_t& value, char*& buffer);
    dual_filter_t clone(const dual_filter_t& value, char*& buffer);
    dual_meta_filter_t clone(const dual_meta_filter_t& value, char*& buffer);
    dual_parameters_t clone(const dual_parameters_t& value, char*& buffer);
    dual_entity_type_t clone(const dual_entity_type_t& value, char*& buffer);
    bool match(const dual_entity_type_t& type, const dual_filter_t& value);

    
}

DUAL_FORCEINLINE const dual_type_index_t* begin(dual_type_set_t& value)
{
    return value.data;
}
DUAL_FORCEINLINE const dual_type_index_t* end(dual_type_set_t& value)
{
    return value.data + value.length;
}

DUAL_FORCEINLINE const dual_type_index_t* begin(const dual_type_set_t& value)
{
    return value.data;
}
DUAL_FORCEINLINE const dual_type_index_t* end(const dual_type_set_t& value)
{
    return value.data + value.length;
}
DUAL_FORCEINLINE const dual_entity_t* begin(dual_entity_set_t& value)
{
    return value.data;
}
DUAL_FORCEINLINE const dual_entity_t* end(dual_entity_set_t& value)
{
    return value.data + value.length;
}
DUAL_FORCEINLINE const dual_entity_t* begin(const dual_entity_set_t& value)
{
    return value.data;
}
DUAL_FORCEINLINE const dual_entity_t* end(const dual_entity_set_t& value)
{
    return value.data + value.length;
}