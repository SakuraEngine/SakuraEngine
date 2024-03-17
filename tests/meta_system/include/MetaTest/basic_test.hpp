#pragma once
#include "SkrBase/meta.h"
// #ifndef __meta__
//     #include "MetaTest/test_check_structure.generated.h"
// #endif

namespace test_path_shorthand sreflect
{
// pass check
// sreflect_struct(
//     "test_expand_path": {
//         "a": "expand_a",
//         "b::b": "expand_b",
//         "c::c::c": "expand_c"
//     }
// ) UsualExpand {
// };
} // namespace test_path_shorthand sreflect

namespace test_functional_shorthand sreflect
{
// pass check
sreflect_struct("test_functional_shorthand": "all")
UsualShorthand {
};

// expand with bad type
sreflect_struct("test_functional_shorthand": "bad_type")
BadTypeShorthand {
};

// unrecognized shorthand

// expand with path

// expand with bad path

// expand with override

// expand with bad override

} // namespace test_functional_shorthand sreflect

namespace test_override sreflect
{
// pass check
sreflect_struct(
    "test_check_override::test_override": "base_value",
    "test_check_override::test_override!": "override",
    "test_check_override!::test_override!": "override",
    "test_check_override!::test_override": "override"
)
UsualOverride {
};

// error: override value without '!' mark
// sreflect_struct(
//     "test_check_override::test_override": "test",
//     "test_check_override::test_override": "override"
// )
// LostOverrideMark {
// };

// pass check
sreflect_struct(
)
UsualAppend {
};

} // namespace test_override sreflect

namespace test_structure_check sreflect
{
// test type error
// sreflect_struct(
//     "test_check_structure": {
//         "int": false,
//         "float": "f",
//         "str": 100,
//         "bool": "shit",
//         "sub" : 10
//     },
//     "test_check_structure::sub::count": "100"
// )
// TestCheckStructure {
// };

// test unrecognized attr
// sreflect_struct(
//     "test_unrecognized_attr": {
//         "a": "a",
//         "b": "b",
//         "c": "c",
//         "sub": {
//             "sub_a": "sub_a",
//             "sub_b": "sub_b",
//             "sub_c": "sub_c"
//         },
//         "sub::sub_a!": "sub_a!",
//         "SuB::sUb_A": "FuCK",
//         "sub::sub_bbbb": "sub_bbb"
//     }
// ) TestUnrecognizedAttr {
// };
} // namespace test_structure_check sreflect
