#pragma once
#include "SkrRT/config.h"
#include "SkrRTTR/trait.hpp"
#if !defined(__meta__)
    #include "ProxyTest/proxy_test.generated.h"
#endif

sreflect_struct(
    "guid" : "4c3cc343-70da-4225-b5be-79b0035f0d8e", 
    "proxy" : true
)
TestProxy {
    SKR_GENERATE_BODY()

    sattr("proxy": true)
    int  inc(int i) const noexcept;

    sattr("proxy::getter" : "a")
    int  get_a() const noexcept;

    sattr("proxy::setter" : "a")
    void set_a(int a) noexcept;
};

// util object
struct TestObject {
    int inc(int i) const noexcept { return i + a; }
    int a = 1233;
};

// pod with extend and override
struct TestPOD {
    int a = 1233;
};
inline int inc(const TestPOD* o, int i) noexcept
{
    return i + o->a;
}
inline int get_a(const TestPOD* o) noexcept
{
    return o->a + 1;
}