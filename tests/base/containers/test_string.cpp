#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"

namespace skr::test_container
{
constexpr TestSizeType kStringSSOCapacity = kStringSSOSize - 1;
inline bool            is_literal(const String& str)
{
    return str.memory().is_literal();
}
inline bool allow_literal(const StringView& view)
{
    return skr::in_const_segment(view.data()) && view.data()[view.size()] == u8'\0';
}
inline TestSizeType capacity_of(TestSizeType capacity)
{
    return std::max(kStringSSOCapacity, capacity);
}
inline TestSizeType capacity_of(const StringView& view)
{
    return allow_literal(view) ? view.size() : capacity_of(view.size());
}
} // namespace skr::test_container

TEST_CASE("Test U8String")
{
    using namespace skr::test_container;

    // public test literals
    StringView        short_literal{ u8"🐓鸡ĜG" };
    Vector<skr_char8> short_buffer_vec{ short_literal.data(), short_literal.size() + 1 };
    StringView        short_buffer{ short_buffer_vec.data() };
    StringView        long_literal{ u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀" };
    Vector<skr_char8> long_buffer_vec{ long_literal.data(), long_literal.size() + 1 };
    StringView        long_buffer{ long_buffer_vec.data() };

    SUBCASE("ctor & dtor")
    {
        String empty;
        REQUIRE_EQ(empty.size(), 0);
        REQUIRE_EQ(empty.capacity(), kStringSSOCapacity);
        REQUIRE_FALSE(is_literal(empty));

        auto test_ctors_of_view = [](const StringView& view) {
            auto length_text = view.length_text();
            SKR_ASSERT(length_text > 1 && "for test subview ctor, view length must > 1");

            // ctor
            String ctor{ view.data() };
            REQUIRE_EQ(ctor.size(), view.size());
            REQUIRE_EQ(ctor.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(ctor), allow_literal(view));

            // len ctor
            String ctor_with_len{ view.data(), view.size() };
            REQUIRE_EQ(ctor_with_len.size(), view.size());
            REQUIRE_EQ(ctor_with_len.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(ctor_with_len), allow_literal(view));

            // view ctor
            String view_ctor{ view };
            REQUIRE_EQ(view_ctor.size(), view.size());
            REQUIRE_EQ(view_ctor.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(view_ctor), allow_literal(view));

            if (allow_literal(view))
            {
                auto subview_end_with_zero     = view.subview(view.text_index_to_buffer(1));
                auto subview_not_end_with_zero = view.subview(0, view.text_index_to_buffer(length_text - 1));

                // len ctor that end with '\0'
                String ctor_with_len_zero_ending{ subview_end_with_zero.data(), subview_end_with_zero.size() };
                REQUIRE_EQ(ctor_with_len_zero_ending.size(), subview_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_zero_ending.capacity(), capacity_of(subview_end_with_zero));
                REQUIRE_EQ(is_literal(ctor_with_len_zero_ending), allow_literal(subview_end_with_zero));

                // len ctor that not end with '\0'
                String ctor_with_len_not_zero_ending{ subview_not_end_with_zero.data(), subview_not_end_with_zero.size() };
                REQUIRE_EQ(ctor_with_len_not_zero_ending.size(), subview_not_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_not_zero_ending.capacity(), capacity_of(subview_not_end_with_zero));
                REQUIRE_EQ(is_literal(ctor_with_len_not_zero_ending), allow_literal(subview_not_end_with_zero));

                // view ctor that end with '\0'
                String view_ctor_zero_ending{ subview_end_with_zero };
                REQUIRE_EQ(view_ctor_zero_ending.size(), subview_end_with_zero.size());
                REQUIRE_EQ(view_ctor_zero_ending.capacity(), capacity_of(subview_end_with_zero));
                REQUIRE_EQ(is_literal(view_ctor_zero_ending), allow_literal(subview_end_with_zero));

                // view ctor that not end with '\0'
                String view_ctor_not_zero_ending{ subview_not_end_with_zero };
                REQUIRE_EQ(view_ctor_not_zero_ending.size(), subview_not_end_with_zero.size());
                REQUIRE_EQ(view_ctor_not_zero_ending.capacity(), capacity_of(subview_not_end_with_zero));
                REQUIRE_EQ(is_literal(view_ctor_not_zero_ending), allow_literal(subview_not_end_with_zero));
            }
        };

        test_ctors_of_view(short_literal);
        test_ctors_of_view(short_buffer);
        test_ctors_of_view(long_literal);
        test_ctors_of_view(long_buffer);
    }

    SUBCASE("copy & move")
    {
        String literal_a = short_literal;
        String sso_a     = short_buffer;
        String heap_a    = long_buffer;

        // copy literal
        String literal_b = literal_a;
        REQUIRE_EQ(literal_b.size(), literal_a.size());
        REQUIRE_EQ(literal_b.capacity(), literal_a.capacity());
        REQUIRE_EQ(literal_b, literal_a);
        REQUIRE_EQ(literal_b.data(), literal_a.data());
        REQUIRE(is_literal(literal_a));
        REQUIRE(is_literal(literal_b));

        // copy sso
        String sso_b = sso_a;
        REQUIRE_EQ(sso_b.size(), sso_a.size());
        REQUIRE_EQ(sso_b.capacity(), sso_a.capacity());
        REQUIRE_EQ(sso_b, sso_a);
        REQUIRE_NE(sso_b.data(), sso_a.data());

        // copy heap
        String heap_b = heap_a;
        REQUIRE_EQ(heap_b.size(), heap_a.size());
        REQUIRE_EQ(heap_b.capacity(), heap_a.capacity());
        REQUIRE_EQ(heap_b, heap_a);
        REQUIRE_NE(heap_b.data(), heap_a.data());

        // move literal
        auto   old_literal_a_size     = literal_a.size();
        auto   old_literal_a_capacity = literal_a.capacity();
        auto   old_literal_a_data     = literal_a.data();
        String literal_c              = std::move(literal_a);
        REQUIRE_EQ(literal_a.size(), 0);
        REQUIRE_EQ(literal_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(literal_c.size(), old_literal_a_size);
        REQUIRE_EQ(literal_c.capacity(), old_literal_a_capacity);
        REQUIRE_EQ(literal_c.data(), old_literal_a_data);
        REQUIRE(is_literal(literal_c));
        REQUIRE(literal_a.is_empty());

        // move sso
        auto   old_sso_a_size     = sso_a.size();
        auto   old_sso_a_capacity = sso_a.capacity();
        String sso_c              = std::move(sso_a);
        REQUIRE_EQ(sso_a.size(), 0);
        REQUIRE_EQ(sso_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(sso_c.size(), old_sso_a_size);
        REQUIRE_EQ(sso_c.capacity(), old_sso_a_capacity);
        REQUIRE(sso_a.is_empty());

        // move heap
        auto   old_heap_a_size     = heap_a.size();
        auto   old_heap_a_capacity = heap_a.capacity();
        auto   old_heap_a_data     = heap_a.data();
        String heap_c              = std::move(heap_a);
        REQUIRE_EQ(heap_a.size(), 0);
        REQUIRE_EQ(heap_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(heap_c.size(), old_heap_a_size);
        REQUIRE_EQ(heap_c.capacity(), old_heap_a_capacity);
        REQUIRE_EQ(heap_c.data(), old_heap_a_data);
        REQUIRE(heap_a.is_empty());
    }

    SUBCASE("assign & move assign")
    {
        String literal_a = short_literal;
        String literal_b;
        String literal_c;
        String sso_a = short_buffer;
        String sso_b;
        String sso_c;
        String heap_a = long_buffer;
        String heap_b;
        String heap_c;

        // assign literal
        literal_b = literal_a;
        REQUIRE_EQ(literal_b.size(), literal_a.size());
        REQUIRE_EQ(literal_b.capacity(), literal_a.capacity());
        REQUIRE_EQ(literal_b, literal_a);
        REQUIRE_EQ(literal_b.data(), literal_a.data());
        REQUIRE(is_literal(literal_a));
        REQUIRE(is_literal(literal_b));

        // assign sso
        sso_b = sso_a;
        REQUIRE_EQ(sso_b.size(), sso_a.size());
        REQUIRE_EQ(sso_b.capacity(), sso_a.capacity());
        REQUIRE_EQ(sso_b, sso_a);
        REQUIRE_NE(sso_b.data(), sso_a.data());

        // assign heap
        heap_b = heap_a;
        REQUIRE_EQ(heap_b.size(), heap_a.size());
        // REQUIRE_EQ(heap_b.capacity(), heap_a.capacity());
        REQUIRE_EQ(heap_b, heap_a);
        REQUIRE_NE(heap_b.data(), heap_a.data());

        // move assign literal
        auto old_literal_a_size     = literal_a.size();
        auto old_literal_a_capacity = literal_a.capacity();
        auto old_literal_a_data     = literal_a.data();
        literal_c                   = std::move(literal_a);
        REQUIRE_EQ(literal_a.size(), 0);
        REQUIRE_EQ(literal_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(literal_c.size(), old_literal_a_size);
        REQUIRE_EQ(literal_c.capacity(), old_literal_a_capacity);
        REQUIRE_EQ(literal_c.data(), old_literal_a_data);
        REQUIRE(is_literal(literal_c));
        REQUIRE(literal_a.is_empty());

        // move assign sso
        auto old_sso_a_size     = sso_a.size();
        auto old_sso_a_capacity = sso_a.capacity();
        sso_c                   = std::move(sso_a);
        REQUIRE_EQ(sso_a.size(), 0);
        REQUIRE_EQ(sso_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(sso_c.size(), old_sso_a_size);
        REQUIRE_EQ(sso_c.capacity(), old_sso_a_capacity);
        REQUIRE(sso_a.is_empty());

        // move assign heap
        auto old_heap_a_size     = heap_a.size();
        auto old_heap_a_capacity = heap_a.capacity();
        auto old_heap_a_data     = heap_a.data();
        heap_c                   = std::move(heap_a);
        REQUIRE_EQ(heap_a.size(), 0);
        REQUIRE_EQ(heap_a.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(heap_c.size(), old_heap_a_size);
        REQUIRE_EQ(heap_c.capacity(), old_heap_a_capacity);
        REQUIRE_EQ(heap_c.data(), old_heap_a_data);
        REQUIRE(heap_a.is_empty());
    }

    SUBCASE("special assign")
    {
        auto test_assign_of_view = [](const StringView& view) {
            auto length_text = view.length_text();
            SKR_ASSERT(length_text > 1 && "for test subview ctor, view length must > 1");

            // ctor
            String ctor;
            ctor.assign(view.data());
            REQUIRE_EQ(ctor.size(), view.size());
            REQUIRE_EQ(ctor.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(ctor), allow_literal(view));

            // len ctor
            String ctor_with_len;
            ctor_with_len.assign(view.data(), view.size());
            REQUIRE_EQ(ctor_with_len.size(), view.size());
            REQUIRE_EQ(ctor_with_len.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(ctor_with_len), allow_literal(view));

            // view ctor
            String view_ctor;
            view_ctor.assign(view);
            REQUIRE_EQ(view_ctor.size(), view.size());
            REQUIRE_EQ(view_ctor.capacity(), capacity_of(view));
            REQUIRE_EQ(is_literal(view_ctor), allow_literal(view));

            if (allow_literal(view))
            {
                auto subview_end_with_zero     = view.subview(view.text_index_to_buffer(1));
                auto subview_not_end_with_zero = view.subview(0, view.text_index_to_buffer(length_text - 1));

                // len ctor that end with '\0'
                String ctor_with_len_zero_ending;
                ctor_with_len_zero_ending.assign(subview_end_with_zero.data(), subview_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_zero_ending.size(), subview_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_zero_ending.capacity(), capacity_of(subview_end_with_zero));
                REQUIRE_EQ(is_literal(ctor_with_len_zero_ending), allow_literal(subview_end_with_zero));

                // len ctor that not end with '\0'
                String ctor_with_len_not_zero_ending;
                ctor_with_len_not_zero_ending.assign(subview_not_end_with_zero.data(), subview_not_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_not_zero_ending.size(), subview_not_end_with_zero.size());
                REQUIRE_EQ(ctor_with_len_not_zero_ending.capacity(), capacity_of(subview_not_end_with_zero));
                REQUIRE_EQ(is_literal(ctor_with_len_not_zero_ending), allow_literal(subview_not_end_with_zero));

                // view ctor that end with '\0'
                String view_ctor_zero_ending;
                view_ctor_zero_ending.assign(subview_end_with_zero);
                REQUIRE_EQ(view_ctor_zero_ending.size(), subview_end_with_zero.size());
                REQUIRE_EQ(view_ctor_zero_ending.capacity(), capacity_of(subview_end_with_zero));
                REQUIRE_EQ(is_literal(view_ctor_zero_ending), allow_literal(subview_end_with_zero));

                // view ctor that not end with '\0'
                String view_ctor_not_zero_ending;
                view_ctor_not_zero_ending.assign(subview_not_end_with_zero);
                REQUIRE_EQ(view_ctor_not_zero_ending.size(), subview_not_end_with_zero.size());
                REQUIRE_EQ(view_ctor_not_zero_ending.capacity(), capacity_of(subview_not_end_with_zero));
                REQUIRE_EQ(is_literal(view_ctor_not_zero_ending), allow_literal(subview_not_end_with_zero));
            }
        };

        test_assign_of_view(short_literal);
        test_assign_of_view(short_buffer);
        test_assign_of_view(long_literal);
        test_assign_of_view(long_buffer);
    }

    SUBCASE("compare")
    {
        // test equal
        {
            String a = short_literal;
            String b = short_literal;
            String c = short_buffer;
            String d = long_literal;
            String e = long_literal;
            String f = long_buffer;

            REQUIRE_EQ(a, b);
            REQUIRE_EQ(a, c);
            REQUIRE_EQ(b, c);
            REQUIRE_EQ(d, e);
            REQUIRE_EQ(d, f);
            REQUIRE_EQ(e, f);
        }

        // test not equal
        {
            String a = short_literal;
            String b = short_literal.subview(short_literal.text_index_to_buffer(1));
            String c = short_literal.subview(0, short_literal.text_index_to_buffer(short_buffer.length_text() - 1));
            String d = long_literal;
            String e = long_literal.subview(long_literal.text_index_to_buffer(1));
            String f = long_literal.subview(0, long_literal.text_index_to_buffer(long_buffer.length_text() - 1));

            REQUIRE_NE(a, b);
            REQUIRE_NE(a, c);
            REQUIRE_NE(b, c);
            REQUIRE_NE(d, e);
            REQUIRE_NE(d, f);
            REQUIRE_NE(e, f);
        }
    }

    // [needn't test] getter

    SUBCASE("str getter")
    {
        String a = short_literal;
        String b = short_buffer;
        String c = long_literal;
        String d = long_buffer;

        REQUIRE_EQ(a.length_text(), short_literal.length_text());
        REQUIRE_EQ(a.length_buffer(), short_literal.length_buffer());
        REQUIRE_EQ(b.length_text(), short_buffer.length_text());
        REQUIRE_EQ(b.length_buffer(), short_buffer.length_buffer());
        REQUIRE_EQ(c.length_text(), long_literal.length_text());
        REQUIRE_EQ(c.length_buffer(), long_literal.length_buffer());
        REQUIRE_EQ(d.length_text(), long_buffer.length_text());
        REQUIRE_EQ(d.length_buffer(), long_buffer.length_buffer());

        REQUIRE_EQ(a.c_str(), a.data());
        REQUIRE_EQ(b.c_str(), b.data());
        REQUIRE_EQ(c.c_str(), c.data());
        REQUIRE_EQ(d.c_str(), d.data());

        REQUIRE_EQ(a.c_str_raw(), reinterpret_cast<const char*>(a.data()));
        REQUIRE_EQ(b.c_str_raw(), reinterpret_cast<const char*>(b.data()));
        REQUIRE_EQ(c.c_str_raw(), reinterpret_cast<const char*>(c.data()));
        REQUIRE_EQ(d.c_str_raw(), reinterpret_cast<const char*>(d.data()));
    }

    // [needn't test] validate

    SUBCASE("memory op")
    {
        String str = long_literal;

        // literal clear
        str.clear();
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), kStringSSOCapacity);
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // buffer clear
        str = long_buffer;
        str.clear();
        auto old_capacity = str.capacity();
        auto old_data     = str.data();
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(old_capacity));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_EQ(str.data(), old_data);
        REQUIRE_FALSE(is_literal(str));

        // release
        str.release();
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(0));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // release with content
        str = long_literal;
        str.release();
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(0));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // release with reserve size
        str = long_literal;
        str.release(10086);
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(10086));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // reserve
        str.release();
        str.reserve(60);
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(60));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // shrink
        str.shrink();
        REQUIRE(str.is_empty());
        REQUIRE_EQ(str.capacity(), capacity_of(0));
        REQUIRE_EQ(str.size(), 0);
        REQUIRE_FALSE(is_literal(str));

        // shrink with content
        str = long_literal;
        str.reserve(10086);
        str.shrink();
        REQUIRE_EQ(str.size(), long_literal.size());
        REQUIRE_EQ(str.capacity(), capacity_of(long_literal));
        REQUIRE_EQ(str, long_literal);

        // resize
        str.release();
        str = long_literal;
        str.resize(200, u8'g');
        REQUIRE_EQ(str.size(), 200);
        REQUIRE_GE(str.capacity(), capacity_of(200));
        REQUIRE_FALSE(is_literal(str));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = long_literal.size(); i < 200; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), u8'g');
        }

        // resize unsafe
        str.release();
        str = long_literal;
        str.resize_unsafe(200);
        REQUIRE_EQ(str.size(), 200);
        REQUIRE_GE(str.capacity(), capacity_of(200));
        REQUIRE_FALSE(is_literal(str));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }

        // resize default
        str.release();
        str = long_literal;
        str.resize_default(200);
        REQUIRE_EQ(str.size(), 200);
        REQUIRE_GE(str.capacity(), capacity_of(200));
        REQUIRE_FALSE(is_literal(str));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = long_literal.size(); i < 200; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }

        // resize zeroed
        str.release();
        str = long_literal;
        str.resize_zeroed(200);
        REQUIRE_EQ(str.size(), 200);
        REQUIRE_GE(str.capacity(), capacity_of(200));
        REQUIRE_FALSE(is_literal(str));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = long_literal.size(); i < 200; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
    }

    // add

    // add at
}