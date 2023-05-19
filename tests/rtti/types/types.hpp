#pragma once
#include "platform/configure.h"
#include "misc/log.hpp"
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
    auto registry = skr::type::GetTypeRegistry();
    const auto tid = skr::type::type_id<T>::get();
    auto type = registry->get_type(tid);
    SKR_LOG_FMT_DEBUG(u8"Static Ctor: {} with id {}", type->Name(), tid);
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
TestEnum : uint32_t
{
    Value0 = 0, 
    Value1 = 1, 
    Value2 = 2 
}
sstatic_ctor(XXXInformation<$T>());

sreflect_struct("guid" : "25809bab-41e8-48c5-806b-1ae4af3edfef")
sattr("rtti" : true)
sattr("serialize" : ["json", "bin"])
TestParent
{
    skr::string buffer;
}
sstatic_ctor(CreateBuffers());

sreflect_struct("guid" : "d55175b2-9d7f-47b8-bccd-a06aeac55240")
sattr("rtti" : true)
sattr("serialize" : ["json", "bin"])
TestSon : public TestParent
{
    skr::string sex;
}
sstatic_ctor(XXXInformation<$T>());

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