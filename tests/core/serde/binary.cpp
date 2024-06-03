#include "SkrSerde/binary/writer.h"
#include "SkrSerde/binary/reader.h"
#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/vector.hpp"

#include "SkrTestFramework/framework.hpp"

struct BinarySerdeTests {
protected:
    skr::Vector<uint8_t>      buffer;
    skr::binary::VectorWriter writer;
    skr::binary::SpanReader   reader;
    SBinaryWriter       warchive{ writer };
    SBinaryReader       rarchive{ reader };
    BinarySerdeTests()
    {
        writer.buffer = &buffer;
    }

    ~BinarySerdeTests()
    {
    }
};

TEST_CASE_METHOD(BinarySerdeTests, "primitives")
{
    uint64_t value  = 0x12345678;
    uint64_t value2 = 0x87654321;
    skr::binary::Archive(&warchive, value);
    skr::binary::Archive(&warchive, value2);

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue  = 0;
    uint64_t readValue2 = 0;
    skr::binary::Archive(&rarchive, readValue);
    EXPECT_EQ(value, readValue);
    skr::binary::Archive(&rarchive, readValue2);
    EXPECT_EQ(value2, readValue2);
}

TEST_CASE_METHOD(BinarySerdeTests, "vector")
{
    uint64_t              value  = 0x12345678;
    uint64_t              value2 = 0x87654321;
    skr::Vector<uint64_t> arr;
    arr.add(value);
    arr.add(value2);
    EXPECT_TRUE(skr::binary::Archive(&warchive, arr));

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<uint64_t> readArr;
    EXPECT_TRUE(skr::binary::Archive(&rarchive, readArr));

    EXPECT_EQ(value, readArr[0]);
    EXPECT_EQ(value2, readArr[1]);
}

TEST_CASE_METHOD(BinarySerdeTests, "arr")
{
    uint64_t              value  = 0x12345678;
    uint64_t              value2 = 0x87654321;
    skr::Vector<uint64_t> arr;
    arr.add(value);
    arr.add(value2);
    skr::binary::Archive(&warchive, arr);

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<uint64_t> readArr;
    skr::binary::Archive(&rarchive, readArr);

    EXPECT_EQ(value, readArr[0]);
    EXPECT_EQ(value2, readArr[1]);
}

TEST_CASE_METHOD(BinarySerdeTests, "str")
{
    skr::String str = u8"Hello World";
    skr::binary::Archive(&warchive, str);

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::String readStr;
    skr::binary::Archive(&rarchive, readStr);

    EXPECT_EQ(str, readStr);
}

TEST_CASE_METHOD(BinarySerdeTests, "str_vec")
{
    skr::Vector<skr::String> arr;
    arr.add(u8"Hello World");
    arr.add(u8"Hello World2");
    skr::binary::Archive(&warchive, arr);

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<skr::String> readArr;
    skr::binary::Archive(&rarchive, readArr);

    EXPECT_EQ(arr[0], readArr[0]);
    EXPECT_EQ(arr[1], readArr[1]);
}

TEST_CASE_METHOD(BinarySerdeTests, "str_arr")
{
    skr::Vector<skr::String> arr;
    arr.add(u8"Hello World");
    arr.add(u8"Hello World2");
    skr::binary::Archive(&warchive, arr);

    reader.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<skr::String> readArr;
    skr::binary::Archive(&rarchive, readArr);

    EXPECT_EQ(arr[0], readArr[0]);
    EXPECT_EQ(arr[1], readArr[1]);
}