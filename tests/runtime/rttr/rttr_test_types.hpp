#pragma once
#include "SkrRT/platform/configure.h"
#if !defined(__meta__)
    #include "RTTRTest/rttr_test_types.generated.h"
#endif

namespace skr_rttr_test sreflect
{
sreflect_struct("guid" : "43e1909f-3da2-4030-8450-9eed6d1823eb")
sattr("rtti": true)
Animal{

};

sreflect_struct("guid" : "46583ac4-d82c-4c44-84e0-67a2ebebfc13")
sattr("rtti": true)
Cat : public Animal{

};

sreflect_struct("guid" : "4228ea2d-c60d-41f8-bfea-69ca01ffd7b2")
sattr("rtti": true)
Dog : public Animal{

};

sreflect_struct("guid" : "4bb7cbef-1d2d-4892-b5cf-88a7dbc6d958")
sattr("rtti": true)
Duck : public Animal{

};

sreflect_struct("guid": "88e524ad-4229-4e3d-9fb8-3bff9220868b")
sattr("rtti": true)
Maxwell : public Dog
{
    sattr("rtti": true)
    sattr("fuck": 100) 
    void no_good(int a, bool b, Dog* c) {}

private:
    float  party_animals;
    int    luisa_compute;
    double lian_quan_da_jiang_hu;
};
} // namespace skr_rttr_test sreflect