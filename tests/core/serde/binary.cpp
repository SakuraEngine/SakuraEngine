#include "SkrTestFramework/framework.hpp"

#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/vector.hpp"

#include "SkrSerde/bin_serde.hpp"

struct BinarySerdeTests {
protected:
    skr::Vector<uint8_t>          buffer;
    skr::archive::BinVectorWriter vec_writer_impl;
    skr::archive::BinSpanReader   vec_reader_impl;
    SBinaryWriter                 writer{ vec_writer_impl };
    SBinaryReader                 reader{ vec_reader_impl };
    BinarySerdeTests()
    {
        vec_writer_impl.buffer = &buffer;
    }

    ~BinarySerdeTests()
    {
    }
};

TEST_CASE_METHOD(BinarySerdeTests, "primitives")
{
    uint64_t value  = 0x12345678;
    uint64_t value2 = 0x87654321;
    skr::bin_write(&writer, value);
    skr::bin_write(&writer, value2);

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue  = 0;
    uint64_t readValue2 = 0;
    skr::bin_read(&reader, readValue);
    EXPECT_EQ(value, readValue);
    skr::bin_read(&reader, readValue2);
    EXPECT_EQ(value2, readValue2);
}

TEST_CASE_METHOD(BinarySerdeTests, "vector")
{
    uint64_t              value  = 0x12345678;
    uint64_t              value2 = 0x87654321;
    skr::Vector<uint64_t> arr;
    arr.add(value);
    arr.add(value2);
    EXPECT_TRUE(skr::bin_write(&writer, arr));

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<uint64_t> readArr;
    EXPECT_TRUE(skr::bin_read(&reader, readArr));

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
    skr::bin_write(&writer, arr);

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<uint64_t> readArr;
    skr::bin_read(&reader, readArr);

    EXPECT_EQ(value, readArr[0]);
    EXPECT_EQ(value2, readArr[1]);
}

TEST_CASE_METHOD(BinarySerdeTests, "str")
{
    skr::String str = u8"Hello World";
    skr::bin_write(&writer, str);

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::String readStr;
    skr::bin_read(&reader, readStr);

    EXPECT_EQ(str, readStr);
}

TEST_CASE_METHOD(BinarySerdeTests, "str_vec")
{
    skr::Vector<skr::String> arr;
    arr.add(u8"Hello World");
    arr.add(u8"Hello World2");
    skr::bin_write(&writer, arr);

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<skr::String> readArr;
    skr::bin_read(&reader, readArr);

    EXPECT_EQ(arr[0], readArr[0]);
    EXPECT_EQ(arr[1], readArr[1]);
}

TEST_CASE_METHOD(BinarySerdeTests, "str_arr")
{
    skr::Vector<skr::String> arr;
    arr.add(u8"Hello World");
    arr.add(u8"Hello World2");
    skr::bin_write(&writer, arr);

    vec_reader_impl.data = skr::span<const uint8_t>(buffer.data(), buffer.size());

    skr::Vector<skr::String> readArr;
    skr::bin_read(&reader, readArr);

    EXPECT_EQ(arr[0], readArr[0]);
    EXPECT_EQ(arr[1], readArr[1]);
}