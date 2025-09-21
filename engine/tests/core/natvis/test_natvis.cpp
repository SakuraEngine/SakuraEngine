#include "SkrTestFramework/framework.hpp"
#include "SkrCore/log.hpp"

// basic types
#include "SkrBase/types.h"

// containers
#include "SkrContainers/vector.hpp"
#include "SkrContainers/sparse_vector.hpp"
#include "SkrContainers/set.hpp"
#include "SkrContainers/multi_set.hpp"
#include "SkrContainers/map.hpp"
#include "SkrContainers/multi_map.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/optional.hpp"
#include "SkrContainers/hashmap.hpp"

inline void test_basic_types()
{
    using namespace skr;
    using namespace skr::literals;

    // guid
    GUID test_guid = u8"bc91a90c-be2b-46b7-9f07-5c931b1f2eab"_guid;
    test_guid            = GUID::Create();
    SKR_LOG_FMT_INFO(u8"test_guid: {}", test_guid);
}

inline void test_vector()
{
    using namespace skr;

    // vector
    Vector<uint32_t> test         = { 1, 1, 4, 5, 1, 4 };
    Vector<uint32_t> test_empty   = {};
    Vector<uint32_t> test_cleared = { 1, 1, 4, 5, 1, 4 };
    test_cleared.clear();

    // fixed vector
    FixedVector<uint32_t, 6> test_fixed         = { 1, 1, 4, 5, 1, 4 };
    FixedVector<uint32_t, 6> test_fixed_empty   = {};
    FixedVector<uint32_t, 6> test_fixed_cleared = { 1, 1, 4, 5, 1, 4 };
    test_fixed_cleared.clear();

    // inline vector
    InlineVector<uint32_t, 2> test_inline         = { 1, 1, 4, 5, 1, 4 };
    InlineVector<uint32_t, 2> test_inline_empty   = {};
    InlineVector<uint32_t, 2> test_inline_cleared = { 1, 1, 4, 5, 1, 4 };
    test_inline_cleared.clear();
}

inline void test_sparse_vector()
{
    using namespace skr;

    // sparse vector
    SparseVector<uint32_t> test         = { 1, 1, 4, 5, 1, 4 };
    SparseVector<uint32_t> test_empty   = {};
    SparseVector<uint32_t> test_cleared = { 1, 1, 4, 5, 1, 4 };
    test_cleared.clear();
    SparseVector<uint32_t> test_hole = { 1, 1, 4, 5, 1, 4 };
    test_hole.remove_at(1);
    test_hole.remove_at(3);
    test_hole.remove_at(5);

    // fixed sparse vector
    FixedSparseVector<uint32_t, 6> test_fixed         = { 1, 1, 4, 5, 1, 4 };
    FixedSparseVector<uint32_t, 6> test_fixed_empty   = {};
    FixedSparseVector<uint32_t, 6> test_fixed_cleared = { 1, 1, 4, 5, 1, 4 };
    test_fixed_cleared.clear();
    FixedSparseVector<uint32_t, 6> test_fixed_hole = { 1, 1, 4, 5, 1, 4 };
    test_fixed_hole.remove_at(1);
    test_fixed_hole.remove_at(3);
    test_fixed_hole.remove_at(5);

    // inline sparse vector
    InlineSparseVector<uint32_t, 2> test_inline         = { 1, 1, 4, 5, 1, 4 };
    InlineSparseVector<uint32_t, 2> test_inline_empty   = {};
    InlineSparseVector<uint32_t, 2> test_inline_cleared = { 1, 1, 4, 5, 1, 4 };
    test_inline_cleared.clear();
    InlineSparseVector<uint32_t, 2> test_inline_hole = { 1, 1, 4, 5, 1, 4 };
    test_inline_hole.remove_at(1);
    test_inline_hole.remove_at(3);
    test_inline_hole.remove_at(5);
}

inline void test_set()
{
    using namespace skr;

    // set
    Set<uint32_t> test         = { 1, 1, 4, 5, 1, 4 };
    Set<uint32_t> test_empty   = {};
    Set<uint32_t> test_cleared = { 1, 1, 4, 5, 1, 4 };
    test_cleared.clear();
    Set<uint32_t> test_hole = { 1, 1, 4, 5, 1, 4 };
    test_hole.remove(1);

    // fixed set
    FixedSet<uint32_t, 6> test_fixed         = { 1, 1, 4, 5, 1, 4 };
    FixedSet<uint32_t, 6> test_fixed_empty   = {};
    FixedSet<uint32_t, 6> test_fixed_cleared = { 1, 1, 4, 5, 1, 4 };
    test_fixed_cleared.clear();
    FixedSet<uint32_t, 6> test_fixed_hole = { 1, 1, 4, 5, 1, 4 };
    test_fixed_hole.remove(1);

    // inline set
    InlineSet<uint32_t, 2> test_inline         = { 1, 1, 4, 5, 1, 4 };
    InlineSet<uint32_t, 2> test_inline_empty   = {};
    InlineSet<uint32_t, 2> test_inline_cleared = { 1, 1, 4, 5, 1, 4 };
    test_inline_cleared.clear();
    InlineSet<uint32_t, 2> test_inline_hole = { 1, 1, 4, 5, 1, 4 };
    test_inline_hole.remove(1);
}

inline void test_multi_set()
{
    using namespace skr;

    // multi set
    MultiSet<uint32_t> test         = { 1, 1, 4, 5, 1, 4 };
    MultiSet<uint32_t> test_empty   = {};
    MultiSet<uint32_t> test_cleared = { 1, 1, 4, 5, 1, 4 };
    test_cleared.clear();
    MultiSet<uint32_t> test_hole = { 1, 1, 4, 5, 1, 4 };
    test_hole.remove(1);

    // fixed multi set
    FixedMultiSet<uint32_t, 6> test_fixed         = { 1, 1, 4, 5, 1, 4 };
    FixedMultiSet<uint32_t, 6> test_fixed_empty   = {};
    FixedMultiSet<uint32_t, 6> test_fixed_cleared = { 1, 1, 4, 5, 1, 4 };
    test_fixed_cleared.clear();
    FixedMultiSet<uint32_t, 6> test_fixed_hole = { 1, 1, 4, 5, 1, 4 };
    test_fixed_hole.remove(1);

    // inline multi set
    InlineMultiSet<uint32_t, 2> test_inline         = { 1, 1, 4, 5, 1, 4 };
    InlineMultiSet<uint32_t, 2> test_inline_empty   = {};
    InlineMultiSet<uint32_t, 2> test_inline_cleared = { 1, 1, 4, 5, 1, 4 };
    test_inline_cleared.clear();
    InlineMultiSet<uint32_t, 2> test_inline_hole = { 1, 1, 4, 5, 1, 4 };
    test_inline_hole.remove(1);
}

inline void test_map()
{
    using namespace skr;

    // map
    Map<uint32_t, uint32_t> test         = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    Map<uint32_t, uint32_t> test_empty   = {};
    Map<uint32_t, uint32_t> test_cleared = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_cleared.clear();
    Map<uint32_t, uint32_t> test_hole = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_hole.remove(1);

    // fixed map
    FixedMap<uint32_t, uint32_t, 6> test_fixed         = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    FixedMap<uint32_t, uint32_t, 6> test_fixed_empty   = {};
    FixedMap<uint32_t, uint32_t, 6> test_fixed_cleared = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_fixed_cleared.clear();
    FixedMap<uint32_t, uint32_t, 6> test_fixed_hole = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_fixed_hole.remove(1);

    // inline map
    InlineMap<uint32_t, uint32_t, 2> test_inline         = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    InlineMap<uint32_t, uint32_t, 2> test_inline_empty   = {};
    InlineMap<uint32_t, uint32_t, 2> test_inline_cleared = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_inline_cleared.clear();
    InlineMap<uint32_t, uint32_t, 2> test_inline_hole = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    test_inline_hole.remove(1);
}

inline void test_string()
{
    using namespace skr;

    String test       = u8"鸡你太美🐓🐓🐓🏀🏀🏀";
    String test_empty = {};
    String test_clear = u8"鸡你太美🐓🐓🐓🏀🏀🏀";
    test_clear.clear();
    String test_sso = u8"鸡你太美";
    test_sso.force_cancel_literal();
    String test_heap = u8"鸡你太美🐓🐓🐓🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🏀🐓🐓🐓";
    test_heap.force_cancel_literal();

    StringView test_view       = u8"鸡你太美🐓🐓🐓🏀🏀🏀";
    StringView test_view_empty = {};
    auto       start           = test_view.text_index_to_buffer(2);
    auto       end             = test_view.text_index_to_buffer(8);
    StringView test_view_sub   = test_view.subview(start, end - start);
}

inline void test_span()
{
    using namespace skr;

    Vector<uint32_t> src_test = { 1, 1, 4, 5, 1, 4 };

    Span<uint32_t> test       = src_test;
    Span<uint32_t> test_empty = {};
}

inline void test_optional()
{
    using namespace skr;

    Optional<GUID> test       = GUID::Create();
    Optional<GUID> test_empty = {};
}

inline void test_phmap()
{
    using namespace skr;

    FlatHashMap<uint32_t, uint32_t> test_flat_map = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    FlatHashMap<uint32_t, uint32_t> test_flat_map_empty;

    ParallelFlatHashMap<uint32_t, uint32_t> test_parallel_flat_map = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
    ParallelFlatHashMap<uint32_t, uint32_t> test_parallel_flat_map_empty;

    FlatHashSet<uint32_t> test_flat_set = { 1, 1, 4, 5, 1, 4 };
    FlatHashSet<uint32_t> test_flat_set_empty;

    ParallelFlatHashSet<uint32_t> test_parallel_flat_set = { 1, 1, 4, 5, 1, 4 };
    ParallelFlatHashSet<uint32_t> test_parallel_flat_set_empty;
}

TEST_CASE("test natvis")
{
    test_basic_types();
    test_vector();
    test_sparse_vector();
    test_set();
    test_multi_set();
    test_map();
    test_string();
    test_span();
    test_optional();
    test_phmap();
}