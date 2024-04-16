#pragma once
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrRT/rttr/guid.hpp"

namespace skr::script
{
struct RecordData;
struct MethodData;
struct FieldData;
struct StaticMethodData;
struct StaticFieldData;
struct FunctionData;
struct ParamData;
struct TypeIdentifier;

struct TypeIdentifier {
    GUID type_id;
    bool is_const;
    bool is_ref;
    bool is_rvalue_ref;
    bool is_pointer;
};

struct ParamData {
    using MakeDefaultFunc = void (*)(void*);

    TypeIdentifier  type;
    String          name;
    bool            is_out;
    MakeDefaultFunc make_default;
};

struct FunctionData {
    // signature
    String            name;
    Vector<String>    name_space;
    TypeIdentifier    ret_type;
    Vector<ParamData> param_data;
    void*             ptr;

    // call & user data
    void* call;
    void* user_data;
};

struct FieldData {
    // signature
    String         name;
    TypeIdentifier type;
    uint32_t       array_dim;
    uint32_t       offset;

    // user data
    void* getter;
    void* setter;
    void* user_data;
};

struct MethodData {
    // signature
    String                name;
    TypeIdentifier        ret_type;
    Vector<ParamData>     param_data;
    AlignedStorage<16, 8> ptr; // assume that member function SIZE <= 16 and ALIGN <= 8

    // call & user data
    void* call;
    void* user_data;
};

struct StaticFieldData {
    // signature
    String         name;
    TypeIdentifier type;
    uint32_t       array_dim;

    // user data
    void* user_data;
};

struct StaticMethodData : public FunctionData {
    // signature
    String            name;
    TypeIdentifier    ret_type;
    Vector<ParamData> param_data;
    void*             ptr;

    // call & user data
    void* call;
    void* user_data;
};

struct RecordData {
    // class basic
    String               name;
    Vector<String>       name_space;
    GUID                 type_id;
    GUID                 super_id;
    Vector<FunctionData> ctor_data;
    FunctionData         dtor_data;

    // method & fields
    Vector<MethodData> methods;
    Vector<FieldData>  fields;

    // static fields & static methods
    Vector<StaticMethodData> static_methods;
    Vector<StaticFieldData>  static_fields;

    // user data
    void* user_data;
};

} // namespace skr::script