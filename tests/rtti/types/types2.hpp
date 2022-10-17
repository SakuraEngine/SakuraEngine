#include "types.hpp"

namespace Types sreflect
{
    
sreflect_struct("guid" : "3557e486-e9e3-43a2-99bb-756aa3a09746")
sattr("rtti" : true)
TestGrandSon : public TestSon
{
    sstatic_ctor(0, PrintField($name))
    eastl::string job;
}
sstatic_ctor(0, XXXInformation<$T>());

}