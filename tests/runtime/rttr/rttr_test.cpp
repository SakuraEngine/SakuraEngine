#include "SkrTestFramework/framework.hpp"
#include "SkrRT/rttr/type_registry.hpp"
#include "rttr_test_types.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/optr.hpp"

static void print_guid(const ::skr::GUID& g)
{
    printf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           g.Data1(), g.Data2(), g.Data3(), g.Data4(0), g.Data4(1), g.Data4(2), g.Data4(3), g.Data4(4), g.Data4(5), g.Data4(6), g.Data4(7));
}

// TODO. impl list
////  1. optr/SkrNewObj/SkrDeleteObj
////  2. record type test
//  3. enum type test
//  4. Vector, UMap, MultiUMap, USet, MultiUSet impl & test
//  5. TResourceHandle, SPtrHelper, StronglyEnum, variant impl & test
//

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