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

template <typename T>
struct TestPrimitiveType {
    TestPrimitiveType(T value)
        : value(value)
    {
        _SJsonWriter writer(1);
        {
            const char8_t* key = u8"key";
            writer.StartObject(u8"");
            writer.WriteValue(key, value);
            writer.EndObject();
        }
        {
            auto json  = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE JSON: %s", json.c_str());

            T _value;

            skr::json::_Reader reader(json.view());
            reader.StartObject(u8"");
            reader.ReadValue(u8"key", _value); // TODO: skr::json::Read
            reader.EndObject();

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
        
        _SJsonWriter writer(1);
        {
            const char8_t* key = u8"key";
            writer.StartObject(u8"");
            writer.StartArray(key, values.data(), values.size());
            writer.EndArray();
            writer.EndObject();
        }
        {
            auto json  = writer.Write();
            SKR_LOG_INFO(u8"PRIMITIVE ARRAY JSON: %s", json.c_str());
            skr::json::_Reader reader(json.view());

            skr::Vector<T> _values;
            size_t count;
            reader.StartObject(u8"");
            reader.StartArray(u8"key", count); // TODO: skr::json::Read
            _values.resize_zeroed(count);
            reader.ReadArray(_values.data(), count);
            reader.EndArray();
            reader.EndObject();

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
    // TestPrimitiveArray<uint8_t>(123, 234);
    // TestPrimitiveArray<uint16_t>(123, 32, 311, 22);
    TestPrimitiveArray<uint32_t>(123, 55, 22);
    TestPrimitiveArray<uint64_t>(123);
    TestPrimitiveArray<float>(234.2f, 32123.f);
    TestPrimitiveArray<double>(123.8, 352.32);
    // TestPrimitiveArray<bool>(true, false, true, false);
    // TestPrimitiveArray<bool>(false, false, true, true);
    // TestPrimitiveArray<skr::String>(u8"12345", u8"😀emoji");
    // TestPrimitiveArray<skr::String>(u8"Text", u8"#@@!*&の");
}

TEST_CASE_METHOD(JSONTests, "errors")
{
    using namespace skr::json;
    
    _Reader(u8"{}").EndArray()
        .error_then([](EReadError error) {
            EXPECT_EQ(error, EReadError::NoOpenScope);
        })
        .and_then([]() {
            EXPECT_EQ(true, false);
        });

    _Reader(u8"{}").EndObject()
        .error_then([](EReadError error) {
            EXPECT_EQ(error, EReadError::NoOpenScope);
        })
        .and_then([]() {
            EXPECT_EQ(true, false);
        });
}