#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrCore/log.hpp"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer {
    ProcInitializer()
    {
        // ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        // ::skr_finalize_crash_handler();
    }
} init;

struct JSONTests {
    JSONTests()
    {
        skr_log_set_level(SKR_LOG_LEVEL_INFO);
    }
};

void _EXPECT_OK(skr::json::ReadResult&& r)
{
    using namespace skr::json;
    r.and_then([]() {
        EXPECT_EQ(true, true);
    })
    .error_then([](ErrorCode error) {
        EXPECT_EQ(true, false);
    });
}

void _EXPECT_ERROR(skr::json::ReadResult&& r, skr::json::ErrorCode err)
{
    using namespace skr::json;
    r.and_then([]() {
        EXPECT_EQ(true, false);
    })
    .error_then([=](ErrorCode error) {
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
        skr::json::_Writer writer(1);
        {
            const char8_t* key = u8"key";
            EXPECT_OK(writer.StartObject(u8""));
            EXPECT_OK(writer.WriteValue(key, value));
            EXPECT_OK(writer.EndObject());
        }
        {
            auto json  = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE JSON: %s", json.c_str());

            T _value;

            skr::json::_Reader reader(json.view());
            EXPECT_OK(reader.StartObject(u8""));
            EXPECT_OK(reader.ReadValue(u8"key", _value)); // TODO: skr::json::Read
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
        
        skr::json::_Writer writer(1);
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
            auto json  = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE ARRAY JSON: %s", json.c_str());
            skr::json::_Reader reader(json.view());

            skr::Vector<T> _values;
            size_t count;
            EXPECT_OK(reader.StartObject(u8""));
            EXPECT_OK(reader.StartArray(u8"key", count)); // TODO: skr::json::Read
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
                T value = values[i];
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
    using namespace skr::json;
    SUBCASE("NoOpenScope")
    {
        EXPECT_ERROR(_Reader(u8"{}").EndArray(), ErrorCode::NoOpenScope);
        EXPECT_ERROR(_Reader(u8"{}").EndObject(), ErrorCode::NoOpenScope);
    }
    SUBCASE("ScopeTypeMismatch(StartArray)")
    {
        auto obj_reader = _Reader(u8"{ \"obj\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_ERROR(obj_reader.StartArray(u8"obj", arr_len), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(EndObjectAsArray)")
    {
        auto obj_reader = _Reader(u8"{}");
        EXPECT_OK(obj_reader.StartObject(u8""));
        EXPECT_ERROR(obj_reader.EndArray(), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(StartArrayAsObject)")
    {
        auto arr_reader = _Reader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_ERROR(arr_reader.StartObject(u8"arr"), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(StartPrimitiveAsObject/Array)")
    {
        size_t arr_len = 0;
        auto value_reader =  _Reader(u8"{ \"value\": 123 }");
        EXPECT_OK(value_reader.StartObject(u8""));
        EXPECT_ERROR(value_reader.StartObject(u8"value"), ErrorCode::ScopeTypeMismatch);
        EXPECT_ERROR(value_reader.StartArray(u8"value", arr_len), ErrorCode::ScopeTypeMismatch);
            
        auto arr_reader = _Reader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 4);
        EXPECT_ERROR(arr_reader.StartObject(u8""), ErrorCode::ScopeTypeMismatch);
        EXPECT_ERROR(arr_reader.StartArray(u8"", arr_len), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("ScopeTypeMismatch(EndArrayAsObject)")
    {
        auto arr_reader = _Reader(u8"{ \"arr\": [ 0, 1, 2, 3 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 4);
        EXPECT_ERROR(arr_reader.EndObject(), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("KeyNotFound(ObjectField)")
    {
        auto obj_reader = _Reader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        skr::String out;
        EXPECT_ERROR(obj_reader.ReadValue(u8"key_mismatch", out), ErrorCode::KeyNotFound);
        size_t arr_len = 0;
        EXPECT_ERROR(obj_reader.StartArray(u8"key_mismatch", arr_len), ErrorCode::KeyNotFound);
        EXPECT_ERROR(obj_reader.StartObject(u8"key_mismatch"), ErrorCode::KeyNotFound);
    }
    SUBCASE("KeyNotFound(ArrayIndexOverflow)")
    {
        auto arr_reader = _Reader(u8"{ \"arr\": [ 0 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        size_t arr_len = 0;
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 1);
        int32_t first_element = 222;
        EXPECT_OK(arr_reader.ReadInt32(u8"", first_element));
        EXPECT_EQ(first_element, 0);
        EXPECT_ERROR(arr_reader.ReadInt32(u8"", first_element), ErrorCode::KeyNotFound);
    }
    SUBCASE("EmptyObjectFieldKey")
    {
        size_t arr_len = 0;
        auto obj_reader = _Reader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject(u8""));
        EXPECT_ERROR(obj_reader.StartObject(u8""), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.StartArray(u8"", arr_len), ErrorCode::EmptyObjectFieldKey);
        
        bool b; int32_t i32; int64_t i64; float f; double d; skr::String s;
        EXPECT_ERROR(obj_reader.ReadValue(u8"", b), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", i32), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", i64), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", f), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", d), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_reader.ReadValue(u8"", s), ErrorCode::EmptyObjectFieldKey);
    }
    SUBCASE("ArrayElementWithKey")
    {
        size_t arr_len = 0;
        auto arr_reader = _Reader(u8"{ \"arr\": [ 0 ] }");
        EXPECT_OK(arr_reader.StartObject(u8""));
        EXPECT_OK(arr_reader.StartArray(u8"arr", arr_len));
        EXPECT_EQ(arr_len, 1);
        
        bool b; int32_t i32; int64_t i64; float f; double d; skr::String s;
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", b), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", i32), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", i64), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", f), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", d), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.ReadValue(u8"k", s), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.StartObject(u8"k"), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_reader.StartArray(u8"k", arr_len), ErrorCode::ArrayElementWithKey);
    }
    SUBCASE("RootObjectWithKey")
    {
        auto obj_reader = _Reader(u8"{ \"key\": \"value\" }");
        EXPECT_ERROR(obj_reader.StartObject(u8"key"), ErrorCode::RootObjectWithKey);
    }
    SUBCASE("PresetKeyIsEmpty")
    {
        auto obj_reader = Reader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject());
        EXPECT_ERROR(obj_reader.Key(u8""), ErrorCode::PresetKeyIsEmpty);
    }
    SUBCASE("PresetKeyNotConsumedYet")
    {
        auto obj_reader = Reader(u8"{ \"key\": \"value\" }");
        EXPECT_OK(obj_reader.StartObject());
        EXPECT_OK(obj_reader.Key(u8"key"));
        EXPECT_ERROR(obj_reader.Key(u8"key"), ErrorCode::PresetKeyNotConsumedYet);
    }
}

TEST_CASE_METHOD(JSONTests, "WriteErrors")
{
    using namespace skr::json;
    SUBCASE("NoOpenScope")
    {
        EXPECT_ERROR(_Writer(1).EndArray(), ErrorCode::NoOpenScope);
        EXPECT_ERROR(_Writer(1).EndObject(), ErrorCode::NoOpenScope);
    }
    SUBCASE("ScopeTypeMismatch")
    {
        auto obj_writer = _Writer(3);
        EXPECT_OK(obj_writer.StartObject(u8""));
        EXPECT_OK(obj_writer.StartObject(u8"obj"));
        EXPECT_ERROR(obj_writer.EndArray(), ErrorCode::ScopeTypeMismatch);

        auto arr_writer = _Writer(3);
        EXPECT_OK(arr_writer.StartObject(u8""));
        EXPECT_OK(arr_writer.StartArray(u8"arr"));
        EXPECT_ERROR(arr_writer.EndObject(), ErrorCode::ScopeTypeMismatch);
    }
    SUBCASE("EmptyObjectFieldKey")
    {
        auto obj_writer = _Writer(3);
        EXPECT_OK(obj_writer.StartObject(u8""));
        EXPECT_ERROR(obj_writer.StartObject(u8""), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.StartArray(u8""), ErrorCode::EmptyObjectFieldKey);
        
        bool b = false; int32_t i32 = 0; int64_t i64 = 0; float f = 0.f; double d = 0.0; skr::String s = u8"";
        EXPECT_ERROR(obj_writer.WriteValue(u8"", b), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", i32), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", i64), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", f), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", d), ErrorCode::EmptyObjectFieldKey);
        EXPECT_ERROR(obj_writer.WriteValue(u8"", s), ErrorCode::EmptyObjectFieldKey);
    }
    SUBCASE("ArrayElementWithKey")
    {
        auto arr_writer = _Writer(3);
        EXPECT_OK(arr_writer.StartObject(u8""));
        EXPECT_OK(arr_writer.StartArray(u8"arr"));
        EXPECT_ERROR(arr_writer.StartObject(u8"k"), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.StartArray(u8"k"), ErrorCode::ArrayElementWithKey);

        bool b = false; int32_t i32 = 0; int64_t i64 = 0; float f = 0.f; double d = 0.0; skr::String s = u8"";
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", b), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", i32), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", i64), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", f), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", d), ErrorCode::ArrayElementWithKey);
        EXPECT_ERROR(arr_writer.WriteValue(u8"k", s), ErrorCode::ArrayElementWithKey);
    }
    SUBCASE("RootObjectWithKey")
    {
        auto obj_writer = _Writer(3);
        EXPECT_ERROR(obj_writer.StartObject(u8"key"), ErrorCode::RootObjectWithKey);
    }
    SUBCASE("PresetKeyIsEmpty")
    {
        auto obj_writer = Writer(3);
        EXPECT_OK(obj_writer.StartObject());
        EXPECT_ERROR(obj_writer.Key(u8""), ErrorCode::PresetKeyIsEmpty);
    }
    SUBCASE("PresetKeyNotConsumedYet")
    {
        auto obj_writer = Writer(3);
        EXPECT_OK(obj_writer.StartObject());
        EXPECT_OK(obj_writer.Key(u8"key"));
        EXPECT_ERROR(obj_writer.Key(u8"key"), ErrorCode::PresetKeyNotConsumedYet);
    }
}