#include "SkrTestFramework/framework.hpp"
#include "SkrRT/rttr/type_registry.hpp"
#include "rttr_test_types.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/optr.hpp"

static void print_guid(const ::skr::GUID& g)
{
    printf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           g.data1(), g.data2(), g.data3(), g.data4(0), g.data4(1), g.data4(2), g.data4(3), g.data4(4), g.data4(5), g.data4(6), g.data4(7));
}

// TODO. impl list
////  1. optr/SkrNewObj/SkrDeleteObj
////  2. record type test
//  3. enum type test
//  4. Vector, UMap, MultiUMap, USet, MultiUSet impl & test
//  5. TResourceHandle, SPtrHelper, TEnumAsByte, variant impl & test

// TODO. test list
// 1. record type test(cast, field, method)
// 2. pointer, array, reference type
// 3. enum type
// 4. Vector, UMap, UMultiMap, USet, UMultiSet

TEST_CASE("test record type")
{
    using namespace skr::rttr;
    using namespace skr_rttr_test;

    SUBCASE("cast")
    {
        // OPtr<Maxwell> p_maxwell = SkrNewObj<Maxwell>();
        // OPtr<Dog>     p_dog     = p_maxwell;
        // OPtr<Animal>  p_animal  = p_maxwell;
        // OPtr<IWolf>   p_wolf    = p_maxwell;

        // REQUIRE_EQ(p_dog.type_cast<Maxwell>(), p_maxwell);
        // REQUIRE_EQ(p_animal.type_cast<Maxwell>(), p_maxwell);
        // REQUIRE_EQ(p_wolf.type_cast<Maxwell>(), p_maxwell);

        // SkrDeleteObj(p_maxwell);
    }

    SUBCASE("field")
    {
        // OPtr<Maxwell> p_maxwell = SkrNewObj<Maxwell>();
        // RecordType*   type      = (RecordType*)p_maxwell.get_type();
        // for (const auto& field_pair : type->fields())
        // {
        //     printf("field name: %s\n", field_pair.key.c_str());
        // }
    }

    SUBCASE("method")
    {
    }
}