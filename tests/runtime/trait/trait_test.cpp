#include "SkrTestFramework/framework.hpp"
#include "trait_test.hpp"


TEST_CASE("simple trait test")
{
    SUBCASE("member function")
    {
        myobject o;
        REQUIRE_EQ(o.inc(0), 1233);
        mytrait t = o;
        REQUIRE_EQ(t.inc(0), 1233);
    }
    SUBCASE("free function")
    {
        myobject2 o;
        REQUIRE_EQ(inc(&o, 0), 1233);
        mytrait t = o;
        REQUIRE_EQ(t.inc(0), 1233);
    }
    SUBCASE("property")
    {
        myobject o;
        REQUIRE_EQ(o.a, 1233);
        mytrait t = o;
        REQUIRE_EQ(t.get_a(), 1233);
        t.set_a(1033);
        REQUIRE_EQ(o.a, 1033);
        REQUIRE_EQ(t.get_a(), 1033);
    }
    
    SUBCASE("property override")
    {
        myobject2 o;
        REQUIRE_EQ(o.a, 1233);
        mytrait t = o;
        REQUIRE_EQ(t.get_a(), 1234);
    }
}