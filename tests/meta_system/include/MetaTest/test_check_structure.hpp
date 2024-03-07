#pragma once
#include "SkrBase/meta.h"
// #ifndef __meta__
//     #include "MetaTest/test_check_structure.generated.h"
// #endif

namespace test_ns sreflect
{
sreflect_struct("test": true) TestRecord {
    sattr("test": true)
    int test_field;

    sattr("test": true)
    int test_method(int sattr("test": true) test_param);
};

sreflect sattr("test": true) int test_function(int sattr("test": true) test_param);

sreflect_enum_class("test": true) TestEnum
{
    TestValue sattr("test": true)
};
} // namespace test_ns sreflect
