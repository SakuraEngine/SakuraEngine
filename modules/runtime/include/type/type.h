#pragma once
#include "utils/types.h"

typedef struct skr_type_t skr_type_t;
typedef struct skr_value_t skr_value_t;
typedef struct skr_value_ref_t skr_value_ref_t;
typedef struct skr_field_t skr_field_t;
typedef struct skr_method_t skr_method_t;
typedef skr_guid_t skr_type_id_t;
struct skr_resource_handle_t;
struct skr_binary_writer_t;
struct skr_binary_reader_t;

SKR_DECLARE_TYPE_ID_FWD(skr::type, DynamicRecordType, skr_dynamic_record_type)

enum skr_type_category_t
{
    SKR_TYPE_CATEGORY_INVALID,
    SKR_TYPE_CATEGORY_BOOL,
    SKR_TYPE_CATEGORY_I32,
    SKR_TYPE_CATEGORY_I64,
    SKR_TYPE_CATEGORY_U32,
    SKR_TYPE_CATEGORY_U64,
    SKR_TYPE_CATEGORY_F32,
    SKR_TYPE_CATEGORY_F64,
    SKR_TYPE_CATEGORY_F32_2,
    SKR_TYPE_CATEGORY_F32_3,
    SKR_TYPE_CATEGORY_F32_4,
    SKR_TYPE_CATEGORY_F32_4x4,
    SKR_TYPE_CATEGORY_ROT,
    SKR_TYPE_CATEGORY_QUAT,
    SKR_TYPE_CATEGORY_GUID,
    SKR_TYPE_CATEGORY_MD5,
    SKR_TYPE_CATEGORY_HANDLE,
    SKR_TYPE_CATEGORY_STR,
    SKR_TYPE_CATEGORY_STRV,
    SKR_TYPE_CATEGORY_ARR,
    SKR_TYPE_CATEGORY_DYNARR,
    SKR_TYPE_CATEGORY_ARRV,
    SKR_TYPE_CATEGORY_OBJ,
    SKR_TYPE_CATEGORY_ENUM,
    SKR_TYPE_CATEGORY_REF,
    SKR_TYPE_CATEGORY_VARIANT
};
typedef enum skr_type_category_t skr_type_category_t;

RUNTIME_EXTERN_C RUNTIME_API 
const char* skr_get_type_name(const skr_guid_t* type);

RUNTIME_EXTERN_C RUNTIME_API 
const struct skr_type_t* skr_get_type(const skr_type_id_t* id);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_register_type_name(const skr_guid_t* type, const char* name);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_get_derived_types(const struct skr_type_t* type, void (*callback)(void* u, struct skr_type_t* type), void* u);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_get_type_id(const struct skr_type_t* type, skr_type_id_t* id);

RUNTIME_EXTERN_C RUNTIME_API
uint32_t skr_get_type_size(const struct skr_type_t* type);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_get_fields(const struct skr_type_t* type, void (*callback)(void* u, skr_field_t* field), void* u);

RUNTIME_EXTERN_C RUNTIME_API 
skr_field_t* skr_get_field(const struct skr_type_t* type, const char* name);

RUNTIME_EXTERN_C RUNTIME_API 
skr_method_t* skr_get_method(const struct skr_type_t* type, const char* name);

RUNTIME_EXTERN_C RUNTIME_API 
struct skr_type_t* skr_get_field_type(const skr_field_t* field);

RUNTIME_EXTERN_C RUNTIME_API 
const char* skr_get_field_name(const skr_field_t* field);

extern const skr_type_t* $type;
extern const skr_field_t* $field;
extern const skr_method_t* $method;

RUNTIME_EXTERN_C RUNTIME_API 
skr_dynamic_record_type_id skr_create_record_type(const skr_guid_t* type_id, uint64_t size, uint64_t align, const skr_guid_t* parent);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_record_type_set_name(skr_dynamic_record_type_id type, const char* name);

RUNTIME_EXTERN_C RUNTIME_API 
void skr_record_type_set_hasher(skr_dynamic_record_type_id type, size_t (*hasher)(const void* self, size_t base));

// RUNTIME_EXTERN_C RUNTIME_API
// void skr_free_record_type(const skr_guid_t* type_id);

#ifdef __cplusplus
namespace skr {
namespace type {

template <class T> struct type_id;
template <class T> struct type_of;

} // namespace type
} // namespace skr

#define SKR_RTTI_DECLARE_TYPE(__NS, __T, __API) \
namespace skr::type \
{ \
    template<> \
    struct type_of<__NS::__T> \
    { \
        __API static const skr_type_t* get(); \
    }; \
    template<> \
    struct type_id<__NS::__T> \
    { \
        __API static const skr_guid_t get();\
    }; \
}

#endif