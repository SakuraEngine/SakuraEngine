#include "SkrRT/serde/json/writer.h"
#include "SkrRT/serde/json/reader.h"
#include "SkrContainers/vector.hpp"

#include "SkrTestFramework/framework.hpp"

struct JSONSerdeTests
{
protected:
    skr::Vector<uint8_t> buffer;
    skr_json_writer_t writer{3};
    simdjson::ondemand::parser parser;
    JSONSerdeTests()
    {

    }
    ~JSONSerdeTests()
    {

    }
};

TEST_CASE_METHOD(JSONSerdeTests, "primitives")
{
    uint64_t value = 0x12345678;
    uint64_t value2 = 0x87654321;
    writer.StartArray();
    skr::json::Write(&writer, value);
    skr::json::Write(&writer, value2);
    writer.EndArray();
    auto result = writer.Str();
    simdjson::padded_string str = simdjson::padded_string((char*)result.c_str(), result.raw().size());
    simdjson::ondemand::document doc = parser.iterate(str);
    simdjson::ondemand::array obj = doc.get_array();
    simdjson::ondemand::value field = obj.at(0).value_unsafe();
    simdjson::ondemand::value field2 = obj.at(1).value_unsafe();
    uint64_t readValue;
    uint64_t readValue2;
    skr::json::Read(std::move(field), readValue);
    EXPECT_EQ(value, readValue);
    skr::json::Read(std::move(field2), readValue2);
    EXPECT_EQ(value2, readValue2);
}

TEST_CASE_METHOD(JSONSerdeTests, "structure")
{
    struct Test
    {
        skr::Vector<uint64_t> arr;
        skr::String str;
    };
    Test value;
    value.arr.add(0x12345678);
    value.arr.add(0x87654321);
    value.str = u8"test";
    writer.StartObject();
    writer.Key(u8"arr");
    skr::json::Write(&writer, value.arr);
    writer.Key(u8"str");
    skr::json::Write(&writer, value.str);
    writer.EndObject();
    auto result = writer.Str();
    simdjson::padded_string str = simdjson::padded_string((char*)result.c_str(), result.raw().size());
    simdjson::ondemand::document doc = parser.iterate(str);
    simdjson::ondemand::object obj = doc.get_object();
    simdjson::ondemand::value field = obj["arr"].value_unsafe();
    skr::Vector<uint64_t> readArr;
    skr::json::Read(std::move(field), readArr);
    EXPECT_EQ(value.arr[0], readArr[0]);
    EXPECT_EQ(value.arr[1], readArr[1]);
    field = obj["str"].value_unsafe();
    skr::String readStr;
    skr::json::Read(std::move(field), readStr);
    EXPECT_EQ(value.str, readStr);
}