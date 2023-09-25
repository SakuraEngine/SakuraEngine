#include "SkrTestFramework/framework.hpp"
#include "SkrRT/rttr/type_registry.hpp"
#include "rttr_test_types.hpp"
#include "SkrRT/rttr/type/record_type.hpp"

static void print_guid(const ::skr::GUID& g)
{
    printf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           g.data1(), g.data2(), g.data3(), g.data4(0), g.data4(1), g.data4(2), g.data4(3), g.data4(4), g.data4(5), g.data4(6), g.data4(7));
}

TEST_CASE("test rttr")
{
    using namespace skr::rttr;
    using namespace skr_rttr_test;
    auto maxwell_type = static_cast<RecordType*>(type_of<Maxwell>());
    for (const auto& [guid, base] : maxwell_type->base_types())
    {
        printf("%s [", base._type->name().c_str());
        print_guid(guid);
        printf("]\n");
    }

    for (const auto& [name, field] : maxwell_type->fields())
    {
        printf("%s : %s [", name.c_str(), field.type->name().c_str());
        print_guid(field.type->type_id());
        printf("](%d)\n", field.offset);
    }

    auto arr_type = type_of<int[10][100][114514]>();
    printf("%s\n", arr_type->name().c_str());

    auto pointer_type = type_of<const volatile Maxwell**>();
    printf("%s\n", pointer_type->name().c_str());

    auto reference_type = type_of<int&>();
    printf("%s\n", reference_type->name().c_str());
}