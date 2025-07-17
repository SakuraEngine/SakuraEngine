#include <SkrTestFramework/framework.hpp>
#include <SkrCore/log.hpp>
#include <SkrRTTR/export/export_builder.hpp>
#include <SkrCore/exec_static.hpp>

#include <SkrContainers/optional.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrContainers/sparse_vector.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/map.hpp>

#include <SkrRTTR/generic/generic_optional.hpp>
#include <SkrRTTR/generic/generic_vector.hpp>
#include <SkrRTTR/generic/generic_sparse_vector.hpp>
#include <SkrRTTR/generic/generic_sparse_hash_set.hpp>
#include <SkrRTTR/generic/generic_sparse_hash_map.hpp>

struct OpTester {
    static int  ctor_count;
    static int  dtor_count;
    static int  copy_count;
    static int  move_count;
    static int  assign_count;
    static int  move_assign_count;
    static void reset_all_count()
    {
        ctor_count        = 0;
        dtor_count        = 0;
        copy_count        = 0;
        move_count        = 0;
        assign_count      = 0;
        move_assign_count = 0;
    }

    inline OpTester()
    {
        ++ctor_count;
    }
    inline ~OpTester()
    {
        ++dtor_count;
    }
    inline OpTester(const OpTester&)
    {
        ++copy_count;
    }
    inline OpTester(OpTester&&)
    {
        ++move_count;
    }
    inline OpTester& operator=(const OpTester&)
    {
        ++assign_count;
        return *this;
    }
    inline OpTester& operator=(OpTester&&)
    {
        ++move_assign_count;
        return *this;
    }
};

int OpTester::ctor_count        = 0;
int OpTester::dtor_count        = 0;
int OpTester::copy_count        = 0;
int OpTester::move_count        = 0;
int OpTester::assign_count      = 0;
int OpTester::move_assign_count = 0;

SKR_RTTR_TYPE(OpTester, "7b3e54f1-1d3a-49f6-b15b-bfe439e14076")
SKR_EXEC_STATIC_CTOR
{
    skr::register_type_loader(
        skr::type_id_of<OpTester>(),
        +[](skr::RTTRType* type) {
            type->build_record([](skr::RTTRRecordData* data) {
                skr::RTTRRecordBuilder<OpTester> builder(data);
                builder.basic_info();
            });
        }
    );
};

TEST_CASE("test generic optional")
{
    using namespace skr;

    Optional<bool>    opt_bool;
    Optional<int16_t> opt_int16;
    Optional<int32_t> opt_int32;
    Optional<int64_t> opt_int64;
    Optional<float>   opt_float;
    Optional<double>  opt_double;
    Optional<GUID>    opt_guid;

    RC<GenericOptional> generic_opt_bool   = build_generic(type_signature_of<Optional<bool>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_int16  = build_generic(type_signature_of<Optional<int16_t>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_int32  = build_generic(type_signature_of<Optional<int32_t>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_int64  = build_generic(type_signature_of<Optional<int64_t>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_float  = build_generic(type_signature_of<Optional<float>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_double = build_generic(type_signature_of<Optional<double>>()).cast_static<GenericOptional>();
    RC<GenericOptional> generic_opt_guid   = build_generic(type_signature_of<Optional<GUID>>()).cast_static<GenericOptional>();

    // check address
    REQUIRE_EQ(&opt_bool.value(), generic_opt_bool->value_ptr(&opt_bool));
    REQUIRE_EQ(&opt_int16.value(), generic_opt_int16->value_ptr(&opt_int16));
    REQUIRE_EQ(&opt_int32.value(), generic_opt_int32->value_ptr(&opt_int32));
    REQUIRE_EQ(&opt_int64.value(), generic_opt_int64->value_ptr(&opt_int64));
    REQUIRE_EQ(&opt_float.value(), generic_opt_float->value_ptr(&opt_float));
    REQUIRE_EQ(&opt_double.value(), generic_opt_double->value_ptr(&opt_double));
    REQUIRE_EQ(&opt_guid.value(), generic_opt_guid->value_ptr(&opt_guid));

    // check has_value
    REQUIRE_FALSE(generic_opt_bool->has_value(&opt_bool));
    REQUIRE_FALSE(generic_opt_int16->has_value(&opt_int16));
    REQUIRE_FALSE(generic_opt_int32->has_value(&opt_int32));
    REQUIRE_FALSE(generic_opt_int64->has_value(&opt_int64));
    REQUIRE_FALSE(generic_opt_float->has_value(&opt_float));
    REQUIRE_FALSE(generic_opt_double->has_value(&opt_double));
    REQUIRE_FALSE(generic_opt_guid->has_value(&opt_guid));

    // do assign
    opt_bool   = true;
    opt_int16  = 1;
    opt_int32  = 2;
    opt_int64  = 3;
    opt_float  = 4.0f;
    opt_double = 5.0;
    opt_guid   = u8"051b41eb-2f26-4509-9032-0ca30d9b1990"_guid;

    // check assigned has value
    REQUIRE(generic_opt_bool->has_value(&opt_bool));
    REQUIRE(generic_opt_int16->has_value(&opt_int16));
    REQUIRE(generic_opt_int32->has_value(&opt_int32));
    REQUIRE(generic_opt_int64->has_value(&opt_int64));
    REQUIRE(generic_opt_float->has_value(&opt_float));
    REQUIRE(generic_opt_double->has_value(&opt_double));
    REQUIRE(generic_opt_guid->has_value(&opt_guid));

    // generic reset
    generic_opt_bool->reset(&opt_bool);
    generic_opt_int16->reset(&opt_int16);
    generic_opt_int32->reset(&opt_int32);
    generic_opt_int64->reset(&opt_int64);
    generic_opt_float->reset(&opt_float);
    generic_opt_double->reset(&opt_double);
    generic_opt_guid->reset(&opt_guid);

    // check reset has value
    REQUIRE_FALSE(opt_bool.has_value());
    REQUIRE_FALSE(opt_int16.has_value());
    REQUIRE_FALSE(opt_int32.has_value());
    REQUIRE_FALSE(opt_int64.has_value());
    REQUIRE_FALSE(opt_float.has_value());
    REQUIRE_FALSE(opt_double.has_value());
    REQUIRE_FALSE(opt_guid.has_value());

    // assign value
    bool    bool_val   = false;
    int16_t int16_val  = 10;
    int32_t int32_val  = 20;
    int64_t int64_val  = 30;
    float   float_val  = 40.5f;
    double  double_val = 50.5;
    auto    guid_val   = u8"a1b2c3d4-e5f6-7890-abcd-123456789abc"_guid;

    // generic assign
    generic_opt_bool->assign_value(&opt_bool, &bool_val);
    generic_opt_int16->assign_value(&opt_int16, &int16_val);
    generic_opt_int32->assign_value(&opt_int32, &int32_val);
    generic_opt_int64->assign_value(&opt_int64, &int64_val);
    generic_opt_float->assign_value(&opt_float, &float_val);
    generic_opt_double->assign_value(&opt_double, &double_val);
    generic_opt_guid->assign_value(&opt_guid, &guid_val);

    // check generic assigned has value
    REQUIRE(generic_opt_bool->has_value(&opt_bool));
    REQUIRE(generic_opt_int16->has_value(&opt_int16));
    REQUIRE(generic_opt_int32->has_value(&opt_int32));
    REQUIRE(generic_opt_int64->has_value(&opt_int64));
    REQUIRE(generic_opt_float->has_value(&opt_float));
    REQUIRE(generic_opt_double->has_value(&opt_double));
    REQUIRE(generic_opt_guid->has_value(&opt_guid));

    // check generic assigned values are correct
    REQUIRE_EQ(opt_bool.value(), bool_val);
    REQUIRE_EQ(opt_int16.value(), int16_val);
    REQUIRE_EQ(opt_int32.value(), int32_val);
    REQUIRE_EQ(opt_int64.value(), int64_val);
    REQUIRE_EQ(opt_float.value(), float_val);
    REQUIRE_EQ(opt_double.value(), double_val);
    REQUIRE_EQ(opt_guid.value(), guid_val);

    // reset all
    opt_bool.reset();
    opt_int16.reset();
    opt_int32.reset();
    opt_int64.reset();
    opt_float.reset();
    opt_double.reset();
    opt_guid.reset();

    // cache values
    bool    cached_bool_val   = bool_val;
    int16_t cached_int16_val  = int16_val;
    int32_t cached_int32_val  = int32_val;
    int64_t cached_int64_val  = int64_val;
    float   cached_float_val  = float_val;
    double  cached_double_val = double_val;
    auto    cached_guid_val   = guid_val;

    // generic assign move
    generic_opt_bool->assign_value_move(&opt_bool, &bool_val);
    generic_opt_int16->assign_value_move(&opt_int16, &int16_val);
    generic_opt_int32->assign_value_move(&opt_int32, &int32_val);
    generic_opt_int64->assign_value_move(&opt_int64, &int64_val);
    generic_opt_float->assign_value_move(&opt_float, &float_val);
    generic_opt_double->assign_value_move(&opt_double, &double_val);
    generic_opt_guid->assign_value_move(&opt_guid, &guid_val);

    // check generic assigned move values are correct
    REQUIRE(generic_opt_bool->has_value(&opt_bool));
    REQUIRE(generic_opt_int16->has_value(&opt_int16));
    REQUIRE(generic_opt_int32->has_value(&opt_int32));
    REQUIRE(generic_opt_int64->has_value(&opt_int64));
    REQUIRE(generic_opt_float->has_value(&opt_float));
    REQUIRE(generic_opt_double->has_value(&opt_double));
    REQUIRE(generic_opt_guid->has_value(&opt_guid));

    // check generic assigned move values are correct
    REQUIRE_EQ(opt_bool.value(), cached_bool_val);
    REQUIRE_EQ(opt_int16.value(), cached_int16_val);
    REQUIRE_EQ(opt_int32.value(), cached_int32_val);
    REQUIRE_EQ(opt_int64.value(), cached_int64_val);
    REQUIRE_EQ(opt_float.value(), cached_float_val);
    REQUIRE_EQ(opt_double.value(), cached_double_val);
    REQUIRE_EQ(opt_guid.value(), cached_guid_val);

    SUBCASE("nested")
    {
        Optional<Optional<int32_t>> nested_opt_int32;
        RC<GenericOptional>         generic_nested_opt_int32 = build_generic(type_signature_of<Optional<Optional<int32_t>>>()).cast_static<GenericOptional>();

        Optional<int32_t> inner_opt_int32;
        inner_opt_int32 = 114514;

        generic_nested_opt_int32->assign_value_move(&nested_opt_int32, &inner_opt_int32);

        REQUIRE(!inner_opt_int32.has_value());
        REQUIRE(nested_opt_int32.has_value());
        REQUIRE(nested_opt_int32.value().has_value());
        REQUIRE_EQ(nested_opt_int32.value().value(), 114514);
    }
}

TEST_CASE("test generic vector")
{
    using namespace skr;

    RC<GenericVector> generic_vec = build_generic(type_signature_of<Vector<int32_t>>()).cast_static<GenericVector>();
    using VecType                 = Vector<int32_t>;

    SUBCASE("data get")
    {
        VecType vec;

        // check data empty
        REQUIRE_EQ(generic_vec->capacity(&vec), vec.capacity());
        REQUIRE_EQ(generic_vec->slack(&vec), vec.slack());
        REQUIRE_EQ(generic_vec->size(&vec), vec.size());
        REQUIRE_EQ(generic_vec->is_empty(&vec), vec.is_empty());
        REQUIRE_EQ(generic_vec->data(&vec), vec.data());

        // check data with content
        vec = { 1, 1, 4, 5, 1, 4 };
        REQUIRE_EQ(generic_vec->capacity(&vec), vec.capacity());
        REQUIRE_EQ(generic_vec->slack(&vec), vec.slack());
        REQUIRE_EQ(generic_vec->size(&vec), vec.size());
        REQUIRE_EQ(generic_vec->is_empty(&vec), vec.is_empty());
        REQUIRE_EQ(generic_vec->data(&vec), vec.data());
    }

    SUBCASE("memory op")
    {
        VecType vec{ 1, 1, 4, 5, 1, 4 };

        // clear
        auto old_capacity = vec.capacity();
        generic_vec->clear(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.capacity(), old_capacity);
        REQUIRE_NE(vec.data(), nullptr);

        // release
        generic_vec->release(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.capacity(), 0);
        REQUIRE_EQ(vec.data(), nullptr);

        // release with reserve
        vec = { 1, 1, 4, 5, 1, 4 };
        generic_vec->release(&vec, 10);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.capacity(), 10);
        REQUIRE_NE(vec.data(), nullptr);

        // reserve
        generic_vec->reserve(&vec, 114514);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.capacity(), 114514);
        REQUIRE_NE(vec.data(), nullptr);

        // shrink
        generic_vec->shrink(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.capacity(), 0);
        REQUIRE_EQ(vec.data(), nullptr);

        // resize
        int32_t new_value = 114514;
        generic_vec->resize(&vec, 10, &new_value);
        REQUIRE_EQ(vec.size(), 10);
        REQUIRE_EQ(vec.capacity(), 10);
        for (size_t i = 0; i < vec.size(); ++i)
        {
            REQUIRE_EQ(vec[i], new_value);
        }

        // resize default
        generic_vec->resize_default(&vec, 5);
        REQUIRE_EQ(vec.size(), 5);
        REQUIRE_GE(vec.capacity(), 5);
        for (size_t i = 0; i < vec.size(); ++i)
        {
            REQUIRE_EQ(vec[i], 114514);
        }

        // resize zeroed
        generic_vec->resize_zeroed(&vec, 114514);
        REQUIRE_EQ(vec.size(), 114514);
        REQUIRE_GE(vec.capacity(), 114514);
        for (size_t i = 0; i < 5; ++i)
        {
            REQUIRE_EQ(vec[i], 114514);
        }
        for (size_t i = 5; i < vec.size(); ++i)
        {
            REQUIRE_EQ(vec[i], 0);
        }
    }

    SUBCASE("add")
    {
        VecType vec = {};

        // add
        {
            int32_t value = 114514;
            generic_vec->add(&vec, &value);
            REQUIRE_EQ(vec.size(), 1);
            REQUIRE_GE(vec.capacity(), 1);
            REQUIRE_EQ(vec[0], value);
        }

        // add n
        {
            int32_t value = 1919810;
            generic_vec->add(&vec, &value, 10);
            REQUIRE_EQ(vec.size(), 11);
            REQUIRE_GE(vec.capacity(), 11);
            for (size_t i = 1; i < vec.size(); ++i)
            {
                REQUIRE_EQ(vec[i], value);
            }
        }

        // add move
        {
            int32_t value = 123456;
            generic_vec->add_move(&vec, &value);
            REQUIRE_EQ(vec.size(), 12);
            REQUIRE_GE(vec.capacity(), 12);
            REQUIRE_EQ(vec[11], 123456);
        }

        vec.clear();

        // add zeroed
        {
            generic_vec->add_zeroed(&vec, 10);
            REQUIRE_EQ(vec.size(), 10);
            REQUIRE_GE(vec.capacity(), 10);
            for (size_t i = 0; i < vec.size(); ++i)
            {
                REQUIRE_EQ(vec[i], 0);
            }
        }

        // add unique
        vec.clear();
        {
            for (int32_t i = 0; i < 10; ++i)
            {
                generic_vec->add_unique(&vec, &i);
            }
            for (int32_t i = 0; i < 10; ++i)
            {
                generic_vec->add_unique(&vec, &i);
            }

            REQUIRE_EQ(vec.size(), 10);
            REQUIRE_GE(vec.capacity(), 10);
            for (int32_t i = 0; i < 10; ++i)
            {
                REQUIRE_EQ(vec[i], i);
            }
        }
    }

    SUBCASE("add at")
    {
        VecType vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

        // add at
        {
            int32_t value = 0;
            generic_vec->add_at(&vec, 0, &value);
            REQUIRE_EQ(vec.size(), 11);
            REQUIRE_GE(vec.capacity(), 11);
            REQUIRE_EQ(vec[0], value);
            for (size_t i = 1; i < vec.size(); ++i)
            {
                REQUIRE_EQ(vec[i], i);
            }
        }

        // add at n
        {
            int32_t value = 114514;
            generic_vec->add_at(&vec, 5, &value, 10);
            REQUIRE_EQ(vec.size(), 21);
            REQUIRE_GE(vec.capacity(), 21);
            for (size_t i = 0; i < 5; ++i)
            {
                REQUIRE_EQ(vec[i], i);
            }
            for (size_t i = 5; i < 15; ++i)
            {
                REQUIRE_EQ(vec[i], value);
            }
            for (size_t i = 15; i < vec.size(); ++i)
            {
                REQUIRE_EQ(vec[i], i - 10);
            }
        }
    }

    SUBCASE("append")
    {
        VecType vec1 = { 1, 2, 3, 4, 5 };
        VecType vec2 = { 6, 7, 8, 9, 10 };

        // append
        generic_vec->append(&vec1, &vec2);
        REQUIRE_EQ(vec1.size(), 10);
        REQUIRE_GE(vec1.capacity(), 10);
        for (size_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(vec1[i], i + 1);
        }
    }

    SUBCASE("append at")
    {
        VecType vec1 = { 1, 2, 3, 4, 5 };
        VecType vec2 = { 6, 7, 8, 9, 10 };

        // append at
        generic_vec->append_at(&vec1, 2, &vec2);
        REQUIRE_EQ(vec1.size(), 10);
        REQUIRE_GE(vec1.capacity(), 10);
        for (size_t i = 0; i < 2; ++i)
        {
            REQUIRE_EQ(vec1[i], i + 1);
        }
        for (size_t i = 2; i < 7; ++i)
        {
            REQUIRE_EQ(vec1[i], i + 4);
        }
        for (size_t i = 7; i < 10; ++i)
        {
            REQUIRE_EQ(vec1[i], i - 4);
        }
    }

    SUBCASE("remove at")
    {
        VecType vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

        // remove at
        generic_vec->remove_at(&vec, 0);
        REQUIRE_EQ(vec.size(), 9);
        REQUIRE_GE(vec.capacity(), 9);
        for (size_t i = 0; i < vec.size(); ++i)
        {
            REQUIRE_EQ(vec[i], i + 2);
        }

        // remove at n
        generic_vec->remove_at(&vec, 5, 3);
        REQUIRE_EQ(vec.size(), 6);
        REQUIRE_GE(vec.capacity(), 6);
        for (size_t i = 0; i < vec.size(); ++i)
        {
            if (i < 5)
            {
                REQUIRE_EQ(vec[i], i + 2);
            }
            else
            {
                REQUIRE_EQ(vec[i], i + 5);
            }
        }
    }

    SUBCASE("remove")
    {
        VecType vec = { 1, 1, 4, 5, 1, 4 };

        // remove
        int32_t value_to_remove = 1;
        bool    removed         = generic_vec->remove(&vec, &value_to_remove);
        REQUIRE(removed);
        REQUIRE_EQ(vec.size(), 5);
        REQUIRE_GE(vec.capacity(), 5);
        REQUIRE_EQ(vec[0], 1);
        REQUIRE_EQ(vec[1], 4);
        REQUIRE_EQ(vec[2], 5);
        REQUIRE_EQ(vec[3], 1);
        REQUIRE_EQ(vec[4], 4);

        // remove last
        value_to_remove = 1;
        removed         = generic_vec->remove_last(&vec, &value_to_remove);
        REQUIRE(removed);
        REQUIRE_EQ(vec.size(), 4);
        REQUIRE_GE(vec.capacity(), 4);
        REQUIRE_EQ(vec[0], 1);
        REQUIRE_EQ(vec[1], 4);
        REQUIRE_EQ(vec[2], 5);
        REQUIRE_EQ(vec[3], 4);

        // remove all
        value_to_remove        = 4;
        uint64_t removed_count = generic_vec->remove_all(&vec, &value_to_remove);
        REQUIRE_EQ(removed_count, 2);
        REQUIRE_EQ(vec.size(), 2);
        REQUIRE_GE(vec.capacity(), 2);
        REQUIRE_EQ(vec[0], 1);
        REQUIRE_EQ(vec[1], 5);

        vec = { 1, 1, 4, 5, 1, 4 };

        // remove swap
        value_to_remove = 1;
        removed         = generic_vec->remove_swap(&vec, &value_to_remove);
        REQUIRE(removed);
        REQUIRE_EQ(vec.size(), 5);
        REQUIRE_GE(vec.capacity(), 5);
        REQUIRE_EQ(vec.count(1), 2);

        // remove last swap
        value_to_remove = 1;
        removed         = generic_vec->remove_last_swap(&vec, &value_to_remove);
        REQUIRE(removed);
        REQUIRE_EQ(vec.size(), 4);
        REQUIRE_GE(vec.capacity(), 4);
        REQUIRE_EQ(vec.count(1), 1);

        // remove all swap
        value_to_remove = 4;
        removed_count   = generic_vec->remove_all_swap(&vec, &value_to_remove);
        REQUIRE_EQ(removed_count, 2);
        REQUIRE_EQ(vec.size(), 2);
        REQUIRE_GE(vec.capacity(), 2);
        REQUIRE_FALSE(vec.contains(value_to_remove));
    }

    SUBCASE("access")
    {
        VecType vec;
        vec.resize_unsafe(100);
        for (size_t i = 0; i < vec.size(); ++i)
        {
            vec[i] = static_cast<int32_t>(i + 1);
        }

        for (size_t i = 0; i < vec.size(); ++i)
        {
            REQUIRE_EQ(generic_vec->at(&vec, i), &vec.at(i));
            REQUIRE_EQ(generic_vec->at_last(&vec, i), &vec.at_last(i));
        }
    }

    SUBCASE("find")
    {
        VecType vec = { 1, 1, 4, 5, 1, 4 };

        // find
        int32_t value_to_find = 1;
        auto    found_ref     = generic_vec->find(&vec, &value_to_find);
        REQUIRE(found_ref.is_valid());
        REQUIRE_EQ(found_ref.index, 0);
        REQUIRE_EQ(found_ref.ptr, &vec.at(0));

        // find last
        value_to_find = 1;
        found_ref     = generic_vec->find_last(&vec, &value_to_find);
        REQUIRE(found_ref.is_valid());
        REQUIRE_EQ(found_ref.index, 4);
        REQUIRE_EQ(found_ref.ptr, &vec.at(4));

        // find not found
        value_to_find = 114514;
        found_ref     = generic_vec->find(&vec, &value_to_find);
        REQUIRE_FALSE(found_ref.is_valid());

        // find last not found
        value_to_find = 1919810;
        found_ref     = generic_vec->find_last(&vec, &value_to_find);
        REQUIRE_FALSE(found_ref.is_valid());
    }

    SUBCASE("contains & count")
    {
        VecType vec = { 1, 1, 4, 5, 1, 4 };

        // contains
        int32_t value_to_check = 1;
        REQUIRE(generic_vec->contains(&vec, &value_to_check));
        value_to_check = 114514;
        REQUIRE_FALSE(generic_vec->contains(&vec, &value_to_check));

        // count
        value_to_check = 1;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 3);
        value_to_check = 4;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 2);
        value_to_check = 1919810;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 0);
    }
}

TEST_CASE("test generic sparse vector")
{
    using namespace skr;

    RC<GenericSparseVector> generic_vec = build_generic(type_signature_of<SparseVector<int32_t>>()).cast_static<GenericSparseVector>();
    using VecType                       = SparseVector<int32_t>;

    auto _make_vec_with_hole = +[]() {
        VecType vec{
            1, 1,
            1, 1,
            4, 4,
            5, 5,
            1, 1,
            4, 4
        };
        for (size_t i = 0; i < vec.sparse_size(); ++i)
        {
            if (i % 2 == 0)
            {
                vec.remove_at(i);
            }
        }
        return vec;
    };

    SUBCASE("data get")
    {
        VecType vec;

        // check data empty
        REQUIRE_EQ(generic_vec->size(&vec), vec.size());
        REQUIRE_EQ(generic_vec->capacity(&vec), vec.capacity());
        REQUIRE_EQ(generic_vec->slack(&vec), vec.slack());
        REQUIRE_EQ(generic_vec->sparse_size(&vec), vec.sparse_size());
        REQUIRE_EQ(generic_vec->hole_size(&vec), vec.hole_size());
        REQUIRE_EQ(generic_vec->bit_size(&vec), vec.bit_size());
        REQUIRE_EQ(generic_vec->freelist_head(&vec), vec.freelist_head());
        REQUIRE_EQ(generic_vec->is_compact(&vec), vec.is_compact());
        REQUIRE_EQ(generic_vec->is_empty(&vec), vec.is_empty());
        REQUIRE_EQ(generic_vec->storage(&vec), vec.storage());
        REQUIRE_EQ(generic_vec->bit_data(&vec), vec.bit_data());

        // check data with content
        vec = { 1, 1, 4, 5, 1, 4 };
        REQUIRE_EQ(generic_vec->size(&vec), vec.size());
        REQUIRE_EQ(generic_vec->capacity(&vec), vec.capacity());
        REQUIRE_EQ(generic_vec->slack(&vec), vec.slack());
        REQUIRE_EQ(generic_vec->sparse_size(&vec), vec.sparse_size());
        REQUIRE_EQ(generic_vec->hole_size(&vec), vec.hole_size());
        REQUIRE_EQ(generic_vec->bit_size(&vec), vec.bit_size());
        REQUIRE_EQ(generic_vec->freelist_head(&vec), vec.freelist_head());
        REQUIRE_EQ(generic_vec->is_compact(&vec), vec.is_compact());
        REQUIRE_EQ(generic_vec->is_empty(&vec), vec.is_empty());
        REQUIRE_EQ(generic_vec->storage(&vec), vec.storage());
        REQUIRE_EQ(generic_vec->bit_data(&vec), vec.bit_data());
    }

    SUBCASE("validate")
    {
        VecType vec = _make_vec_with_hole();

        // check has data
        for (size_t i = 0; i < vec.sparse_size(); ++i)
        {
            REQUIRE_EQ(generic_vec->has_data(&vec, i), i % 2 != 0);
        }

        // check is hole
        for (size_t i = 0; i < vec.sparse_size(); ++i)
        {
            REQUIRE_EQ(generic_vec->is_hole(&vec, i), i % 2 == 0);
        }

        // check valid index
        for (size_t i = 0; i < vec.sparse_size(); ++i)
        {
            REQUIRE(generic_vec->is_valid_index(&vec, i));
        }
    }

    SUBCASE("memory op")
    {
        VecType vec = _make_vec_with_hole();

        // clear
        auto old_capacity = vec.capacity();
        generic_vec->clear(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.sparse_size(), 0);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_EQ(vec.capacity(), old_capacity);
        REQUIRE_NE(vec.storage(), nullptr);
        REQUIRE_NE(vec.bit_data(), nullptr);

        // release
        generic_vec->release(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.sparse_size(), 0);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_EQ(vec.capacity(), 0);
        REQUIRE_EQ(vec.storage(), nullptr);
        REQUIRE_EQ(vec.bit_data(), nullptr);

        // release with reserve
        vec = _make_vec_with_hole();
        generic_vec->release(&vec, 10);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.sparse_size(), 0);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_EQ(vec.capacity(), 10);
        REQUIRE_NE(vec.storage(), nullptr);
        REQUIRE_NE(vec.bit_data(), nullptr);

        // reserve
        generic_vec->reserve(&vec, 114514);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.sparse_size(), 0);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_EQ(vec.capacity(), 114514);

        // shrink
        generic_vec->shrink(&vec);
        REQUIRE_EQ(vec.size(), 0);
        REQUIRE_EQ(vec.sparse_size(), 0);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_EQ(vec.capacity(), 0);

        // compact
        vec = _make_vec_with_hole();
        REQUIRE_FALSE(generic_vec->is_compact(&vec));
        REQUIRE(generic_vec->compact(&vec));
        REQUIRE_EQ(vec.size(), 6);
        REQUIRE_EQ(vec.sparse_size(), 6);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_GE(vec.capacity(), 12);
        REQUIRE_EQ(vec.count(1), 3);
        REQUIRE_EQ(vec.count(4), 2);
        REQUIRE_EQ(vec.count(5), 1);

        // compact stable
        vec = _make_vec_with_hole();
        REQUIRE_FALSE(generic_vec->is_compact(&vec));
        REQUIRE(generic_vec->compact_stable(&vec));
        REQUIRE_EQ(vec.size(), 6);
        REQUIRE_EQ(vec.sparse_size(), 6);
        REQUIRE_EQ(vec.hole_size(), 0);
        REQUIRE_GE(vec.capacity(), 12);
        REQUIRE_EQ(vec[0], 1);
        REQUIRE_EQ(vec[1], 1);
        REQUIRE_EQ(vec[2], 4);
        REQUIRE_EQ(vec[3], 5);
        REQUIRE_EQ(vec[4], 1);
        REQUIRE_EQ(vec[5], 4);

        // compact top
        vec.clear();
        for (size_t i = 0; i < 100; ++i)
        {
            vec.add(i);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            if (i % 2 == 0)
            {
                vec.remove_at(i);
            }
        }
        for (size_t i = 50; i < 100; ++i)
        {
            vec.remove_at(i);
        }
        REQUIRE_FALSE(generic_vec->is_compact(&vec));
        REQUIRE(generic_vec->compact_top(&vec));
        REQUIRE_EQ(vec.size(), 25);
        REQUIRE_EQ(vec.sparse_size(), 50);
        REQUIRE_EQ(vec.hole_size(), 25);
        REQUIRE_GE(vec.capacity(), 50);
        for (size_t i = 0; i < vec.sparse_size(); ++i)
        {
            if (i % 2 == 0)
            {
                REQUIRE_FALSE(vec.has_data(i));
            }
            else
            {
                REQUIRE_EQ(vec[i], i);
            }
        }
    }

    SUBCASE("add")
    {
        VecType vec = {};

        // add
        {
            vec.clear();
            int32_t value = 114514;
            generic_vec->add(&vec, &value);
            REQUIRE_EQ(vec.size(), 1);
            REQUIRE_GE(vec.capacity(), 1);
            REQUIRE_EQ(vec[0], value);
        }

        // add with hole
        {
            vec           = _make_vec_with_hole();
            int32_t value = 1919810;
            generic_vec->add(&vec, &value);
            REQUIRE_EQ(vec.size(), 7);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 5);
            REQUIRE_GE(vec.capacity(), 7);
        }

        // add move
        {
            vec.clear();
            int32_t value = 123456;
            generic_vec->add_move(&vec, &value);
            REQUIRE_EQ(vec.size(), 1);
            REQUIRE_GE(vec.capacity(), 1);
            REQUIRE_EQ(vec[0], 123456);
        }

        // add move with hole
        {
            vec           = _make_vec_with_hole();
            int32_t value = 654321;
            generic_vec->add_move(&vec, &value);
            REQUIRE_EQ(vec.size(), 7);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 5);
            REQUIRE_GE(vec.capacity(), 7);
        }

        // add zeroed
        {
            vec.clear();
            generic_vec->add_zeroed(&vec);
            REQUIRE_EQ(vec.size(), 1);
            REQUIRE_GE(vec.capacity(), 1);
            REQUIRE_EQ(vec[0], 0);
        }

        // add zeroed with hole
        {
            vec = _make_vec_with_hole();
            generic_vec->add_zeroed(&vec);
            REQUIRE_EQ(vec.size(), 7);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 5);
            REQUIRE_GE(vec.capacity(), 7);
        }
    }

    SUBCASE("add at")
    {
        VecType vec      = _make_vec_with_hole();
        int32_t values[] = { 1, 1, 4, 5, 1, 4 };

        // add at
        {
            for (int32_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (!vec.has_data(i))
                {
                    generic_vec->add_at(&vec, i, &values[i / 2]);
                }
            }
            REQUIRE_EQ(vec.size(), 12);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 0);
            REQUIRE_GE(vec.capacity(), 12);
            for (size_t i = 0; i < vec.sparse_size(); ++i)
            {
                REQUIRE_EQ(vec[i], values[i / 2]);
            }
        }

        // add at move
        {
            vec = _make_vec_with_hole();
            for (int32_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (!vec.has_data(i))
                {
                    generic_vec->add_at_move(&vec, i, &values[i / 2]);
                }
            }
            REQUIRE_EQ(vec.size(), 12);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 0);
            REQUIRE_GE(vec.capacity(), 12);
            for (size_t i = 0; i < vec.sparse_size(); ++i)
            {
                REQUIRE_EQ(vec[i], values[i / 2]);
            }
        }

        // add at zeroed
        {
            vec = _make_vec_with_hole();
            for (int32_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (!vec.has_data(i))
                {
                    generic_vec->add_at_zeroed(&vec, i);
                }
            }
            REQUIRE_EQ(vec.size(), 12);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 0);
            REQUIRE_EQ(vec.capacity(), 12);
            for (size_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (i % 2 == 0)
                {
                    REQUIRE_EQ(vec[i], 0);
                }
                else
                {
                    REQUIRE_EQ(vec[i], values[i / 2]);
                }
            }
        }
    }

    SUBCASE("append")
    {
        VecType vec1     = { 1, 1, 4, 5, 1, 4 };
        VecType vec2     = { 6, 7, 8, 9, 10, 11 };
        int32_t values[] = { 1, 1, 4, 5, 1, 4 };

        // append
        generic_vec->append(&vec1, &vec2);
        REQUIRE_EQ(vec1.size(), 12);
        REQUIRE_EQ(vec1.sparse_size(), 12);
        REQUIRE_EQ(vec1.hole_size(), 0);
        REQUIRE_GE(vec1.capacity(), 12);
        for (size_t i = 0; i < vec1.sparse_size(); ++i)
        {
            if (i < 6)
            {
                REQUIRE_EQ(vec1[i], values[i]);
            }
            else
            {
                REQUIRE_EQ(vec1[i], i);
            }
        }
    }

    SUBCASE("remove")
    {
        VecType vec;

        // remove at
        {
            int32_t values[] = { 1, 1, 4, 5, 1, 4 };
            vec              = {
                1, 1,
                1, 1,
                4, 4,
                5, 5,
                1, 1,
                4, 4
            };

            for (size_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (i % 2 == 0)
                {
                    generic_vec->remove_at(&vec, i);
                }
            }

            REQUIRE_EQ(vec.size(), 6);
            REQUIRE_EQ(vec.sparse_size(), 12);
            REQUIRE_EQ(vec.hole_size(), 6);
            REQUIRE_GE(vec.capacity(), 12);

            for (size_t i = 0; i < vec.sparse_size(); ++i)
            {
                if (i % 2 == 0)
                {
                    REQUIRE_FALSE(vec.has_data(i));
                }
                else
                {
                    REQUIRE_EQ(vec[i], values[i / 2]);
                }
            }
        }

        // remove
        {
            vec                      = { 1, 1, 4, 5, 1, 4 };
            uint32_t value_to_remove = 1;
            generic_vec->remove(&vec, &value_to_remove);
            REQUIRE_EQ(vec.size(), 5);
            REQUIRE_EQ(vec.sparse_size(), 6);
            REQUIRE_EQ(vec.hole_size(), 1);
            REQUIRE_GE(vec.capacity(), 6);
            REQUIRE_FALSE(vec.has_data(0));
            REQUIRE_EQ(vec.count(1), 2);
            REQUIRE_EQ(vec.count(4), 2);
            REQUIRE_EQ(vec.count(5), 1);
        }

        // remove last
        {
            vec                      = { 1, 1, 4, 5, 1, 4 };
            uint32_t value_to_remove = 1;
            generic_vec->remove_last(&vec, &value_to_remove);
            REQUIRE_EQ(vec.size(), 5);
            REQUIRE_EQ(vec.sparse_size(), 6);
            REQUIRE_EQ(vec.hole_size(), 1);
            REQUIRE_GE(vec.capacity(), 6);
            REQUIRE_FALSE(vec.has_data(4));
            REQUIRE_EQ(vec.count(1), 2);
            REQUIRE_EQ(vec.count(4), 2);
            REQUIRE_EQ(vec.count(5), 1);
        }

        // remove all
        {
            vec                      = { 1, 1, 4, 5, 1, 4 };
            uint32_t value_to_remove = 4;
            uint64_t removed_count   = generic_vec->remove_all(&vec, &value_to_remove);
            REQUIRE_EQ(removed_count, 2);
            REQUIRE_EQ(vec.size(), 4);
            REQUIRE_EQ(vec.sparse_size(), 6);
            REQUIRE_EQ(vec.hole_size(), 2);
            REQUIRE_GE(vec.capacity(), 6);
            REQUIRE_FALSE(vec.has_data(2));
            REQUIRE_FALSE(vec.has_data(5));
            REQUIRE_EQ(vec.count(1), 3);
            REQUIRE_EQ(vec.count(4), 0);
            REQUIRE_EQ(vec.count(5), 1);
        }
    }

    SUBCASE("access")
    {
        VecType vec;
        for (size_t i = 0; i < 100; ++i)
        {
            vec.add(i);
        }

        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_EQ(&vec.at(i), generic_vec->at(&vec, i));
            REQUIRE_EQ(&vec.at_last(i), generic_vec->at_last(&vec, i));
        }
    }

    SUBCASE("find")
    {
        VecType vec = { 1, 1, 4, 5, 1, 4 };

        // find
        uint32_t value_to_find = 1;
        auto     found_ref     = generic_vec->find(&vec, &value_to_find);
        REQUIRE(found_ref.is_valid());
        REQUIRE_EQ(found_ref.index, 0);
        REQUIRE_EQ(found_ref.ptr, &vec.at(0));

        // find last
        value_to_find = 1;
        found_ref     = generic_vec->find_last(&vec, &value_to_find);
        REQUIRE(found_ref.is_valid());
        REQUIRE_EQ(found_ref.index, 4);
        REQUIRE_EQ(found_ref.ptr, &vec.at(4));

        // find not found
        value_to_find = 114514;
        found_ref     = generic_vec->find(&vec, &value_to_find);
        REQUIRE_FALSE(found_ref.is_valid());

        // find last not found
        value_to_find = 1919810;
        found_ref     = generic_vec->find_last(&vec, &value_to_find);
        REQUIRE_FALSE(found_ref.is_valid());
    }

    SUBCASE("contains & count")
    {
        VecType vec = { 1, 1, 4, 5, 1, 4 };

        // contains
        uint32_t value_to_check = 1;
        REQUIRE(generic_vec->contains(&vec, &value_to_check));
        value_to_check = 114514;
        REQUIRE_FALSE(generic_vec->contains(&vec, &value_to_check));

        // count
        value_to_check = 1;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 3);
        value_to_check = 4;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 2);
        value_to_check = 1919810;
        REQUIRE_EQ(generic_vec->count(&vec, &value_to_check), 0);
    }
}

TEST_CASE("test generic sparse hash set")
{
    using namespace skr;

    RC<GenericSparseHashSet> generic_set = build_generic(type_signature_of<Set<int32_t>>()).cast_static<GenericSparseHashSet>();
    using SetType                        = Set<int32_t>;

    SUBCASE("data get")
    {
        SetType set;

        // check empty data
        REQUIRE_EQ(generic_set->size(&set), set.size());
        REQUIRE_EQ(generic_set->capacity(&set), set.capacity());
        REQUIRE_EQ(generic_set->slack(&set), set.slack());
        REQUIRE_EQ(generic_set->sparse_size(&set), set.sparse_size());
        REQUIRE_EQ(generic_set->hole_size(&set), set.hole_size());
        REQUIRE_EQ(generic_set->bit_size(&set), set.bit_size());
        REQUIRE_EQ(generic_set->freelist_head(&set), set.freelist_head());
        REQUIRE_EQ(generic_set->is_compact(&set), set.is_compact());
        REQUIRE_EQ(generic_set->is_empty(&set), set.is_empty());
        REQUIRE_EQ(generic_set->storage(&set), set.data_vector().storage());
        REQUIRE_EQ(generic_set->bit_data(&set), set.data_vector().bit_data());
        REQUIRE_EQ(generic_set->bucket(&set), set.bucket());

        // check data with content
        set = { 1, 1, 4, 5, 1, 4 };
        REQUIRE_EQ(generic_set->size(&set), set.size());
        REQUIRE_EQ(generic_set->capacity(&set), set.capacity());
        REQUIRE_EQ(generic_set->slack(&set), set.slack());
        REQUIRE_EQ(generic_set->sparse_size(&set), set.sparse_size());
        REQUIRE_EQ(generic_set->hole_size(&set), set.hole_size());
        REQUIRE_EQ(generic_set->bit_size(&set), set.bit_size());
        REQUIRE_EQ(generic_set->freelist_head(&set), set.freelist_head());
        REQUIRE_EQ(generic_set->is_compact(&set), set.is_compact());
        REQUIRE_EQ(generic_set->is_empty(&set), set.is_empty());
        REQUIRE_EQ(generic_set->storage(&set), set.data_vector().storage());
        REQUIRE_EQ(generic_set->bit_data(&set), set.data_vector().bit_data());
        REQUIRE_EQ(generic_set->bucket(&set), set.bucket());
    }

    SUBCASE("memory op")
    {
        SetType set = { 1, 1, 4, 5, 1, 4 };

        // clear
        auto old_capacity = set.capacity();
        generic_set->clear(&set);
        REQUIRE_EQ(set.size(), 0);
        REQUIRE_EQ(set.sparse_size(), 0);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_EQ(set.capacity(), old_capacity);
        REQUIRE_NE(set.data_vector().storage(), nullptr);
        REQUIRE_NE(set.data_vector().bit_data(), nullptr);
        REQUIRE_NE(set.bucket(), nullptr);

        // release
        generic_set->release(&set);
        REQUIRE_EQ(set.size(), 0);
        REQUIRE_EQ(set.sparse_size(), 0);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_EQ(set.capacity(), 0);
        REQUIRE_EQ(set.data_vector().storage(), nullptr);
        REQUIRE_EQ(set.data_vector().bit_data(), nullptr);
        REQUIRE_EQ(set.bucket(), nullptr);

        // release with reserve
        set = { 1, 1, 4, 5, 1, 4 };
        generic_set->release(&set, 10);
        REQUIRE_EQ(set.size(), 0);
        REQUIRE_EQ(set.sparse_size(), 0);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_GE(set.capacity(), 10);
        REQUIRE_NE(set.data_vector().storage(), nullptr);
        REQUIRE_NE(set.data_vector().bit_data(), nullptr);
        REQUIRE_NE(set.bucket(), nullptr);

        // reserve
        generic_set->reserve(&set, 114514);
        REQUIRE_EQ(set.size(), 0);
        REQUIRE_EQ(set.sparse_size(), 0);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_EQ(set.capacity(), 114514);
        REQUIRE_NE(set.data_vector().storage(), nullptr);
        REQUIRE_NE(set.data_vector().bit_data(), nullptr);
        REQUIRE_NE(set.bucket(), nullptr);

        // reserve with content
        set.release();
        set = { 1, 1, 4, 5, 1, 4 };
        generic_set->reserve(&set, 114);
        REQUIRE_EQ(set.size(), 3);
        REQUIRE_GE(set.capacity(), 114);
        REQUIRE(set.contains(1));
        REQUIRE(set.contains(4));
        REQUIRE(set.contains(5));

        // shrink
        set.clear();
        generic_set->shrink(&set);
        REQUIRE_EQ(set.size(), 0);
        REQUIRE_EQ(set.sparse_size(), 0);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_EQ(set.capacity(), 0);
        REQUIRE_EQ(set.bucket(), nullptr);

        // shrink with content
        set = { 1, 1, 4, 5, 1, 4 };
        set.reserve(10000);
        generic_set->shrink(&set);
        REQUIRE_EQ(set.size(), 3);
        REQUIRE_GE(set.capacity(), 3);
        REQUIRE(set.contains(1));
        REQUIRE(set.contains(4));
        REQUIRE(set.contains(5));

        // compact
        set = { 1, 2, 3, 4, 5, 6 };
        set.remove(2);
        set.remove(3);
        set.remove(6);
        REQUIRE_FALSE(generic_set->is_compact(&set));
        REQUIRE(generic_set->compact(&set));
        REQUIRE_EQ(set.size(), 3);
        REQUIRE_EQ(set.sparse_size(), 3);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_GE(set.capacity(), 6);
        REQUIRE(set.contains(1));
        REQUIRE(set.contains(4));
        REQUIRE(set.contains(5));

        // compact stable
        set = { 1, 2, 3, 4, 5, 6 };
        set.remove(2);
        set.remove(3);
        set.remove(6);
        REQUIRE_FALSE(generic_set->is_compact(&set));
        REQUIRE(generic_set->compact_stable(&set));
        REQUIRE_EQ(set.size(), 3);
        REQUIRE_EQ(set.sparse_size(), 3);
        REQUIRE_EQ(set.hole_size(), 0);
        REQUIRE_GE(set.capacity(), 6);
        REQUIRE(set.contains(1));
        REQUIRE(set.contains(4));
        REQUIRE(set.contains(5));

        // compact top
        set.clear();
        for (size_t i = 0; i < 100; ++i)
        {
            set.add(i);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            if (i % 2 == 0)
            {
                set.remove_at(i);
            }
        }
        for (size_t i = 50; i < 100; ++i)
        {
            set.remove_at(i);
        }
        REQUIRE_FALSE(generic_set->is_compact(&set));
        REQUIRE(generic_set->compact_top(&set));
        REQUIRE_EQ(set.size(), 25);
        REQUIRE_EQ(set.sparse_size(), 50);
        REQUIRE_EQ(set.hole_size(), 25);
    }

    SUBCASE("add")
    {
        SetType set = {};

        // add
        {
            int32_t value = 114514;
            generic_set->add(&set, &value);
            REQUIRE_EQ(set.size(), 1);
            REQUIRE_GE(set.capacity(), 1);
            REQUIRE(set.contains(value));
        }

        // add move
        {
            int32_t value = 123456;
            generic_set->add_move(&set, &value);
            REQUIRE_EQ(set.size(), 2);
            REQUIRE_GE(set.capacity(), 2);
            REQUIRE(set.contains(114514));
            REQUIRE(set.contains(value));
        }

        // add duplicated
        {
            int32_t value = 114514;
            generic_set->add(&set, &value);
            REQUIRE_EQ(set.size(), 2);
            REQUIRE_GE(set.capacity(), 2);
            REQUIRE(set.contains(114514));
            REQUIRE(set.contains(123456));
        }
    }

    SUBCASE("append")
    {
        SetType a = { 1, 2, 3, 4, 5 };
        SetType b = { 1, 2, 3, 6, 7, 8, 9, 10 };

        generic_set->append(&a, &b);
        REQUIRE_EQ(a.size(), 10);
        REQUIRE_GE(a.capacity(), 10);
        for (int32_t i = 1; i <= 10; ++i)
        {
            REQUIRE(a.contains(i));
        }
    }

    SUBCASE("remove")
    {
        SetType set             = { 1, 2, 3, 4, 5, 6 };
        int32_t value_to_remove = 3;
        generic_set->remove(&set, &value_to_remove);
        REQUIRE_EQ(set.size(), 5);
        REQUIRE_GE(set.capacity(), 5);
        REQUIRE(set.contains(1));
        REQUIRE(set.contains(2));
        REQUIRE_FALSE(set.contains(3));
        REQUIRE(set.contains(4));
        REQUIRE(set.contains(5));
        REQUIRE(set.contains(6));
    }

    SUBCASE("contains")
    {
        SetType set = { 1, 2, 3, 4, 5, 6 };

        // contains
        int32_t value_to_check = 3;
        REQUIRE(generic_set->contains(&set, &value_to_check));
        value_to_check = 7;
        REQUIRE_FALSE(generic_set->contains(&set, &value_to_check));
    }

    SUBCASE("set ops")
    {
        SetType full = { 1, 2, 3, 4, 5, 6 };
        SetType a    = { 1, 2, 3 };
        SetType b    = { 4, 5, 6 };
        SetType c    = { 1, 2, 3, 4, 5, 6 };
        SetType d    = { 1, 2, 3, 4, 5, 6, 7, 8 };

        REQUIRE(generic_set->is_sub_set_of(&a, &full));
        REQUIRE(generic_set->is_sub_set_of(&b, &full));
        REQUIRE(generic_set->is_sub_set_of(&c, &full));
        REQUIRE_FALSE(generic_set->is_sub_set_of(&d, &full));
        REQUIRE_FALSE(generic_set->is_sub_set_of(&a, &b));
    }
}

TEST_CASE("test generic sparse hash map")
{
    using namespace skr;

    using MapType                        = Map<int32_t, int32_t>;
    RC<GenericSparseHashMap> generic_set = build_generic(type_signature_of<MapType>()).cast_static<GenericSparseHashMap>();

    SUBCASE("data get")
    {
        MapType map;

        // check empty data
        REQUIRE_EQ(generic_set->size(&map), map.size());
        REQUIRE_EQ(generic_set->capacity(&map), map.capacity());
        REQUIRE_EQ(generic_set->slack(&map), map.slack());
        REQUIRE_EQ(generic_set->sparse_size(&map), map.sparse_size());
        REQUIRE_EQ(generic_set->hole_size(&map), map.hole_size());
        REQUIRE_EQ(generic_set->bit_size(&map), map.bit_size());
        REQUIRE_EQ(generic_set->freelist_head(&map), map.freelist_head());
        REQUIRE_EQ(generic_set->is_compact(&map), map.is_compact());
        REQUIRE_EQ(generic_set->is_empty(&map), map.is_empty());
        REQUIRE_EQ(generic_set->storage(&map), map.data_vector().storage());
        REQUIRE_EQ(generic_set->bit_data(&map), map.data_vector().bit_data());
        REQUIRE_EQ(generic_set->bucket(&map), map.bucket());

        // check data with content
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 } };
        REQUIRE_EQ(generic_set->size(&map), map.size());
        REQUIRE_EQ(generic_set->capacity(&map), map.capacity());
        REQUIRE_EQ(generic_set->slack(&map), map.slack());
        REQUIRE_EQ(generic_set->sparse_size(&map), map.sparse_size());
        REQUIRE_EQ(generic_set->hole_size(&map), map.hole_size());
        REQUIRE_EQ(generic_set->bit_size(&map), map.bit_size());
        REQUIRE_EQ(generic_set->freelist_head(&map), map.freelist_head());
        REQUIRE_EQ(generic_set->is_compact(&map), map.is_compact());
        REQUIRE_EQ(generic_set->is_empty(&map), map.is_empty());
        REQUIRE_EQ(generic_set->storage(&map), map.data_vector().storage());
        REQUIRE_EQ(generic_set->bit_data(&map), map.data_vector().bit_data());
        REQUIRE_EQ(generic_set->bucket(&map), map.bucket());
    }

    SUBCASE("memory op")
    {
        MapType map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 } };

        // clear
        auto old_capacity = map.capacity();
        generic_set->clear(&map);
        REQUIRE_EQ(map.size(), 0);
        REQUIRE_EQ(map.sparse_size(), 0);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_EQ(map.capacity(), old_capacity);
        REQUIRE_NE(map.data_vector().storage(), nullptr);
        REQUIRE_NE(map.data_vector().bit_data(), nullptr);
        REQUIRE_NE(map.bucket(), nullptr);

        // release
        generic_set->release(&map);
        REQUIRE_EQ(map.size(), 0);
        REQUIRE_EQ(map.sparse_size(), 0);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_EQ(map.capacity(), 0);
        REQUIRE_EQ(map.data_vector().storage(), nullptr);
        REQUIRE_EQ(map.data_vector().bit_data(), nullptr);
        REQUIRE_EQ(map.bucket(), nullptr);

        // release with reserve
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 } };
        generic_set->release(&map, 10);
        REQUIRE_EQ(map.size(), 0);
        REQUIRE_EQ(map.sparse_size(), 0);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_GE(map.capacity(), 10);
        REQUIRE_NE(map.data_vector().storage(), nullptr);
        REQUIRE_NE(map.data_vector().bit_data(), nullptr);
        REQUIRE_NE(map.bucket(), nullptr);

        // reserve
        generic_set->reserve(&map, 114514);
        REQUIRE_EQ(map.size(), 0);
        REQUIRE_EQ(map.sparse_size(), 0);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_EQ(map.capacity(), 114514);
        REQUIRE_NE(map.data_vector().storage(), nullptr);
        REQUIRE_NE(map.data_vector().bit_data(), nullptr);
        REQUIRE_NE(map.bucket(), nullptr);

        // reserve with content
        map.release();
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 } };
        generic_set->reserve(&map, 114);
        REQUIRE_EQ(map.size(), 5);
        REQUIRE_GE(map.capacity(), 114);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(2));
        REQUIRE(map.contains(3));
        REQUIRE(map.contains(4));
        REQUIRE(map.contains(5));

        // shrink
        map.clear();
        generic_set->shrink(&map);
        REQUIRE_EQ(map.size(), 0);
        REQUIRE_EQ(map.sparse_size(), 0);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_EQ(map.capacity(), 0);
        REQUIRE_EQ(map.bucket(), nullptr);

        // shrink with content
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 } };
        map.reserve(10000);
        generic_set->shrink(&map);
        REQUIRE_EQ(map.size(), 5);
        REQUIRE_GE(map.capacity(), 5);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(2));
        REQUIRE(map.contains(3));
        REQUIRE(map.contains(4));
        REQUIRE(map.contains(5));

        // compact
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }, { 6, 60 } };
        map.remove(2);
        map.remove(3);
        map.remove(6);
        REQUIRE_FALSE(generic_set->is_compact(&map));
        REQUIRE(generic_set->compact(&map));
        REQUIRE_EQ(map.size(), 3);
        REQUIRE_EQ(map.sparse_size(), 3);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_GE(map.capacity(), 6);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(4));
        REQUIRE(map.contains(5));

        // compact stable
        map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }, { 6, 60 } };
        map.remove(2);
        map.remove(3);
        map.remove(6);
        REQUIRE_FALSE(generic_set->is_compact(&map));
        REQUIRE(generic_set->compact_stable(&map));
        REQUIRE_EQ(map.size(), 3);
        REQUIRE_EQ(map.sparse_size(), 3);
        REQUIRE_EQ(map.hole_size(), 0);
        REQUIRE_GE(map.capacity(), 6);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(4));
        REQUIRE(map.contains(5));

        // compact top
        map.clear();
        for (size_t i = 0; i < 100; ++i)
        {
            map.add(i, i * 10);
        }
        for (size_t i = 0; i < 50; ++i)
        {
            if (i % 2 == 0)
            {
                map.remove_at(i);
            }
        }
        for (size_t i = 50; i < 100; ++i)
        {
            map.remove_at(i);
        }
        REQUIRE_FALSE(generic_set->is_compact(&map));
        REQUIRE(generic_set->compact_top(&map));
        REQUIRE_EQ(map.size(), 25);
        REQUIRE_EQ(map.sparse_size(), 50);
        REQUIRE_EQ(map.hole_size(), 25);
    }

    SUBCASE("add")
    {
        MapType map = {};

        // add
        {
            int32_t key   = 1;
            int32_t value = 10;
            generic_set->add(&map, &key, &value);
            REQUIRE_EQ(map.size(), 1);
            REQUIRE_GE(map.capacity(), 1);
            REQUIRE(map.contains(key));
        }

        // add move
        {
            int32_t key   = 2;
            int32_t value = 20;
            generic_set->add_move(&map, &key, &value);
            REQUIRE_EQ(map.size(), 2);
            REQUIRE_GE(map.capacity(), 2);
            REQUIRE(map.contains(1));
            REQUIRE(map.contains(key));
        }

        // add duplicates
        {
            int32_t key   = 1;
            int32_t value = 100;
            generic_set->add(&map, &key, &value);
            REQUIRE_EQ(map.size(), 2);
            REQUIRE_GE(map.capacity(), 2);
            REQUIRE(map.contains(1));
            REQUIRE(map.contains(2));
            REQUIRE_EQ(map.find(1).value(), 100);
        }
    }

    SUBCASE("try add")
    {
        MapType map = {};

        // try add new
        {
            int32_t key   = 1;
            int32_t value = 10;
            auto    ref   = generic_set->try_add_unsafe(&map, &key);
            generic_set->inner_value()->copy(generic_set->inner_kv_pair()->value(ref.ptr), &value);
            REQUIRE(ref.is_valid());
            REQUIRE_EQ(ref.index, 0);
            REQUIRE_EQ(ref.ptr, &map.at(0));
            REQUIRE_EQ(map.size(), 1);
            REQUIRE(map.contains(key));
            REQUIRE_EQ(map.find(key).value(), value);
        }

        // try add move new
        {
            int32_t key   = 2;
            int32_t value = 20;
            auto    ref   = generic_set->try_add_move_unsafe(&map, &key);
            generic_set->inner_value()->copy(generic_set->inner_kv_pair()->value(ref.ptr), &value);
            REQUIRE(ref.is_valid());
            REQUIRE_EQ(ref.index, 1);
            REQUIRE_EQ(ref.ptr, &map.at(1));
            REQUIRE_EQ(map.size(), 2);
            REQUIRE(map.contains(key));
            REQUIRE_EQ(map.find(key).value(), 20);
        }

        // try add existing
        {
            int32_t key   = 1;
            int32_t value = 100;
            auto    ref   = generic_set->try_add_unsafe(&map, &key);
            generic_set->inner_value()->copy(generic_set->inner_kv_pair()->value(ref.ptr), &value);
            REQUIRE(ref.is_valid());
            REQUIRE_EQ(ref.index, 0);
            REQUIRE_EQ(ref.ptr, &map.at(0));
            REQUIRE_EQ(map.size(), 2);
            REQUIRE(map.contains(key));
            REQUIRE_EQ(map.find(key).value(), 100);
        }

        // try add move existing
        {
            int32_t key   = 2;
            int32_t value = 200;
            auto    ref   = generic_set->try_add_move_unsafe(&map, &key);
            generic_set->inner_value()->copy(generic_set->inner_kv_pair()->value(ref.ptr), &value);
            REQUIRE(ref.is_valid());
            REQUIRE_EQ(ref.index, 1);
            REQUIRE_EQ(ref.ptr, &map.at(1));
            REQUIRE_EQ(map.size(), 2);
            REQUIRE(map.contains(key));
            REQUIRE_EQ(map.find(key).value(), 200);
        }
    }

    SUBCASE("append")
    {
        MapType a = { { 1, 10 }, { 2, 20 }, { 3, 30 } };
        MapType b = { { 4, 40 }, { 5, 50 }, { 6, 60 } };

        generic_set->append(&a, &b);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(2));
        REQUIRE(a.contains(3));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(6));
        REQUIRE_EQ(a.find(1).value(), 10);
        REQUIRE_EQ(a.find(2).value(), 20);
        REQUIRE_EQ(a.find(3).value(), 30);
        REQUIRE_EQ(a.find(4).value(), 40);
        REQUIRE_EQ(a.find(5).value(), 50);
        REQUIRE_EQ(a.find(6).value(), 60);
    }

    SUBCASE("remove")
    {
        MapType map           = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }, { 6, 60 } };
        int32_t key_to_remove = 3;
        bool    removed       = generic_set->remove(&map, &key_to_remove);
        REQUIRE(removed);
        REQUIRE_EQ(map.size(), 5);
        REQUIRE_GE(map.capacity(), 5);
        REQUIRE(map.contains(1));
        REQUIRE(map.contains(2));
        REQUIRE_FALSE(map.contains(3));
        REQUIRE(map.contains(4));
        REQUIRE(map.contains(5));
        REQUIRE(map.contains(6));
        REQUIRE_EQ(map.find(1).value(), 10);
        REQUIRE_EQ(map.find(2).value(), 20);
        REQUIRE_EQ(map.find(4).value(), 40);
        REQUIRE_EQ(map.find(5).value(), 50);
        REQUIRE_EQ(map.find(6).value(), 60);
    }

    SUBCASE("contains")
    {
        MapType map = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }, { 6, 60 } };

        // contains
        int32_t key_to_check = 3;
        REQUIRE(generic_set->contains(&map, &key_to_check));
        key_to_check = 7;
        REQUIRE_FALSE(generic_set->contains(&map, &key_to_check));
    }
}