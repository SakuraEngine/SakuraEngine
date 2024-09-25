#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"

TEST_CASE("Test U8StringView")
{
    using namespace skr::test_container;

    SUBCASE("ctor & dtor")
    {
        StringView empty;
        REQUIRE_EQ(empty.data(), nullptr);
        REQUIRE_EQ(empty.size(), 0);
        REQUIRE_EQ(empty.length_buffer(), 0);
        REQUIRE_EQ(empty.length_text(), 0);

        const skr_char8* text = u8"🐓鸡ĜG";
        StringView       auto_len{ text };
        REQUIRE_EQ(auto_len.data(), text);
        REQUIRE_EQ(auto_len.size(), 10);
        REQUIRE_EQ(auto_len.length_buffer(), 10);
        REQUIRE_EQ(auto_len.length_text(), 4);

        StringView manual_len{ text, 9 };
        REQUIRE_EQ(manual_len.data(), text);
        REQUIRE_EQ(manual_len.size(), 9);
        REQUIRE_EQ(manual_len.length_buffer(), 9);
        REQUIRE_EQ(manual_len.length_text(), 3);

        const Vector<char8_t> text_vector{ u8"🐓鸡ĜG", 10 };
        StringView            from_container{ text_vector };
        REQUIRE_EQ(from_container.data(), text_vector.data());
        REQUIRE_EQ(from_container.size(), 10);
        REQUIRE_EQ(from_container.length_buffer(), 10);
        REQUIRE_EQ(from_container.length_text(), 4);
    }

    // [needn't test] copy & move
    // [needn't test] assign & move assign

    SUBCASE("compare")
    {
        const skr_char8* text_a = u8"🐓鸡ĜG";
        const skr_char8* text_b = u8"🐓鸡Ĝ";
        const skr_char8* text_c = u8"GĜ鸡🐓";

        StringView a{ text_a };     // 🐓鸡ĜG
        StringView b{ text_a, 9 };  // 🐓鸡Ĝ
        StringView c{ text_b };     // 🐓鸡Ĝ
        StringView d{ text_a };     // 🐓鸡ĜG
        StringView e{ text_c + 6 }; // 🐓
        StringView f{ text_a, 4 };  // 🐓
        StringView empty{};
        StringView with_value_empty{ text_a, 0 };

        REQUIRE_EQ(a, a);
        REQUIRE_EQ(a, d);
        REQUIRE_EQ(b, c);
        REQUIRE_EQ(e, f);

        REQUIRE_NE(a, b);
        REQUIRE_NE(a, c);
        REQUIRE_NE(a, e);
        REQUIRE_NE(a, f);

        REQUIRE_NE(a, empty);
        REQUIRE_NE(a, with_value_empty);
        REQUIRE_EQ(empty, with_value_empty);
    }

    // [needn't test] getter

    SUBCASE("str get")
    {
        const skr_char8* text = u8"🐓鸡ĜG";
        StringView       a{ text };
        StringView       b{ text, 6 };
        StringView       c{ text + 1, 6 };

        REQUIRE_EQ(a.length_buffer(), 10);
        REQUIRE_EQ(a.length_text(), 4);

        REQUIRE_EQ(b.length_buffer(), 6);
        REQUIRE_EQ(b.length_text(), 3); // 2 bad ch in tail

        REQUIRE_EQ(c.length_buffer(), 6);
        REQUIRE_EQ(c.length_text(), 4); // 3 bad ch in head
    }

    // [needn't test] validator

    SUBCASE("index & modify")
    {
        const skr_char8*   text  = u8"🐓鸡ĜG";
        const skr::UTF8Seq seq[] = {
            { u8"🐓", 4 },
            { u8"鸡", 3 },
            { u8"Ĝ", 2 },
            { u8"G", 1 },
        };

        StringView view{ text };

        REQUIRE_EQ(view.at_buffer(0), text[0]);
        REQUIRE_EQ(view.at_buffer(1), text[1]);
        REQUIRE_EQ(view.at_buffer(2), text[2]);
        REQUIRE_EQ(view.at_buffer(3), text[3]);
        REQUIRE_EQ(view.at_buffer(4), text[4]);
        REQUIRE_EQ(view.at_buffer(5), text[5]);
        REQUIRE_EQ(view.at_buffer(6), text[6]);
        REQUIRE_EQ(view.at_buffer(7), text[7]);
        REQUIRE_EQ(view.at_buffer(8), text[8]);
        REQUIRE_EQ(view.at_buffer(9), text[9]);

        REQUIRE_EQ(view.last_buffer(0), text[9]);
        REQUIRE_EQ(view.last_buffer(1), text[8]);
        REQUIRE_EQ(view.last_buffer(2), text[7]);
        REQUIRE_EQ(view.last_buffer(3), text[6]);
        REQUIRE_EQ(view.last_buffer(4), text[5]);
        REQUIRE_EQ(view.last_buffer(5), text[4]);
        REQUIRE_EQ(view.last_buffer(6), text[3]);
        REQUIRE_EQ(view.last_buffer(7), text[2]);
        REQUIRE_EQ(view.last_buffer(8), text[1]);
        REQUIRE_EQ(view.last_buffer(9), text[0]);

        REQUIRE_EQ(view.at_text(0), seq[0]);
        REQUIRE_EQ(view.at_text(1), seq[0]);
        REQUIRE_EQ(view.at_text(2), seq[0]);
        REQUIRE_EQ(view.at_text(3), seq[0]);
        REQUIRE_EQ(view.at_text(4), seq[1]);
        REQUIRE_EQ(view.at_text(5), seq[1]);
        REQUIRE_EQ(view.at_text(6), seq[1]);
        REQUIRE_EQ(view.at_text(7), seq[2]);
        REQUIRE_EQ(view.at_text(8), seq[2]);
        REQUIRE_EQ(view.at_text(9), seq[3]);

        REQUIRE_EQ(view.last_text(0), seq[3]);
        REQUIRE_EQ(view.last_text(1), seq[2]);
        REQUIRE_EQ(view.last_text(2), seq[2]);
        REQUIRE_EQ(view.last_text(3), seq[1]);
        REQUIRE_EQ(view.last_text(4), seq[1]);
        REQUIRE_EQ(view.last_text(5), seq[1]);
        REQUIRE_EQ(view.last_text(6), seq[0]);
        REQUIRE_EQ(view.last_text(7), seq[0]);
        REQUIRE_EQ(view.last_text(8), seq[0]);
        REQUIRE_EQ(view.last_text(9), seq[0]);
    }

    SUBCASE("sub view")
    {
        StringView view{ u8"🐓鸡ĜG" };
        StringView sub_a = view.first(4);
        StringView sub_b = view.subview(0, 4);
        StringView sub_c = view.last(6);
        StringView sub_d = view.subview(4);
        StringView sub_e = view.subview(4, 3);

        REQUIRE_EQ(sub_a, sub_b);
        REQUIRE_EQ(sub_a, StringView{ u8"🐓" });
        REQUIRE_EQ(sub_c, sub_d);
        REQUIRE_EQ(sub_c, StringView{ u8"鸡ĜG" });
        REQUIRE_EQ(sub_e, StringView{ u8"鸡" });
    }

    SUBCASE("find")
    {
        StringView   view{ u8"This 🐓 is 🐓 a good 🐓 text 🐓" };
        StringView   find_view = u8"🐓";
        skr::UTF8Seq find_seq  = { u8"🐓", 4 };

        auto found_view      = view.find(find_view);
        auto found_seq       = view.find(find_seq);
        auto found_last_view = view.find_last(find_view);
        auto found_last_seq  = view.find_last(find_seq);

        REQUIRE(found_view);
        REQUIRE(found_seq);
        REQUIRE(found_last_view);
        REQUIRE(found_last_seq);

        REQUIRE_EQ(found_view.index(), 5);
        REQUIRE_EQ(found_seq.index(), 5);
        REQUIRE_EQ(found_last_view.index(), 35);
        REQUIRE_EQ(found_last_seq.index(), 35);
    }

    SUBCASE("contains & count")
    {
        StringView   view{ u8"This 🐓 is 🐓 a good 🐓 text 🐓" };
        StringView   none_view = u8"🐔";
        skr::UTF8Seq none_seq  = { u8"🐔", 4 };
        StringView   has_view  = u8"🐓";
        skr::UTF8Seq has_seq   = { u8"🐓", 4 };

        REQUIRE_FALSE(view.contains(none_view));
        REQUIRE_FALSE(view.contains(none_seq));
        REQUIRE(view.contains(has_view));
        REQUIRE(view.contains(has_seq));

        REQUIRE_EQ(view.count(none_view), 0);
        REQUIRE_EQ(view.count(none_seq), 0);
        REQUIRE_EQ(view.count(has_view), 4);
        REQUIRE_EQ(view.count(has_seq), 4);
    }

    SUBCASE("starts_with & ends_with")
    {
        StringView view_a{ u8"This is a good text" };
        StringView view_b{ u8"🐓🐓🐓 This is a good text 🐓🐓🐓" };

        skr::UTF8Seq seq = { u8"🐓", 4 };

        REQUIRE_FALSE(view_a.starts_with(u8"🐓🐓🐓"));
        REQUIRE_FALSE(view_a.ends_with(u8"🐓🐓🐓"));
        REQUIRE(view_b.starts_with(u8"🐓🐓🐓"));
        REQUIRE(view_b.ends_with(u8"🐓🐓🐓"));

        REQUIRE_FALSE(view_a.starts_with(seq));
        REQUIRE_FALSE(view_a.ends_with(seq));
        REQUIRE(view_b.starts_with(seq));
        REQUIRE(view_b.ends_with(seq));
    }

    SUBCASE("remove prefix & suffix")
    {
        StringView view_a{ u8"🐓🐓🐓 This is a good text 🐓🐓🐓" };
        StringView view_b{ u8" This is a good text 🐓🐓🐓" };
        StringView view_c{ u8"🐓🐓🐓 This is a good text " };
        StringView view_d{ u8" This is a good text " };
        StringView view_e{ u8"🐓🐓 This is a good text 🐓🐓🐓" };
        StringView view_f{ u8"🐓🐓🐓 This is a good text 🐓🐓" };
        StringView view_g{ u8"🐓🐓 This is a good text 🐓🐓" };

        skr::UTF8Seq seq = { u8"🐓", 4 };

        REQUIRE_EQ(view_a.remove_prefix(u8"🐓🐓🐓"), view_b);
        REQUIRE_EQ(view_a.remove_suffix(u8"🐓🐓🐓"), view_c);
        REQUIRE_EQ(view_a.remove_prefix(u8" This is a good text"), view_a);
        REQUIRE_EQ(view_a.remove_suffix(u8" This is a good text"), view_a);
        REQUIRE_EQ(view_g.remove_prefix(u8"🐓🐓🐓"), view_g);
        REQUIRE_EQ(view_g.remove_suffix(u8"🐓🐓🐓"), view_g);

        REQUIRE_EQ(view_a.remove_prefix(seq), view_e);
        REQUIRE_EQ(view_a.remove_suffix(seq), view_f);
        REQUIRE_EQ(view_d.remove_prefix(seq), view_d);
        REQUIRE_EQ(view_d.remove_suffix(seq), view_d);
    }

    SUBCASE("trim")
    {
        StringView trim_view{ u8" \tThis is a good text\t " };
        StringView trim_view_start{ u8"This is a good text\t " };
        StringView trim_view_end{ u8" \tThis is a good text" };
        StringView trim_view_full{ u8"This is a good text" };
        StringView trim_view_start_seq{ u8"\tThis is a good text\t " };
        StringView trim_view_end_seq{ u8" \tThis is a good text\t" };
        StringView trim_view_full_seq{ u8"\tThis is a good text\t" };
        StringView trim_view_empty{ u8" \t\t " };
        StringView empty{};

        skr::UTF8Seq seq = { u8" ", 1 };

        REQUIRE_EQ(trim_view.trim(), trim_view_full);
        REQUIRE_EQ(trim_view.trim_start(), trim_view_start);
        REQUIRE_EQ(trim_view.trim_end(), trim_view_end);
        REQUIRE_EQ(trim_view.trim(seq), trim_view_full_seq);
        REQUIRE_EQ(trim_view.trim_start(seq), trim_view_start_seq);
        REQUIRE_EQ(trim_view.trim_end(seq), trim_view_end_seq);
        REQUIRE_EQ(trim_view_empty.trim(), empty);

        StringView custom_trim_view{ u8"🐓🐓🐓 \tThis is a good text\t 🐓🐓🐓" };
        StringView custom_trim_view_start{ u8"This is a good text\t 🐓🐓🐓" };
        StringView custom_trim_view_end{ u8"🐓🐓🐓 \tThis is a good text" };
        StringView custom_trim_view_full{ u8"This is a good text" };
        StringView custom_trim_view_start_seq{ u8" \tThis is a good text\t 🐓🐓🐓" };
        StringView custom_trim_view_end_seq{ u8"🐓🐓🐓 \tThis is a good text\t " };
        StringView custom_trim_view_full_seq{ u8" \tThis is a good text\t " };
        StringView custom_trim_view_empty{ u8"🐓🐓🐓 \t\t 🐓🐓🐓" };

        StringView   custom_view = u8"🐓 \t";
        skr::UTF8Seq custom_seq  = { u8"🐓", 4 };

        REQUIRE_EQ(custom_trim_view.trim(custom_view), custom_trim_view_full);
        REQUIRE_EQ(custom_trim_view.trim_start(custom_view), custom_trim_view_start);
        REQUIRE_EQ(custom_trim_view.trim_end(custom_view), custom_trim_view_end);
        REQUIRE_EQ(custom_trim_view.trim(custom_seq), custom_trim_view_full_seq);
        REQUIRE_EQ(custom_trim_view.trim_start(custom_seq), custom_trim_view_start_seq);
        REQUIRE_EQ(custom_trim_view.trim_end(custom_seq), custom_trim_view_end_seq);
        REQUIRE_EQ(custom_trim_view_empty.trim(custom_view), empty);
    }

    SUBCASE("trim invalid")
    {
        StringView view{ u8"🐓🐓🐓 This is a good text 🐓🐓🐓" };
        StringView bad        = view.subview(1, view.size() - 2);
        StringView trim_start = bad.subview(3);
        StringView trim_end   = bad.subview(0, bad.size() - 3);
        StringView trim_full  = u8"🐓🐓 This is a good text 🐓🐓";

        REQUIRE_EQ(bad.trim_invalid(), trim_full);
        REQUIRE_EQ(bad.trim_invalid_start(), trim_start);
        REQUIRE_EQ(bad.trim_invalid_end(), trim_end);
    }

    SUBCASE("partition")
    {
        StringView view{ u8"This is a 🐓 good text" };
        StringView left{ u8"This is a " };
        StringView mid{ u8"🐓" };
        StringView right{ u8" good text" };
        auto [result_left, result_mid, result_right] = view.partition(u8"🐓");
        REQUIRE_EQ(result_left, left);
        REQUIRE_EQ(result_mid, mid);
        REQUIRE_EQ(result_right, right);

        // not found
        StringView not_found_view{ u8"This is a good text" };
        auto [not_found_left, not_found_mid, not_found_right] = not_found_view.partition(u8"🐓");
        REQUIRE_EQ(not_found_left, not_found_view);
        REQUIRE(not_found_mid.is_empty());
        REQUIRE(not_found_right.is_empty());

        // first
        StringView first_view{ u8"🐓 This is a good text" };
        auto [first_left, first_mid, first_right] = first_view.partition(u8"🐓");
        REQUIRE(first_left.is_empty());
        REQUIRE_EQ(first_mid, u8"🐓");
        REQUIRE_EQ(first_right, StringView{ u8" This is a good text" });

        // last
        StringView last_view{ u8"This is a good text 🐓" };
        auto [last_left, last_mid, last_right] = last_view.partition(u8"🐓");
        REQUIRE_EQ(last_left, StringView{ u8"This is a good text " });
        REQUIRE_EQ(last_mid, u8"🐓");
        REQUIRE(last_right.is_empty());
    }

    SUBCASE("split")
    {
        StringView view{ u8"This 🐓 is 🐓🐓 a good 🐓 text 🐓" };
        StringView split_view{ u8"🐓" };
        StringView split_result[] = {
            u8"This ",
            u8" is ",
            u8"",
            u8" a good ",
            u8" text ",
        };
        StringView split_result_cull_empty[] = {
            u8"This ",
            u8" is ",
            u8" a good ",
            u8" text ",
        };

        // split to container
        {
            Vector<StringView> result;
            uint64_t           count;

            result.clear();
            count = view.split(result, split_view);
            REQUIRE_EQ(count, 5);
            for (uint64_t i = 0; i < count; ++i)
            {
                REQUIRE_EQ(result[i], split_result[i]);
            }

            result.clear();
            count = view.split(result, split_view, true);
            REQUIRE_EQ(count, 4);
            for (uint64_t i = 0; i < count; ++i)
            {
                REQUIRE_EQ(result[i], split_result_cull_empty[i]);
            }

            result.clear();
            count = view.split(result, split_view, false, 3);
            REQUIRE_EQ(count, 3);
            for (uint64_t i = 0; i < count; ++i)
            {
                REQUIRE_EQ(result[i], split_result[i]);
            }

            result.clear();
            count = view.split(result, split_view, true, 3);
            REQUIRE_EQ(count, 3);
            for (uint64_t i = 0; i < count; ++i)
            {
                REQUIRE_EQ(result[i], split_result_cull_empty[i]);
            }
        }

        // custom split
        {
            uint64_t count, idx;

            idx   = 0;
            count = view.split_each(
            [&](const StringView& v) {
                REQUIRE_EQ(v, split_result[idx]);
                ++idx;
            },
            split_view);
            REQUIRE_EQ(count, 5);

            idx   = 0;
            count = view.split_each(
            [&](const StringView& v) {
                REQUIRE_EQ(v, split_result_cull_empty[idx]);
                ++idx;
            },
            split_view,
            true);
            REQUIRE_EQ(count, 4);

            idx   = 0;
            count = view.split_each(
            [&](const StringView& v) {
                REQUIRE_EQ(v, split_result[idx]);
                ++idx;
            },
            split_view,
            false,
            3);
            REQUIRE_EQ(count, 3);

            idx   = 0;
            count = view.split_each(
            [&](const StringView& v) {
                REQUIRE_EQ(v, split_result_cull_empty[idx]);
                ++idx;
            },
            split_view,
            true,
            3);
            REQUIRE_EQ(count, 3);
        }
    }

    SUBCASE("text index")
    {
        StringView view{ u8"🐓鸡ĜG" };

        REQUIRE_EQ(view.buffer_index_to_text(0), 0);
        REQUIRE_EQ(view.buffer_index_to_text(1), 0);
        REQUIRE_EQ(view.buffer_index_to_text(2), 0);
        REQUIRE_EQ(view.buffer_index_to_text(3), 0);
        REQUIRE_EQ(view.buffer_index_to_text(4), 1);
        REQUIRE_EQ(view.buffer_index_to_text(5), 1);
        REQUIRE_EQ(view.buffer_index_to_text(6), 1);
        REQUIRE_EQ(view.buffer_index_to_text(7), 2);
        REQUIRE_EQ(view.buffer_index_to_text(8), 2);
        REQUIRE_EQ(view.buffer_index_to_text(9), 3);

        REQUIRE_EQ(view.text_index_to_buffer(0), 0);
        REQUIRE_EQ(view.text_index_to_buffer(1), 4);
        REQUIRE_EQ(view.text_index_to_buffer(2), 7);
        REQUIRE_EQ(view.text_index_to_buffer(3), 9);

        StringView bad_view = view.subview(1, view.size() - 5);

        REQUIRE_EQ(bad_view.buffer_index_to_text(0), 0);
        REQUIRE_EQ(bad_view.buffer_index_to_text(1), 1);
        REQUIRE_EQ(bad_view.buffer_index_to_text(2), 2);
        REQUIRE_EQ(bad_view.buffer_index_to_text(3), 3);
        REQUIRE_EQ(bad_view.buffer_index_to_text(4), 4);

        REQUIRE_EQ(bad_view.text_index_to_buffer(0), 0);
        REQUIRE_EQ(bad_view.text_index_to_buffer(1), 1);
        REQUIRE_EQ(bad_view.text_index_to_buffer(2), 2);
        REQUIRE_EQ(bad_view.text_index_to_buffer(3), 3);
        REQUIRE_EQ(bad_view.text_index_to_buffer(4), 4);
    }

    SUBCASE("cursor & iterator")
    {
        StringView   view{ u8"🐓鸡ĜG" };
        skr::UTF8Seq each_result[] = {
            { u8"🐓", 4 },
            { u8"鸡", 3 },
            { u8"Ĝ", 2 },
            { u8"G", 1 },
        };

        uint64_t idx;

        // cursor
        {
            idx = 0;
            for (auto cursor = view.cursor_begin(); !cursor.reach_end(); cursor.move_next())
            {
                REQUIRE_EQ(cursor.ref(), each_result[idx]);
                ++idx;
            }
            REQUIRE_EQ(idx, 4);

            idx = 4;
            for (auto cursor = view.cursor_end(); !cursor.reach_begin(); cursor.move_prev())
            {
                --idx;
                REQUIRE_EQ(cursor.ref(), each_result[idx]);
            }
            REQUIRE_EQ(idx, 0);
        }

        // iter
        {
            idx = 0;
            for (auto iter = view.iter(); iter.has_next(); iter.move_next())
            {
                REQUIRE_EQ(iter.ref(), each_result[idx]);
                ++idx;
            }
            REQUIRE_EQ(idx, 4);

            idx = 4;
            for (auto iter = view.iter_inv(); iter.has_next(); iter.move_next())
            {
                --idx;
                REQUIRE_EQ(iter.ref(), each_result[idx]);
            }
            REQUIRE_EQ(idx, 0);
        }

        // range
        {
            idx = 0;
            for (auto v : view.range())
            {
                REQUIRE_EQ(v, each_result[idx]);
                ++idx;
            }
            REQUIRE_EQ(idx, 4);

            idx = 4;
            for (auto v : view.range_inv())
            {
                --idx;
                REQUIRE_EQ(v, each_result[idx]);
            }
            REQUIRE_EQ(idx, 0);
        }
    }
}