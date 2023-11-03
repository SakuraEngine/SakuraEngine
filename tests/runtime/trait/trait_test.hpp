#pragma once
#include "SkrRT/platform/configure.h"
#include "SkrRT/rttr/trait.hpp"
#if !defined(__meta__)
    #include "TraitTest/trait_test.generated.h"
#endif

sreflect_struct("guid" : "5CACA8C3-4F59-4814-8B0F-E6328FA0D361", "trait" : true)
mytrait
{
    GENERATED_TRAIT_BODY(mytrait)

    int getA() const;
};

struct myobject
{
    int getA() const { return 1233; }
};


struct myobject2
{
};
inline int getA(const myobject2*) { return 1233; }