#pragma once
#include <SkrCore/serialize/serialize_traits.hpp>
#include <SkrContainersDef/vector.hpp>
#include <SkrContainersDef/span.hpp>

// TODO. 支持 steam 向序列化器传入 Stream
// TODO. 支持 Structured Bin 序列化器

// fwd & guid
namespace skr
{
struct ArWriteBin;
struct ArReadBin;
} // namespace skr
SKR_TYPE_INFO(skr::ArWriteBin, "6f98f2dd-ec27-45ed-960f-c31aa854a229");
SKR_TYPE_INFO(skr::ArReadBin, "d3c6f0e1-2f3b-4e2e-8f7a-3c9e5b6a7d4c");

namespace skr
{
struct SKR_CORE_API ArWriteBin final : public ArchiveWrite
{
    // ctor & dtor
    ArWriteBin();
    ~ArWriteBin();

    //==> ArchiveWrite APIs
    // structured api
    void impl_key(StringView key) override;
    void impl_key_copy(StringView key) override;
    void impl_begin_array() override;
    void impl_end_array() override;
    void impl_begin_object() override;
    void impl_end_object() override;

    // typed data
    void impl_primitive(EArchivePrimitiveType type, const void* data) override;
    void impl_str(StringView str) override;

    // non-structured api
    void impl_bytes(const void* data, uint64_t size) override;
    void impl_bits(const void* data, uint64_t size, uint64_t offset) override;
    //==> ArchiveWrite APIs end

    // reset
    void reset();
    void reset_buffer();

    // getter
    inline const Vector<uint8_t>& buffer() const { return _buffer; }
    inline Vector<uint8_t>& buffer() { return _buffer; }

private:
    Vector<uint8_t> _buffer;
};
struct SKR_CORE_API ArReadBin final : public ArchiveRead
{
    // ctor & dtor
    ArReadBin();
    ~ArReadBin();

    //==> ArchiveRead APIs
    // structured api
    void impl_key(StringView key) override;
    bool impl_has_value() override;
    void impl_begin_array() override;
    uint64_t impl_array_size() override;
    void impl_end_array() override;
    void impl_begin_object() override;
    void impl_end_object() override;

    // typed data
    void impl_primitive(EArchivePrimitiveType type, void* data) override;
    //! view must be valid before any other archive operation, except checkpoint()
    StringView impl_str_view() override;

    // non-structured api
    void impl_bytes(void* data, uint64_t size) override;
    void impl_bits(void* data, uint64_t size, uint64_t offset) override;
    //==> ArchiveRead APIs end

    // usage
    inline bool is_complete_used() const { return _read_pos == _buffer.size(); }
    inline uint64_t read_slack() const { return _read_pos < _buffer.size(); }
    inline uint64_t read_pos() const { return _read_pos; }

    // step 1. prepare data
    void use_buffer(Span<const uint8_t> buffer);
    Span<const uint8_t> buffer() const;

    // step 2. reset read position
    void reset();
    void reset_buffer();

private:
    // helpers
    bool _move_next(uint64_t size, uint64_t& old_pos);

private:
    uint64_t _read_pos = 0;
    Span<const uint8_t> _buffer;
};
} // namespace skr
