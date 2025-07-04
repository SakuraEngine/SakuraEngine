#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrCore/log.hpp"
#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer {
    ProcInitializer()
    {
        // ::skr_initialize_crash_handler();
    }
    ~ProcInitializer()
    {
        // ::skr_finalize_crash_handler();
    }
} init;

struct JSONTests {
    JSONTests()
    {
        skr_log_set_level(SKR_LOG_LEVEL_INFO);
    }
};

void _EXPECT_OK(skr::archive::JsonReadResult&& r)
{
    using namespace skr::archive;
    r.and_then([]() {
         EXPECT_EQ(true, true);
     })
    .error_then([](JsonErrorCode error) {
        EXPECT_EQ(true, false);
    });
}

void _EXPECT_ERROR(skr::archive::JsonReadResult&& r, skr::archive::JsonErrorCode err)
{
    using namespace skr::archive;
    r.and_then([]() {
         EXPECT_EQ(true, false);
     })
    .error_then([=](JsonErrorCode error) {
        EXPECT_EQ(error, err);
    });
}

// for better ide display
#define EXPECT_OK _EXPECT_OK
#define EXPECT_ERROR _EXPECT_ERROR

template <typename T>
struct TestPrimitiveType {
    TestPrimitiveType(T value)
        : value(value)
    {
        skr::archive::_JsonWriter writer(1);
        {
            const char8_t* key = u8"key";
            EXPECT_OK(writer.StartObject(u8""));
            EXPECT_OK(writer.WriteValue(key, value));
            EXPECT_OK(writer.EndObject());
        }
        {
            auto json = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE JSON: %s", json.c_str());

            T _value;

            skr::archive::_JsonReader reader(json.view());
            EXPECT_OK(reader.StartObject(u8""));
            EXPECT_OK(reader.ReadValue(u8"key", _value)); // TODO: skr::archive::Read
            EXPECT_OK(reader.EndObject());

            if constexpr (std::is_floating_point_v<T>)
                EXPECT_NEAR(value, _value, 0.0001);
            else
                EXPECT_EQ(value, _value);
        }
    }
    T value;
};

TEST_CASE_METHOD(JSONTests, "primitive")
{
    TestPrimitiveType<int>(123);
    TestPrimitiveType<int64_t>(123);
    TestPrimitiveType<uint8_t>(123);
    TestPrimitiveType<uint16_t>(123);
    TestPrimitiveType<uint32_t>(123);
    TestPrimitiveType<uint64_t>(123);
    TestPrimitiveType<float>(234.2f);
    TestPrimitiveType<double>(123.8);
    TestPrimitiveType<bool>(true);
    TestPrimitiveType<bool>(false);
    TestPrimitiveType<skr::String>(u8"12345");
    TestPrimitiveType<skr::String>(u8"SerdeTest");
    TestPrimitiveType<skr::String>(u8"😀emoji");
}

template <typename T>
struct TestPrimitiveArray {
    template <typename Arg>
    void push_args(const Arg& v)
    {
        values.emplace(v);
    }

    template <typename Arg, typename... Args>
    void push_args(const Arg& a, Args... params)
    {
        push_args(a);
        push_args(std::forward<Args>(params)...);
    }

    template <typename... Args>
    TestPrimitiveArray(Args... params)
    {
        push_args(std::forward<Args>(params)...);

        skr::archive::_JsonWriter writer(1);
        {
            const char8_t* key = u8"key";
            EXPECT_OK(writer.StartObject(u8""));
            if constexpr (std::is_same_v<T, skr::String>)
            {
                EXPECT_OK(writer.StartArray(key));
                for (size_t i = 0; i < sizeof...(Args); i++)
                {
                    EXPECT_OK(writer.WriteString(u8"", values[i]));
                }
            }
            else
                EXPECT_OK(writer.StartArray(key, values.data(), values.size()));
            EXPECT_OK(writer.EndArray());
            EXPECT_OK(writer.EndObject());
        }
        {
            auto json = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE ARRAY JSON: %s", json.c_str());
            skr::archive::_JsonReader reader(json.view());

            skr::Vector<T> _values;
            size_t         count;
            EXPECT_OK(reader.StartObject(u8""));
            EXPECT_OK(reader.StartArray(u8"key", count)); // TODO: skr::archive::Read
            _values.resize_zeroed(count);
            if constexpr (std::is_same_v<T, skr::String>)
            {
                for (size_t i = 0; i < count; i++)
                {
                    skr::String _value;
                    EXPECT_OK(reader.ReadString(u8"", _value));
                    _values[i] = _value;
                }
            }
            else
                EXPECT_OK(reader.ReadArray(_values.data(), count));
            EXPECT_OK(reader.EndArray());
            EXPECT_OK(reader.EndObject());

            for (size_t i = 0; i < values.size(); i++)
            {
                T value  = values[i];
                T _value = _values[i];
                if constexpr (std::is_floating_point_v<T>)
                    EXPECT_NEAR(value, _value, 0.0001);
                else
                    EXPECT_EQ(value, _value);
            }
        }
    }
    skr::Vector<T> values;
};

TEST_CASE_METHOD(JSONTests, "array")
{
    skr::Vector<int> is;
    TestPrimitiveArray<int>(123, 345, -323);
    TestPrimitiveArray<int64_t>(123, 456, -32);
    TestPrimitiveArray<uint32_t>(123, 55, 22);
    TestPrimitiveArray<uint64_t>(123);
    TestPrimitiveArray<float>(234.2f, 32123.f);
    TestPrimitiveArray<double>(123.8, 352.32);
    // TestPrimitiveArray<uint8_t>(123, 234);
    // TestPrimitiveArray<uint16_t>(123, 32, 311, 22);
    // TestPrimitiveArray<bool>(true, false, true, false);
    // TestPrimitiveArray<bool>(false, false, true, true);
    TestPrimitiveArray<skr::String>(u8"12345", u8"😀emoji");
    TestPrimitiveArray<skr::String>(u8"Text", u8"#@@!*&の");
}

TEST_CASE_METHOD(JSONTests, "ReadErrors")
{
    using namespace skr::archive;
    SUBCASE("NoOpenScope")
    {
        EXPECT_ERROR(_JsonReader(u8"{}").EndArray(), JsonErrorCode::NoOpenScope);
        EXPECT_ERROR(_JsonReader(u8"{}").EndObject(), JsonErrorCode::NoOpenScope);
    }
    SUBCASE("ScopeTypeMismatch(StartArray)")
    {
        auto obj_reader = _JsonReader(u8"{ \"obj\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_ERROR(obj_reader.StartArray(u8"obj", arr_len), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(EndObjectAsArray)")
    {
        auto obj_reader = _JsonReader(u8"{}");
        EXPECT_OK(obj_reader.StartObject(u8""));
        EXPECT_ERROR(obj_reader.EndArray(), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(StartArrayAsObject)")
    {
        auto arr_reader = _JsonReader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_ERROR(arr_reader.StartObject(u8"arr"), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(StartPrimitiveAsObject/Array)")
    {
        size_t arr_len      = 0;
        auto   value_reader = _JsonReader(u8"{ \"value\": 123 }");
        EXPECT_OK(value_reader.StartObject(u8""));
        EXPECT_ERROR(value_reader.StartObject(u8"value"), JsonErrorCode::ScopeTypeMismatch);
        EXPECT_ERROR(value_reader.StartArray(u8"value", arr_len), JsonErrorCode::ScopeTypeMismatch);

        auto arr_reader = _JsonReader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 4);
        EXPECT_ERROR(arr_reader.StartObject(u8""), JsonErrorCode::ScopeTypeMismatch);
        EXPECT_ERROR(arr_reader.StartArray(u8"", arr_len), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(EndArrayAsObject)")
    {
        auto arr_reader = _JsonReader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 4);
        EXPECT_ERROR(arr_reader.EndObject(), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("KeyNotFound(ObjectField)")
    {
        auto obj_reader = _JsonReader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        skr::String out;
        EXPECT_ERROR(obj_reader.ReadValue(u8"key_mismatch", out), JsonErrorCode::KeyNotFound);
        size_t arr_len = 0;
        EXPECT_ERROR(obj_reader.StartArray(u8"key_mismatch", arr_len), JsonErrorCode::KeyNotFound);
        EXPECT_ERROR(obj_reader.StartObject(u8"key_mismatch"), JsonErrorCode::KeyNotFound);
    }
    SUBCASE("KeyNotFound(ArrayIndexOverflow)")
    {
        auto arr_reader = _JsonReader(u8"{ \"arr\": [ 0 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 1);
        int32_t first_element = 222;
        EXPECT_OK(arr_reader.ReadInt32(u8"", first_element));
        EXPECT_EQ(first_element, 0);
        EXPECT_ERROR(arr_reader.ReadInt32(u8"", first_element), JsonErrorCode::KeyNotFound);
    }
    SUBCASE("EmptyObjectFieldKey")
    {
        size_t arr_len    = 0;
        auto   obj_reader = _JsonReader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        EXPECT_ERROR(obj_reader.StartObject(u8""), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.StartArray(u8"", arr_len), JsonErrorCode::EmptyObjectFieldKey);

        bool        b;
        int32_t     i32;
        int64_t     i64;
        float       f;
        double      d;
        skr::String s;
        EXPECT_ERROR(obj_reader.ReadValue(u8"", b), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", i32), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", i64), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", f), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", d), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", s), JsonErrorCode::EmptyObjectFieldKey);
    }
    SUBCASE("ArrayElementWithKey")
    {
        size_t arr_len    = 0;
        auto   arr_reader = _JsonReader(u8"{ \"arr\": [ 0 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 1);

        bool        b;
        int32_t     i32;
        int64_t     i64;
        float       f;
        double      d;
        skr::String s;
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", b), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", i32), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", i64), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", f), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", d), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", s), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.StartObject(u8"k"), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.StartArray(u8"k", arr_len), JsonErrorCode::ArrayElementWithKey);
    }
    SUBCASE("RootObjectWithKey")
    {
        auto obj_reader = _JsonReader(u8"{ \"key\": \"value\" }");
        EXPECT_ERROR(obj_reader.StartObject(u8"key"), JsonErrorCode::RootObjectWithKey);
    }
    SUBCASE("PresetKeyIsEmpty")
    {
        auto obj_reader = JsonReader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject());
        EXPECT_ERROR(obj_reader.Key(u8""), JsonErrorCode::PresetKeyIsEmpty);
    }
    SUBCASE("PresetKeyNotConsumedYet")
    {
        auto obj_reader = JsonReader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject());
        EXPECT_OK(obj_reader.Key(u8"key"));
        EXPECT_ERROR(obj_reader.Key(u8"key"), JsonErrorCode::PresetKeyNotConsumedYet);
    }
}

TEST_CASE_METHOD(JSONTests, "WriteErrors")
{
    using namespace skr::archive;
    SUBCASE("NoOpenScope")
    {
        EXPECT_ERROR(_JsonWriter(1).EndArray(), JsonErrorCode::NoOpenScope);
        EXPECT_ERROR(_JsonWriter(1).EndObject(), JsonErrorCode::NoOpenScope);
    }
    SUBCASE("ScopeTypeMismatch")
    {
        auto obj_writer = _JsonWriter(3);
        EXPECT_OK(obj_writer.StartObject(u8""));
        EXPECT_OK(obj_writer.StartObject(u8"obj"));
        EXPECT_ERROR(obj_writer.EndArray(), JsonErrorCode::ScopeTypeMismatch);

        auto arr_writer = _JsonWriter(3);
        EXPECT_OK(arr_writer.StartObject(u8""));
        EXPECT_OK(arr_writer.StartArray(u8"arr"));
        EXPECT_ERROR(arr_writer.EndObject(), JsonErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("EmptyObjectFieldKey")
    {
        auto obj_writer = _JsonWriter(3);
        EXPECT_OK(obj_writer.StartObject(u8""));
        EXPECT_ERROR(obj_writer.StartObject(u8""), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.StartArray(u8""), JsonErrorCode::EmptyObjectFieldKey);

        bool        b   = false;
        int32_t     i32 = 0;
        int64_t     i64 = 0;
        float       f   = 0.f;
        double      d   = 0.0;
        skr::String s   = u8"";
        EXPECT_ERROR(obj_writer.WriteValue(u8"", b), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", i32), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", i64), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", f), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", d), JsonErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", s), JsonErrorCode::EmptyObjectFieldKey);
    }
    SUBCASE("ArrayElementWithKey")
    {
        auto arr_writer = _JsonWriter(3);
        EXPECT_OK(arr_writer.StartObject(u8""));
        EXPECT_OK(arr_writer.StartArray(u8"arr"));
        EXPECT_ERROR(arr_writer.StartObject(u8"k"), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.StartArray(u8"k"), JsonErrorCode::ArrayElementWithKey);

        bool        b   = false;
        int32_t     i32 = 0;
        int64_t     i64 = 0;
        float       f   = 0.f;
        double      d   = 0.0;
        skr::String s   = u8"";
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", b), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", i32), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", i64), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", f), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", d), JsonErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", s), JsonErrorCode::ArrayElementWithKey);
    }
    SUBCASE("RootObjectWithKey")
    {
        auto obj_writer = _JsonWriter(3);
        EXPECT_ERROR(obj_writer.StartObject(u8"key"), JsonErrorCode::RootObjectWithKey);
    }
    SUBCASE("PresetKeyIsEmpty")
    {
        auto obj_writer = JsonWriter(3);
        EXPECT_OK(obj_writer.StartObject());
        EXPECT_ERROR(obj_writer.Key(u8""), JsonErrorCode::PresetKeyIsEmpty);
    }
    SUBCASE("PresetKeyNotConsumedYet")
    {
        auto obj_writer = JsonWriter(3);
        EXPECT_OK(obj_writer.StartObject());
        EXPECT_OK(obj_writer.Key(u8"key"));
        EXPECT_ERROR(obj_writer.Key(u8"key"), JsonErrorCode::PresetKeyNotConsumedYet);
    }
}