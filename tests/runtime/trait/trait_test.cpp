#include "SkrTestFramework/framework.hpp"
#include "trait_test.hpp"


TEST_CASE("simple trait test")
{
    SUBCASE("member function")
    {
        myobject o;
        REQUIRE_EQ(o.getA(), 1233);
        mytrait t = o;
        REQUIRE_EQ(t.getA(), 1233);
    }
    SUBCASE("free function")
    {
        myobject2 o;
        REQUIRE_EQ(getA(&o), 1233);
        mytrait t = o;
        REQUIRE_EQ(t.getA(), 1233);
    }
}