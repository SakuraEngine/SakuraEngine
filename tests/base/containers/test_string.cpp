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
inline TestSizeType capacity_of(const StringView& view)
{
    return allow_literal(view) ? view.size() : std::max(kStringSSOCapacity, view.size());
}
} // namespace skr::test_container

TEST_CASE("Test U8String")
{
    using namespace skr::test_container;

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

        StringView        short_literal{ u8"🐓鸡ĜG" };
        Vector<skr_char8> short_buffer_vec{ short_literal.data(), short_literal.size() + 1 };
        StringView        short_buffer{ short_buffer_vec.data() };
        StringView        long_literal{ u8"🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀🐓🏀" };
        Vector<skr_char8> long_buffer_vec{ long_literal.data(), long_literal.size() + 1 };
        StringView        long_buffer{ long_buffer_vec.data() };
        test_ctors_of_view(short_literal);
        test_ctors_of_view(short_buffer);
        test_ctors_of_view(long_literal);
        test_ctors_of_view(long_buffer);
    }
}