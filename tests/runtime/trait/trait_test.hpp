#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/rttr/trait.hpp"
#if !defined(__meta__)
    #include "TraitTest/trait_test.generated.h"
#endif

sreflect_struct("guid" : "5CACA8C3-4F59-4814-8B0F-E6328FA0D361", "trait" : true)
mytrait
{
    TRAIT_MIXIN(mytrait)

    int inc(int i) const noexcept;

    sattr("getter" : "a")
    int get_a() const noexcept;

    sattr("setter" : "a")
    void set_a(int a) noexcept;
};

// class type
struct myobject
{
    int inc(int i) const noexcept { return i + a; }
    int a = 1233;
};

// pod type
struct myobject2
{
    int a = 1233;
};
//extend myobject2
inline int inc(const myobject2* o, int i) noexcept { return i + o->a; }
//override get_a
inline int get_a(const myobject2* o) noexcept { return o->a + 1; }