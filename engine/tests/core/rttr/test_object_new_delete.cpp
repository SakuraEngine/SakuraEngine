#include "SkrTestFramework/framework.hpp"
#include "SkrRTTR/export/export_builder.hpp"
#include <SkrBase/type_info.hpp>
#include "./test_rttr_types.hpp"

TEST_CASE("test object new/delete")
{
    using namespace skr;
    using namespace test_rttr;

    IRTTRBasic* as_obj = SkrNew<TestDerivedC>();
    ITestInterfaceA* as_interface_a = SkrNew<TestDerivedC>();
    ITestInterfaceB* as_interface_b = SkrNew<TestDerivedC>();
    TestDerivedA* as_derived_a = SkrNew<TestDerivedC>();
    TestDerivedB* as_derived_b = SkrNew<TestDerivedC>();

    SkrDelete(as_obj);
    SkrDelete(as_interface_a);
    SkrDelete(as_interface_b);
    SkrDelete(as_derived_a);
    SkrDelete(as_derived_b);
}