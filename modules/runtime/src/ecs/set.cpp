#include "set.hpp"
#include "ecs/dual.h"
#include "ecs/entity.hpp"
#include "hash.hpp"

namespace dual
{
    bool equal(const dual_type_set_t& a, const dual_type_set_t& b)
    {
        if(a.length != b.length)
            return false;
        return std::equal(a.data, a.data+a.length, b.data, b.data+b.length);
    }

    bool equal(const dual_entity_set_t& a, const dual_entity_set_t& b)
    {
        if(a.length != b.length)
            return false;
        return std::equal(a.data, a.data+a.length, b.data, b.data+b.length);
    }
    
    bool equal(const dual_entity_type_t& a, const dual_entity_type_t& b)
    {
        return equal(a.type, b.type) && equal(a.meta, b.meta);
    }

    size_t hash(const dual_type_set_t& value, size_t basis)
    {
        return set_utils<dual_type_index_t>::hash(value, basis);
    }
    
    size_t hash(const dual_entity_set_t& value, size_t basis)
    {
        return set_utils<dual_entity_t>::hash(value, basis);
    }

    bool ordered(const dual_type_set_t& value)
    {
        return value.length == 0 || std::is_sorted(value.data, value.data+value.length);
    }
    bool ordered(const dual_entity_set_t& value)
    {
        return value.length == 0 ||std::is_sorted(value.data, value.data+value.length);
    }
    bool ordered(const dual_entity_type_t& value)
    {
        return ordered(value.type) && ordered(value.meta);
    }
    bool ordered(const dual_filter_t& value)
    {
        return ordered(value.all) && ordered(value.none) &&
            ordered(value.all_shared) && ordered(value.none_shared);
    }
    bool ordered(const dual_meta_filter_t& value)
    {
        return ordered(value.all_meta) && ordered(value.any_meta) && ordered(value.none_meta) &&
            ordered(value.changed);
    }
    bool ordered(const dual_delta_type_t& value)
    {
        return ordered(value.added) && ordered(value.removed);
    }

    size_t hash(const dual_filter_t& value, size_t basis)
    {
        auto result = set_utils<dual_type_index_t>::hash(value.all, basis);
        result = set_utils<dual_type_index_t>::hash(value.none, result);
        result = set_utils<dual_type_index_t>::hash(value.all_shared, result);
        result = set_utils<dual_type_index_t>::hash(value.none_shared, result);
        return result;
    }

    size_t hash(const dual_meta_filter_t& value, size_t basis)
    {
        auto result = set_utils<dual_entity_t>::hash(value.all_meta, basis);
        result = set_utils<dual_entity_t>::hash(value.any_meta, result);
        result = set_utils<dual_entity_t>::hash(value.none_meta, result);
        result = set_utils<dual_type_index_t>::hash(value.changed, result);
        result = hash_bytes(&value.timestamp, 1, result);
        return result;
    }

    size_t hash(const dual_entity_type_t& value, size_t basis)
    {
        auto result = set_utils<dual_type_index_t>::hash(value.type, basis);
        result = set_utils<dual_entity_t>::hash(value.meta, result);
        return result;
    }

    size_t data_size(const dual_type_set_t& value)
    {
        return sizeof(dual_type_index_t) * value.length;
    }

    size_t data_size(const dual_entity_set_t& value)
    {
        return sizeof(dual_entity_t) * value.length;
    }

    size_t data_size(const dual_filter_t& value)
    {
        return data_size(value.all) + data_size(value.none) +
        data_size(value.all_shared) + data_size(value.none_shared);
    }

    size_t data_size(const dual_meta_filter_t& value)
    {
        return data_size(value.all_meta) + data_size(value.any_meta) + data_size(value.none_meta) +
        data_size(value.changed);
    }

    size_t data_size(const dual_parameters_t& value)
    {
        return value.length * sizeof(dual_type_index_t) + value.length * sizeof(dual_operation_t);
    }

    size_t data_size(const dual_entity_type_t& value)
    {
        return data_size(value.type) + data_size(value.meta);
    }


    dual_type_set_t clone(const dual_type_set_t& value, char*& buffer)
    {
        dual_type_set_t result{(dual_type_index_t*)buffer, value.length};
        std::memcpy(buffer, value.data, sizeof(dual_type_index_t) * value.length);
        buffer += sizeof(dual_type_index_t) * value.length;
        return result;
    }

    dual_entity_set_t clone(const dual_entity_set_t& value, char*& buffer)
    {
        dual_entity_set_t result{(dual_entity_t*)buffer, value.length};
        std::memcpy(buffer, value.data, sizeof(dual_entity_t) * value.length);
        buffer += sizeof(dual_entity_t) * value.length;
        return result;
    }
    
    dual_filter_t clone(const dual_filter_t& value, char*& buffer)
    {
        return {
            clone(value.all, buffer),
            clone(value.none, buffer),
            clone(value.all_shared, buffer),
            clone(value.none_shared, buffer)
        };
    }
    
    dual_meta_filter_t clone(const dual_meta_filter_t& value, char*& buffer)
    {
        return {
            clone(value.all_meta, buffer),
            clone(value.any_meta, buffer),
            clone(value.none_meta, buffer),
            clone(value.changed, buffer),
            value.timestamp
        };
    }
    
    dual_parameters_t clone(const dual_parameters_t& value, char*& buffer)
    {
        dual_type_index_t* types = (dual_type_index_t*)buffer;
        memcpy(types, value.types, sizeof(dual_type_index_t) * value.length);
        buffer += sizeof(dual_type_index_t) * value.length;
        dual_operation_t* ops = (dual_operation_t*)buffer;
        memcpy(ops, value.accesses, sizeof(dual_operation_t) * value.length);
        buffer += sizeof(dual_operation_t) * value.length;
        return {
            types,
            ops,
            value.length
        };
    }
    
    dual_entity_type_t clone(const dual_entity_type_t& value, char*& buffer)
    {
        return {
            clone(value.type, buffer),
            clone(value.meta, buffer),
        };
    }
    
}