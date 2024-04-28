#pragma once
#include "SkrRTTR/type/generic_type.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/span.hpp"

namespace skr::rttr
{
struct SKR_CORE_API VectorType : public GenericType {
    VectorType(Type* target_type, span<size_t> dimensions, skr::String name);

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

private:
    Type*          _target_type;
    size_t         _size;
    Vector<size_t> _dimensions;
};
} // namespace skr::rttr