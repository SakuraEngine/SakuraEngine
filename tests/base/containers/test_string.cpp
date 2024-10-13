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

    SUBCASE("add")
    {
        String str = long_literal;

        // add
        str.add(u8'g');
        REQUIRE_EQ(str.size(), long_literal.size() + 1);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1));
        REQUIRE_EQ(str.at_buffer(long_literal.size()), u8'g');
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        REQUIRE_EQ(str.at_buffer(long_literal.size()), u8'g');

        // add unsafe
        str.add_unsafe(10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        REQUIRE_EQ(str.at_buffer(long_literal.size()), u8'g');
        // for (TestSizeType i = long_literal.size() + 1; i < long_literal.size() + 1 + 10; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }

        // add default
        str.add_default(10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10 + 10));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        REQUIRE_EQ(str.at_buffer(long_literal.size()), u8'g');
        // for (TestSizeType i = long_literal.size() + 1; i < long_literal.size() + 1 + 10; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }
        for (TestSizeType i = long_literal.size() + 1 + 10; i < long_literal.size() + 1 + 10 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }

        // add zeroed
        str.add_zeroed(10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10 + 10 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10 + 10 + 10));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        REQUIRE_EQ(str.at_buffer(long_literal.size()), u8'g');
        // for (TestSizeType i = long_literal.size() + 1; i < long_literal.size() + 1 + 10; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }
        for (TestSizeType i = long_literal.size() + 1 + 10; i < long_literal.size() + 1 + 10 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
        for (TestSizeType i = long_literal.size() + 1 + 10 + 10; i < long_literal.size() + 1 + 10 + 10 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
    }

    SUBCASE("add at")
    {
        String str = long_literal;

        // add at
        str.add_at(0, u8'g');
        REQUIRE_EQ(str.size(), long_literal.size() + 1);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1));
        REQUIRE_EQ(str.at_buffer(0), u8'g');
        for (TestSizeType i = 1; i < long_literal.size() + 1; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - 1));
        }

        // add at unsafe
        str.add_at_unsafe(0, 10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10));
        // for (TestSizeType i = 0; i < 10; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }
        REQUIRE_EQ(str.at_buffer(10), u8'g');
        for (TestSizeType i = 11; i < long_literal.size() + 1 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - 11));
        }

        // add at default
        str.add_at_default(0, 10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10 + 10));
        for (TestSizeType i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
        // for (TestSizeType i = 10; i < 20; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }
        REQUIRE_EQ(str.at_buffer(20), u8'g');
        for (TestSizeType i = 21; i < long_literal.size() + 1 + 10 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - 21));
        }

        // add at zeroed
        str.add_at_zeroed(0, 10);
        REQUIRE_EQ(str.size(), long_literal.size() + 1 + 10 + 10 + 10);
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + 1 + 10 + 10 + 10));
        for (TestSizeType i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
        for (TestSizeType i = 10; i < 20; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), 0);
        }
        // for (TestSizeType i = 20; i < 30; ++i)
        // {
        //     REQUIRE_EQ(str.at_buffer(i), 0);
        // }
        REQUIRE_EQ(str.at_buffer(30), u8'g');
        for (TestSizeType i = 31; i < long_literal.size() + 1 + 10 + 10 + 10; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - 31));
        }
    }

    SUBCASE("append")
    {
        StringView   append_view_a = short_literal;
        StringView   append_view_b = u8"🏀🐓";
        skr::UTF8Seq append_seq_a{ U'🏀' };

        String str = long_literal;

        // append
        auto len_before_append = str.length_buffer();
        str.append(append_view_a.data());
        auto len_after_append = str.length_buffer();
        REQUIRE_EQ(str.size(), len_before_append + append_view_a.size());
        REQUIRE_GE(str.capacity(), capacity_of(len_before_append + append_view_a.size()));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = len_before_append; i < len_after_append; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append));
        }

        // append with len
        auto len_before_append_len = str.length_buffer();
        auto append_len            = append_view_b.text_index_to_buffer(1);
        str.append(append_view_b.data(), append_len);
        auto len_after_append_len = str.length_buffer();
        REQUIRE_EQ(str.size(), len_before_append_len + append_len);
        REQUIRE_GE(str.capacity(), capacity_of(len_before_append_len + append_len));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = len_before_append; i < len_after_append; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append));
        }
        for (TestSizeType i = len_before_append_len; i < len_after_append_len; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - len_before_append_len));
        }

        // append view
        auto len_before_append_view = str.length_buffer();
        str.append(append_view_a);
        auto len_after_append_view = str.length_buffer();
        REQUIRE_EQ(str.size(), len_before_append_view + append_view_a.size());
        REQUIRE_GE(str.capacity(), capacity_of(len_before_append_view + append_view_a.size()));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = len_before_append; i < len_after_append; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append));
        }
        for (TestSizeType i = len_before_append_len; i < len_after_append_len; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - len_before_append_len));
        }
        for (TestSizeType i = len_before_append_view; i < len_after_append_view; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append_view));
        }

        // append seq
        auto len_before_append_seq = str.length_buffer();
        str.append(append_seq_a);
        auto len_after_append_seq = str.length_buffer();
        REQUIRE_EQ(str.size(), len_before_append_seq + append_seq_a.len);
        REQUIRE_GE(str.capacity(), capacity_of(len_before_append_seq + append_seq_a.len));
        for (TestSizeType i = 0; i < long_literal.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i));
        }
        for (TestSizeType i = len_before_append; i < len_after_append; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append));
        }
        for (TestSizeType i = len_before_append_len; i < len_after_append_len; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - len_before_append_len));
        }
        for (TestSizeType i = len_before_append_view; i < len_after_append_view; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - len_before_append_view));
        }
        for (TestSizeType i = len_before_append_seq; i < len_after_append_seq; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_seq_a.at(i - len_before_append_seq));
        }
    }

    SUBCASE("append at")
    {
        StringView   append_view_a = short_literal;
        StringView   append_view_b = u8"🏀🐓";
        skr::UTF8Seq append_seq_a{ U'🏀' };

        String str = long_literal;

        // append at
        str.append_at(0, append_view_a.data());
        auto append_at_begin = 0;
        auto append_at_end   = append_view_a.size();
        auto append_at_len   = str.length_buffer();
        REQUIRE_EQ(str.size(), long_literal.size() + append_view_a.size());
        REQUIRE_GE(str.capacity(), capacity_of(long_literal.size() + append_view_a.size()));
        for (TestSizeType i = append_at_begin; i < append_at_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_begin));
        }
        for (TestSizeType i = append_at_end; i < str.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - append_at_end));
        }

        // append at with len
        auto append_len = append_view_b.text_index_to_buffer(1);
        str.append_at(0, append_view_b.data(), append_len);
        auto append_at_len_begin = 0;
        auto append_at_len_end   = append_len;
        auto append_at_len_len   = str.length_buffer();
        append_at_begin += append_len;
        append_at_end += append_len;
        REQUIRE_EQ(str.size(), append_at_len + append_len);
        REQUIRE_GE(str.capacity(), capacity_of(append_at_len + append_len));
        for (TestSizeType i = append_at_len_begin; i < append_at_len_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - append_at_len_begin));
        }
        for (TestSizeType i = append_at_begin; i < append_at_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_begin));
        }
        for (TestSizeType i = append_at_end; i < str.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - append_at_end));
        }

        // append at view
        str.append_at(0, append_view_a);
        auto append_at_view_begin = 0;
        auto append_at_view_end   = append_view_a.size();
        auto append_at_view_len   = str.length_buffer();
        append_at_len_begin += append_view_a.size();
        append_at_len_end += append_view_a.size();
        append_at_begin += append_view_a.size();
        append_at_end += append_view_a.size();
        REQUIRE_EQ(str.size(), append_at_len_len + append_view_a.size());
        REQUIRE_GE(str.capacity(), capacity_of(append_at_len_len + append_view_a.size()));
        for (TestSizeType i = append_at_view_begin; i < append_at_view_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_view_begin));
        }
        for (TestSizeType i = append_at_len_begin; i < append_at_len_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - append_at_len_begin));
        }
        for (TestSizeType i = append_at_begin; i < append_at_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_begin));
        }
        for (TestSizeType i = append_at_end; i < str.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - append_at_end));
        }

        // append at seq
        str.append_at(0, append_seq_a);
        auto append_at_seq_begin = 0;
        auto append_at_seq_end   = append_seq_a.len;
        auto append_at_seq_len   = str.length_buffer();
        append_at_view_begin += append_seq_a.len;
        append_at_view_end += append_seq_a.len;
        append_at_len_begin += append_seq_a.len;
        append_at_len_end += append_seq_a.len;
        append_at_begin += append_seq_a.len;
        append_at_end += append_seq_a.len;
        REQUIRE_EQ(str.size(), append_at_view_len + append_seq_a.len);
        REQUIRE_GE(str.capacity(), capacity_of(append_at_view_len + append_seq_a.len));
        for (TestSizeType i = append_at_seq_begin; i < append_at_seq_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_seq_a.at(i - append_at_seq_begin));
        }
        for (TestSizeType i = append_at_view_begin; i < append_at_view_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_view_begin));
        }
        for (TestSizeType i = append_at_len_begin; i < append_at_len_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_b.at_buffer(i - append_at_len_begin));
        }
        for (TestSizeType i = append_at_begin; i < append_at_end; ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), append_view_a.at_buffer(i - append_at_begin));
        }
        for (TestSizeType i = append_at_end; i < str.size(); ++i)
        {
            REQUIRE_EQ(str.at_buffer(i), long_literal.at_buffer(i - append_at_end));
        }
    }

    SUBCASE("remove")
    {
        // remove at
        {
            StringView view              = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView remove_item_view  = u8"🐓";
            StringView removed_view_1    = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView remove_start_view = u8"🏀";
            StringView removed_view_2    = u8"🏀🏀🐓🏀🐓🏀🐓🏀🐓🏀";

            String str = view;

            // remove start
            str.remove_at(0, remove_item_view.size());
            REQUIRE_EQ(str.size(), removed_view_1.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_view_1);

            // remove center
            str.remove_at(remove_start_view.size(), remove_item_view.size());
            REQUIRE_EQ(str.size(), removed_view_2.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_view_2);
        }

        // [view] remove & remove last & remove all
        {
            StringView view              = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView remove_item_view  = u8"🐓";
            StringView removed_view      = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView removed_last_view = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🏀";
            StringView removed_all_view  = u8"🏀🏀🏀🏀🏀🏀";

            String str = view;

            // remove
            str.remove(remove_item_view);
            REQUIRE_EQ(str.size(), removed_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_view);

            // remove last
            str.remove_last(remove_item_view);
            REQUIRE_EQ(str.size(), removed_last_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_last_view);

            // remove all
            str.remove_all(remove_item_view);
            REQUIRE_EQ(str.size(), removed_all_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_all_view);
        }

        // [seq] remove & remove last & remove all
        {
            StringView   view              = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            skr::UTF8Seq remove_item_seq   = { U'🐓' };
            StringView   removed_view      = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView   removed_last_view = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🏀";
            StringView   removed_all_view  = u8"🏀🏀🏀🏀🏀🏀";

            String str = view;

            // remove
            str.remove(remove_item_seq);
            REQUIRE_EQ(str.size(), removed_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_view);

            // remove last
            str.remove_last(remove_item_seq);
            REQUIRE_EQ(str.size(), removed_last_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_last_view);

            // remove all
            str.remove_all(remove_item_seq);
            REQUIRE_EQ(str.size(), removed_all_view.size());
            REQUIRE_GE(str.capacity(), view.size());
            REQUIRE_EQ(str, removed_all_view);
        }

        // [view] [copy] remove & remove last & remove all
        {
            StringView view              = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView remove_item_view  = u8"🐓";
            StringView removed_view      = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView removed_last_view = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🏀";
            StringView removed_all_view  = u8"🏀🏀🏀🏀🏀🏀";

            String str = view;

            // remove
            auto removed = str.remove_copy(remove_item_view);
            REQUIRE_EQ(removed.size(), removed_view.size());
            REQUIRE_EQ(removed, removed_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            // remove last
            auto removed_last = str.remove_last_copy(remove_item_view);
            REQUIRE_EQ(removed_last.size(), removed_last_view.size());
            REQUIRE_EQ(removed_last, removed_last_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            // remove all
            auto removed_all = str.remove_all_copy(remove_item_view);
            REQUIRE_EQ(removed_all.size(), removed_all_view.size());
            REQUIRE_EQ(removed_all, removed_all_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);
        }

        // [seq] [copy] remove & remove last & remove all
        {
            StringView   view              = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            skr::UTF8Seq remove_item_seq   = { U'🐓' };
            StringView   removed_view      = u8"🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            StringView   removed_last_view = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🏀";
            StringView   removed_all_view  = u8"🏀🏀🏀🏀🏀🏀";

            String str = view;

            // remove
            auto removed = str.remove_copy(remove_item_seq);
            REQUIRE_EQ(removed.size(), removed_view.size());
            REQUIRE_EQ(removed, removed_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            // remove last
            auto removed_last = str.remove_last_copy(remove_item_seq);
            REQUIRE_EQ(removed_last.size(), removed_last_view.size());
            REQUIRE_EQ(removed_last, removed_last_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            // remove all
            auto removed_all = str.remove_all_copy(remove_item_seq);
            REQUIRE_EQ(removed_all.size(), removed_all_view.size());
            REQUIRE_EQ(removed_all, removed_all_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);
        }
    }

    SUBCASE("replace")
    {
        // replace
        {
            StringView view = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";

            StringView replace_less_from_view = u8"🐓";
            StringView replace_less_to_view   = u8"g";
            StringView replaced_less_view     = u8"g🏀g🏀g🏀g🏀g🏀g🏀";

            StringView replace_eq_from_view = u8"🏀";
            StringView replace_eq_to_view   = u8"🐓";
            StringView replaced_eq_view     = u8"🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓";

            StringView replace_more_from_view = u8"🐓";
            StringView replace_more_to_view   = u8"🐓鸡";
            StringView replaced_more_view     = u8"🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀";

            String str = view;
            str.replace(replace_less_from_view, replace_less_to_view);
            REQUIRE_EQ(str.size(), replaced_less_view.size());
            REQUIRE_EQ(str, replaced_less_view);

            str = view;
            str.replace(replace_eq_from_view, replace_eq_to_view);
            REQUIRE_EQ(str.size(), replaced_eq_view.size());
            REQUIRE_EQ(str, replaced_eq_view);

            str = view;
            str.replace(replace_more_from_view, replace_more_to_view);
            REQUIRE_EQ(str.size(), replaced_more_view.size());
            REQUIRE_EQ(str, replaced_more_view);
        }

        // [ranged] replace
        {
            StringView   view          = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            TestSizeType replace_start = view.text_index_to_buffer(2);
            TestSizeType replace_end   = view.text_index_to_buffer(view.length_text() - 2);
            TestSizeType replace_count = replace_end - replace_start;

            StringView replace_less_from_view = u8"🐓";
            StringView replace_less_to_view   = u8"g";
            StringView replaced_less_view     = u8"🐓🏀g🏀g🏀g🏀g🏀🐓🏀";

            StringView replace_eq_from_view = u8"🏀";
            StringView replace_eq_to_view   = u8"🐓";
            StringView replaced_eq_view     = u8"🐓🏀🐓🐓🐓🐓🐓🐓🐓🐓🐓🏀";

            StringView replace_more_from_view = u8"🐓";
            StringView replace_more_to_view   = u8"🐓鸡";
            StringView replaced_more_view     = u8"🐓🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓🏀";

            String str = view;
            str.replace(replace_less_from_view, replace_less_to_view, replace_start, replace_count);
            REQUIRE_EQ(str.size(), replaced_less_view.size());
            REQUIRE_EQ(str, replaced_less_view);

            str = view;
            str.replace(replace_eq_from_view, replace_eq_to_view, replace_start, replace_count);
            REQUIRE_EQ(str.size(), replaced_eq_view.size());
            REQUIRE_EQ(str, replaced_eq_view);

            str = view;
            str.replace(replace_more_from_view, replace_more_to_view, replace_start, replace_count);
            REQUIRE_EQ(str.size(), replaced_more_view.size());
            REQUIRE_EQ(str, replaced_more_view);
        }

        // [copy] replace
        {
            StringView view = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";

            StringView replace_less_from_view = u8"🐓";
            StringView replace_less_to_view   = u8"g";
            StringView replaced_less_view     = u8"g🏀g🏀g🏀g🏀g🏀g🏀";

            StringView replace_eq_from_view = u8"🏀";
            StringView replace_eq_to_view   = u8"🐓";
            StringView replaced_eq_view     = u8"🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓🐓";

            StringView replace_more_from_view = u8"🐓";
            StringView replace_more_to_view   = u8"🐓鸡";
            StringView replaced_more_view     = u8"🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀";

            String str           = view;
            auto   replaced_less = str.replace_copy(replace_less_from_view, replace_less_to_view);
            REQUIRE_EQ(replaced_less.size(), replaced_less_view.size());
            REQUIRE_EQ(replaced_less, replaced_less_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            auto replaced_eq = str.replace_copy(replace_eq_from_view, replace_eq_to_view);
            REQUIRE_EQ(replaced_eq.size(), replaced_eq_view.size());
            REQUIRE_EQ(replaced_eq, replaced_eq_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            auto replaced_more = str.replace_copy(replace_more_from_view, replace_more_to_view);
            REQUIRE_EQ(replaced_more.size(), replaced_more_view.size());
            REQUIRE_EQ(replaced_more, replaced_more_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);
        }

        // [copy] [ranged] replace
        {
            StringView   view          = u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀";
            TestSizeType replace_start = view.text_index_to_buffer(2);
            TestSizeType replace_end   = view.text_index_to_buffer(view.length_text() - 2);
            TestSizeType replace_count = replace_end - replace_start;

            StringView replace_less_from_view = u8"🐓";
            StringView replace_less_to_view   = u8"g";
            StringView replaced_less_view     = u8"🐓🏀g🏀g🏀g🏀g🏀🐓🏀";

            StringView replace_eq_from_view = u8"🏀";
            StringView replace_eq_to_view   = u8"🐓";
            StringView replaced_eq_view     = u8"🐓🏀🐓🐓🐓🐓🐓🐓🐓🐓🐓🏀";

            StringView replace_more_from_view = u8"🐓";
            StringView replace_more_to_view   = u8"🐓鸡";
            StringView replaced_more_view     = u8"🐓🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓鸡🏀🐓🏀";

            String str           = view;
            auto   replaced_less = str.replace_copy(replace_less_from_view, replace_less_to_view, replace_start, replace_count);
            REQUIRE_EQ(replaced_less.size(), replaced_less_view.size());
            REQUIRE_EQ(replaced_less, replaced_less_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            auto replaced_eq = str.replace_copy(replace_eq_from_view, replace_eq_to_view, replace_start, replace_count);
            REQUIRE_EQ(replaced_eq.size(), replaced_eq_view.size());
            REQUIRE_EQ(replaced_eq, replaced_eq_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);

            auto replaced_more = str.replace_copy(replace_more_from_view, replace_more_to_view, replace_start, replace_count);
            REQUIRE_EQ(replaced_more.size(), replaced_more_view.size());
            REQUIRE_EQ(replaced_more, replaced_more_view);
            REQUIRE_EQ(str.size(), view.size());
            REQUIRE_EQ(str, view);
        }
    }

    // index & modify

    // sub_string

    // find
    // contains & count

    // starts & ends
    // remove prefix & suffix
    // trim
    // trim invalid

    // partition
    // split

    // text index

    // iterators
}