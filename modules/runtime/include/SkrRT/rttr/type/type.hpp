#pragma once
#include <cstdint>
#include "SkrRT/rttr/guid.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/serde/json/writer_fwd.h"
#include "SkrRT/serde/json/reader_fwd.h"

namespace skr::rttr
{
enum ETypeCategory
{
    SKR_TYPE_CATEGORY_INVALID,

    SKR_TYPE_CATEGORY_PRIMITIVE,
    SKR_TYPE_CATEGORY_ENUM,
    SKR_TYPE_CATEGORY_RECORD,
    SKR_TYPE_CATEGORY_GENERIC,
};

struct SKR_RUNTIME_API Type {
    Type(ETypeCategory type_category, string name, GUID type_id, size_t size, size_t alignment);
    virtual ~Type() = default;

    // getter
    SKR_INLINE ETypeCategory type_category() const { return _type_category; }
    SKR_INLINE const string& name() const { return _name; }
    SKR_INLINE GUID          type_id() const { return _type_id; }
    SKR_INLINE size_t        size() const { return _size; }
    SKR_INLINE size_t        alignment() const { return _alignment; }

    // call functions
    virtual bool call_ctor(void* ptr) const                       = 0;
    virtual bool call_dtor(void* ptr) const                       = 0;
    virtual bool call_copy(void* dst, const void* src) const      = 0;
    virtual bool call_move(void* dst, void* src) const            = 0;
    virtual bool call_assign(void* dst, const void* src) const    = 0;
    virtual bool call_move_assign(void* dst, void* src) const     = 0;
    virtual bool call_hash(const void* ptr, size_t& result) const = 0;

    // serialize
    virtual int                   write_binary(const void* dst, skr_binary_writer_t* writer) const = 0;
    virtual int                   read_binary(void* dst, skr_binary_reader_t* reader) const        = 0;
    virtual void                  write_json(const void* dst, skr_json_writer_t* writer) const     = 0;
    virtual skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const          = 0;

    // TODO. convert

private:
    // basic data
    ETypeCategory _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INVALID;
    string        _name          = {};
    GUID          _type_id       = {};
    size_t        _size          = 0;
    size_t        _alignment     = 0;
};
} // namespace skr::rttr