#pragma once
#include "containers/detail/sptr.hpp"
#include "containers/variant.hpp"
#include "SkrRT/platform/configure.h"
#include "misc/log.hpp"
#include "resource/config_resource.h"
#include "resource/resource_handle.h"
#include "type/type.hpp"
#include <containers/string.hpp>
#include <EASTL/unique_ptr.h>
#include "RTTITestTypes/module.configure.h"
#if !defined(__meta__)
    #include "RTTITestTypes/types.generated.h"
#endif

// TODO: Move To Core 
struct SRuntimeAttribute
{

}; 

template<typename T>
void XXXInformation()
{
    const auto type = skr::type::type_of<T>::get();
    SKR_LOG_FMT_DEBUG(u8"Static Ctor: {}", type->Name());
}

RTTI_TEST_TYPES_API void PrintField(const char* name);

inline void CreateBuffers()
{

}

using EnumValue = uint32_t;

namespace Types sreflect
{

sreflect_enum("guid" : "1a0b91c7-6690-41d6-acfd-0c2b61f181f3")
sattr("rtti" : true)
sattr("serialize" : ["json", "bin"])
TestEnum : uint32_t
{
    Value0 = 0, 
    Value1 = 1, 
    Value2 = 2 
}
sstatic_ctor(XXXInformation<$T>());

sreflect_struct("guid" : "25809bab-41e8-48c5-806b-1ae4af3edfef")
sattr("rtti" : true)
TestObject : skr::SInterface
{

};

sreflect_struct("guid" : "25809bab-41e8-48c5-806b-1ae4af3edfef")
sattr("rtti" : true)
TestParent
{
    skr::string buffer;
}
sstatic_ctor(CreateBuffers());

sreflect_struct("guid" : "d55175b2-9d7f-47b8-bccd-a06aeac55240")
sattr("rtti" : true) 
TestSon : public TestParent
{
    uint32_t a; // SKR_TYPE_CATEGORY_U32
    skr_float2_t b; // SKR_TYPE_CATEGORY_F32_2
    double c; // SKR_TYPE_CATEGORY_F64
    skr_resource_handle_t d; // SKR_TYPE_CATEGORY_HANDLE
    skr::resource::TResourceHandle<TestObject> e; // SKR_TYPE_CATEGORY_HANDLE
    TestEnum f; // SKR_TYPE_CATEGORY_ENUM
    skr::vector<TestEnum> g; // SKR_TYPE_CATEGORY_DYNARR
    skr::span<TestEnum> h; // SKR_TYPE_CATEGORY_ARRV
    TestEnum i[10]; // SKR_TYPE_CATEGORY_ARR
    skr::string_view k; // SKR_TYPE_CATEGORY_STRV
    TestParent* l; // SKR_TYPE_CATEGORY_REF
    skr::SPtr<TestParent> m; // SKR_TYPE_CATEGORY_REF
    TestParent& n; // SKR_TYPE_CATEGORY_REF
    skr::variant<TestEnum, skr_guid_t> o; // SKR_TYPE_CATEGORY_VARIANT
    skr::SPtr<TestObject> p; // SKR_TYPE_CATEGORY_REF
    TestObject* q; // SKR_TYPE_CATEGORY_REF
    skr::vector<skr::span<skr::variant<skr::SPtr<TestObject>, TestParent*>>> r;
}
sstatic_ctor(XXXInformation<$T>());

sreflect_struct("guid" : "17D3EC00-2817-4EAE-8BAF-976BF4124ABF")
sattr("rtti" : true)
sattr("serialize" : ["json", "bin"])
TestSerde
{
    skr::vector<skr::variant<uint32_t, skr::string, TestEnum>> c;
};

}

template<typename T>
struct XXXInformationAttribute : public SRuntimeAttribute
{

};

template <>
struct eastl::default_delete<struct SRuntimeAttribue>
{
    void operator()(struct SRuntimeAttribue* p) const
    {

    }
};


inline static void DontCreateObject()
{

}