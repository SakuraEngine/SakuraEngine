#pragma once
#include "types.hpp"
#if !defined(__meta__)
    #include "RTTITestTypes/types2.generated.h"
#endif

namespace Types sreflect
{
    
sreflect_struct("guid" : "3557e486-e9e3-43a2-99bb-756aa3a09746")
sattr("rtti" : true)
TestGrandSon : public TestSon
{
    sstatic_ctor(PrintField($name))
    skr::string job;
}
sstatic_ctor(XXXInformation<$T>())
sattr("serialize" : ["json", "bin"])
sattr("component" : true);

}