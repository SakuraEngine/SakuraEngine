#include "rtti-test-types/module.configure.h"
#include "types.hpp"
#include "types2.hpp"

RTTI_TEST_TYPES_API void PrintField(const char* name)
{
    printf(name ? name : "types.cpp\n");
}