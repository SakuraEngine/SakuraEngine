#include "SkrRT/serde/binary/writer.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/span.hpp"
#include "SkrRT/containers/vector.hpp"
#include "gtest/gtest.h"

class BINARY : public ::testing::Test
{
protected:
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer;
    skr::binary::SpanReader reader;
    skr_binary_writer_t warchive{writer};
    skr_binary_reader_t rarchive{reader};
    void SetUp() override
    {
        writer.buffer = &buffer;
    }

    void TearDown() override
    {
    }
};

TEST_F(BINARY, primitives)
{
    uint64_t value = 0x12345678;
    uint64_t value2 = 0x87654321;
    skr::binary::Archive(&warchive, value);
    skr::binary::Archive(&warchive, value2);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue = 0;
    uint64_t readValue2 = 0;
    skr::binary::Archive(&rarchive, readValue);
    EXPECT_EQ(value, readValue);
    skr::binary::Archive(&rarchive, readValue2);
    EXPECT_EQ(value2, readValue2);
}

TEST_F(BINARY, arr)
{
    uint64_t value = 0x12345678;
    uint64_t value2 = 0x87654321;
    skr::vector<uint64_t> arr;
    arr.push_back(value);
    arr.push_back(value2);
    skr::binary::Archive(&warchive, arr);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    skr::vector<uint64_t> readArr;
    skr::binary::Archive(&rarchive, readArr);

    EXPECT_EQ(value, readArr[0]);
    EXPECT_EQ(value2, readArr[1]);
}

TEST_F(BINARY, str)
{
    skr::string str = u8"Hello World";
    skr::binary::Archive(&warchive, str);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    skr::string readStr;
    skr::binary::Archive(&rarchive, readStr);

    EXPECT_EQ(str, readStr);
}

TEST_F(BINARY, str_arr)
{
    skr::vector<skr::string> arr;
    arr.push_back(u8"Hello World");
    arr.push_back(u8"Hello World2");
    skr::binary::Archive(&warchive, arr);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    skr::vector<skr::string> readArr;
    skr::binary::Archive(&rarchive, readArr);

    EXPECT_EQ(arr[0], readArr[0]);
    EXPECT_EQ(arr[1], readArr[1]);
}