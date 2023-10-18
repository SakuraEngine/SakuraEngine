#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API PointerType : public GenericType {
    PointerType(Type* target_type, string name);

    bool call_ctor(void* ptr) const override;
    bool call_dtor(void* ptr) const override;
    bool call_copy(void* dst, const void* src) const override;
    bool call_move(void* dst, void* src) const override;
    bool call_assign(void* dst, const void* src) const override;
    bool call_move_assign(void* dst, void* src) const override;
    bool call_hash(const void* ptr, size_t& result) const override;

    inline Type* target_type() const { return _target_type; }

    int                   write_binary(const void* dst, skr_binary_writer_t* writer) const override;
    int                   read_binary(void* dst, skr_binary_reader_t* reader) const override;
    void                  write_json(const void* dst, skr_json_writer_t* writer) const override;
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override;

private:
    Type* _target_type;
};
} // namespace skr::rttr