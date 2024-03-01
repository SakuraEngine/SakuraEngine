#pragma once
#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
struct SKR_RUNTIME_API PointerType : public GenericType {
    PointerType(Type* target_type, skr::String name);

    bool query_feature(ETypeFeature feature) const override;

    void   call_ctor(void* ptr) const override;
    void   call_dtor(void* ptr) const override;
    void   call_copy(void* dst, const void* src) const override;
    void   call_move(void* dst, void* src) const override;
    void   call_assign(void* dst, const void* src) const override;
    void   call_move_assign(void* dst, void* src) const override;
    size_t call_hash(const void* ptr) const override;

    int                   write_binary(const void* dst, skr_binary_writer_t* writer) const override;
    int                   read_binary(void* dst, skr_binary_reader_t* reader) const override;
    void                  write_json(const void* dst, skr_json_writer_t* writer) const override;
    skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const override;

    inline Type* target_type() const { return _target_type; }

private:
    Type* _target_type;
};
} // namespace skr::rttr