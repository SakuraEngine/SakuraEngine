#pragma once
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_handle.h"
#include "SkrContainers/string.hpp"
#include "SkrContainers/variant.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/config.h"

#if !defined(__meta__)
    #include "RTTRTest/types.generated.h"
#endif

// TODO: Move To Core
struct SRuntimeAttribute {
};

template <typename T>
void XXXInformation()
{
    // const auto type = skr::type::type_of<T>::get();
    // SKR_TEST_INFO(u8"Static Ctor: {}", type->Name());
}

void PrintField(const char* name);

inline void CreateBuffers()
{
}

using EnumValue = uint32_t;

namespace Types sreflect
{

sreflect_enum("guid" : "1a0b91c7-6690-41d6-acfd-0c2b61f181f3")
sattr("serialize" : ["json", "bin"])
TestEnum : uint32_t
{
    Value0 = 0,
    Value1 = 1,
    Value2 = 2
} sstatic_ctor(XXXInformation<$T>());

sreflect_struct("guid" : "25809bab-41e8-48c5-806b-1ae4af3edfef")
TestObject : skr::SInterface {
};

sreflect_struct("guid" : "25809bab-41e8-48c5-806b-1ae4af3edfef")
TestParent {
    skr::String buffer;
} sstatic_ctor(CreateBuffers());

sreflect_struct("guid" : "d55175b2-9d7f-47b8-bccd-a06aeac55240")
TestSon : public TestParent {
    uint32_t                                   a; // SKR_TYPE_CATEGORY_U32
    skr_float2_t                               b; // SKR_TYPE_CATEGORY_F32_2
    double                                     c; // SKR_TYPE_CATEGORY_F64
    skr_resource_handle_t                      d; // SKR_TYPE_CATEGORY_HANDLE
    skr::resource::TResourceHandle<TestObject> e; // SKR_TYPE_CATEGORY_HANDLE
    TestEnum                                   f; // SKR_TYPE_CATEGORY_ENUM
    // skr::Vector<TestEnum> g; // SKR_TYPE_CATEGORY_DYNARR
    // skr::span<TestEnum> h; // SKR_TYPE_CATEGORY_ARRV
    TestEnum                           i[10]; // SKR_TYPE_CATEGORY_ARR
    skr::StringView                   k;     // SKR_TYPE_CATEGORY_STRV
    TestParent*                        l;     // SKR_TYPE_CATEGORY_REF
    skr::SPtr<TestParent>              m;     // SKR_TYPE_CATEGORY_REF
    TestParent&                        n;     // SKR_TYPE_CATEGORY_REF
    skr::variant<TestEnum, skr_guid_t> o;     // SKR_TYPE_CATEGORY_VARIANT
    skr::SPtr<TestObject>              p;     // SKR_TYPE_CATEGORY_REF
    TestObject*                        q;     // SKR_TYPE_CATEGORY_REF
    // skr::Vector<skr::span<skr::variant<skr::SPtr<TestObject>, TestParent*>>> r;
} sstatic_ctor(XXXInformation<$T>());

sreflect_struct("guid" : "17D3EC00-2817-4EAE-8BAF-976BF4124ABF")
sattr("serialize" : ["json", "bin"])
TestSerde {
    int i;
    // skr::Vector<skr::variant<uint32_t, skr::String, TestEnum>> c;
};

} // namespace Types sreflect

template <typename T>
struct XXXInformationAttribute : public SRuntimeAttribute {
};

inline static void DontCreateObject()
{
}