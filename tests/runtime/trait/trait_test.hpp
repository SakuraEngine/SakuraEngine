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
};

struct myobject
{
    int inc(int i) const noexcept { return i + 1233; }
};


struct myobject2
{
};
inline int inc(const myobject2*, int i) noexcept { return i + 1233; }