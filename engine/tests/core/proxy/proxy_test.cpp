#include "SkrTestFramework/framework.hpp"
#include "proxy_test.hpp"

template<skr::Proxyable<TestProxy> T>
void test_concept(T o)
{
    REQUIRE_EQ(o.inc(0), 1233);
    TestProxy t = o;
    REQUIRE_EQ(t.inc(0), 1233);
}

TEST_CASE("simple trait test")
{
    SUBCASE("member function")
    {
        TestObject o;
        REQUIRE_EQ(o.inc(0), 1233);
        TestProxy t = o;
        REQUIRE_EQ(t.inc(0), 1233);
    }
    SUBCASE("free function")
    {
        TestPOD o;
        REQUIRE_EQ(inc(&o, 0), 1233);
        TestProxy t = o;

        REQUIRE_EQ(t.inc(0), 1233);
    }
    SUBCASE("property")
    {
        TestObject o;
        REQUIRE_EQ(o.a, 1233);
        TestProxy t = o;
        REQUIRE_EQ(t.get_a(), 1233);
        t.set_a(1033);
        REQUIRE_EQ(o.a, 1033);
        REQUIRE_EQ(t.get_a(), 1033);
    }

    SUBCASE("property override")
    {
        TestPOD o;
        REQUIRE_EQ(o.a, 1233);
        TestProxy t = o;
        REQUIRE_EQ(t.get_a(), 1234);
    }

    SUBCASE("concept")
    {
        TestObject o;
        test_concept(o);
    }
}