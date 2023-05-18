#include "serde/binary/writer.h"
#include "serde/binary/reader.h"
#include "containers/span.hpp"
#include "containers/vector.hpp"
#include "gtest/gtest.h"

class BINARY_BITPACK : public ::testing::Test
{
protected:
    eastl::vector<uint8_t> buffer;
    skr::binary::VectorWriterBitpacked writer;
    skr::binary::SpanReaderBitpacked reader;
    void SetUp() override
    {
        writer.buffer = &buffer;
    }

    void TearDown() override
    {
    }
};

TEST_F(BINARY_BITPACK, Aligned)
{
    uint64_t value = 0x12345678;
    uint64_t value2 = 0x87654321;
    writer.write(&value, sizeof(value));
    writer.write(&value2, sizeof(value2));

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue = 0;
    uint64_t readValue2 = 0;
    reader.read(&readValue, sizeof(value));
    EXPECT_EQ(value, readValue);
    reader.read(&readValue2, sizeof(value2));
    EXPECT_EQ(value2, readValue2);
}

TEST_F(BINARY_BITPACK, WithinByte)
{
    uint64_t value = 0b11;
    uint64_t value2 = 0b1010;
    writer.write_bits(&value, 2);
    writer.write_bits(&value2, 4);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue = 0;
    uint64_t readValue2 = 0;
    reader.read_bits(&readValue, 2);
    reader.read_bits(&readValue2, 4);
    EXPECT_EQ(value, readValue);
    EXPECT_EQ(value2, readValue2);
}

TEST_F(BINARY_BITPACK, MultipleBytes)
{
    uint64_t value = 0x03FFFFF00000FFFF;
    uint64_t value2 = 0x0000000000000300;
    writer.write_bits(&value, 14*4 + 2);
    writer.write_bits(&value2, 2*4 + 2);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    uint64_t readValue = 0;
    uint64_t readValue2 = 0;
    reader.read_bits(&readValue, 14*4 + 2);
    reader.read_bits(&readValue2, 2*4 + 2);
    EXPECT_EQ(value, readValue);
    EXPECT_EQ(value2, readValue2);
}

TEST_F(BINARY_BITPACK, ArchiveAPI)
{
    skr_binary_writer_t archiveWrite(writer);
    skr_binary_reader_t archiveRead(reader);

    uint64_t value = 0x03FFFFF00000FFFF;
    uint64_t value2 = 0x0000000000000300;
    uint64_t value3 = 0x03FFFFF00000FFFF;
    uint32_t value4 = 0x00000003;
    archiveWrite.write_bits(&value, 14*4 + 2);
    archiveWrite.write_bits(&value2, 2*4 + 2);
    archiveWrite.write(&value4, sizeof(uint32_t));
    //archiveWrite.write_bits(&value3, 14*4 + 2);

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());
    
    uint64_t readValue = 0;
    uint64_t readValue2 = 0;
    uint64_t readValue3 = 0;
    uint32_t readValue4 = 0;
    archiveRead.read_bits(&readValue, 14*4 + 2);
    EXPECT_EQ(value, readValue);
    archiveRead.read_bits(&readValue2, 2*4 + 2);
    EXPECT_EQ(value2, readValue2);
    archiveRead.read(&readValue4, sizeof(uint32_t));
    EXPECT_EQ(value4, readValue4);
    //archiveRead.read_bits(&readValue3, 14*4 + 2);
    //EXPECT_EQ(value3, readValue3);
}

TEST_F(BINARY_BITPACK, VectorPack)
{
    skr_binary_writer_t archiveWrite(writer);
    skr_binary_reader_t archiveRead(reader);
    skr_float3_t value = { 1.0f, 2.0f, 3.0f };
    skr_float3_t value2 = { 3.12345f, 2.12345f, 1.12345f };

    skr::binary::Archive(&archiveWrite, value, skr::binary::VectorSerdeConfig<float>{});
    skr::binary::Archive(&archiveWrite, value2, skr::binary::VectorSerdeConfig<float>{100000});

    reader.data = skr::span<uint8_t>(buffer.data(), buffer.size());

    skr_float3_t readValue = { 0.0f, 0.0f, 0.0f };
    skr_float3_t readValue2 = { 0.0f, 0.0f, 0.0f };
    skr::binary::Archive(&archiveRead, readValue, skr::binary::VectorSerdeConfig<float>{});
    EXPECT_FLOAT_EQ(value.x, readValue.x);
    EXPECT_FLOAT_EQ(value.y, readValue.y);
    EXPECT_FLOAT_EQ(value.z, readValue.z);
    skr::binary::Archive(&archiveRead, readValue2, skr::binary::VectorSerdeConfig<float>{100000});
    EXPECT_FLOAT_EQ(value2.x, readValue2.x);
    EXPECT_FLOAT_EQ(value2.y, readValue2.y);
    EXPECT_FLOAT_EQ(value2.z, readValue2.z);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}