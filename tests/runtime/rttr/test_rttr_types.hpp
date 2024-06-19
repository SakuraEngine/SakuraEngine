#pragma once
#include "SkrRT/config.h"
#include "SkrRTTR/iobject.hpp"
#if !defined(__meta__)
    #include "RTTRTest/test_rttr_types.generated.h"
#endif

// TODO. basic test
//  1. reflect record
//      a. bases
//      b. fields
//      c. static fields
//      d. methods
//      e. static methods
//  2. reflect enum
//  3. flags
//  4. attrs

// TODO. test reflect bases and exclude bases

// TODO. test invoke & field visit

// TODO. default value

namespace test_rttr
{
sreflect_struct("guid" : "43e1909f-3da2-4030-8450-9eed6d1823eb")
Animal : virtual public ::skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct("guid" : "46583ac4-d82c-4c44-84e0-67a2ebebfc13")
Cat : public Animal {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct("guid" : "4228ea2d-c60d-41f8-bfea-69ca01ffd7b2")
Dog : public Animal {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct("guid" : "4bb7cbef-1d2d-4892-b5cf-88a7dbc6d958")
Duck : public Animal {
    SKR_RTTR_GENERATE_BODY()
};

sreflect_struct("guid": "5189a020-c3e6-449f-970d-4a7411c8fde3")
IWolf : virtual public ::skr::rttr::IObject {
    SKR_RTTR_GENERATE_BODY()

    virtual ~IWolf() = default;
    void test_a() {}
    void test_b() {}
};

sreflect_struct("guid": "88e524ad-4229-4e3d-9fb8-3bff9220868b")
Maxwell : public Dog,
          public IWolf {
    SKR_RTTR_GENERATE_BODY()

    sattr("rttr": true)
    void        no_good(int a, bool b, Dog* c) {}

    sattr("rttr": true)
    static void no_good(int a = 0, float b = 114.514f) {}

    sattr("rttr": true)
    float       party_animals;

    int          luisa_compute;

    sattr("rttr": true)
    double       lian_quan_da_jiang_hu;

    sattr("rttr": true)
    static float fuck;
};
} // namespace test_rttr