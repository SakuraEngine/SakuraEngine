#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/vector/vector.hpp"

namespace skr::container
{
template <typename Memory>
struct RawString : protected Vector<Memory> {
    using BaseVector = Vector<Memory>;

    // from vector
    using typename BaseVector::DataType;
    using typename BaseVector::SizeType;
    using typename BaseVector::AllocatorCtorParam;

    // TODO. data ref
    // TODO. cursor & iterator
    // TODO. stl iterator

    // static helper

    // ctor & dtor
    RawString(AllocatorCtorParam param = {}) noexcept;
    RawString(SizeType size, AllocatorCtorParam param = {}) noexcept;
    ~RawString() noexcept;

    // copy & move
    RawString(const RawString& rhs) noexcept;
    RawString(RawString&& rhs) noexcept;

    // assign & move assign
    RawString& operator=(const RawString& rhs) noexcept;
    RawString& operator=(RawString&& rhs) noexcept;

    // compare
    // TODO. compare with view & raw string
    bool operator==(const RawString& rhs) noexcept;
    bool operator!=(const RawString& rhs) noexcept;

    // getter
    using BaseVector::size;
    using BaseVector::capacity;
    using BaseVector::slack;
    using BaseVector::empty;
    using BaseVector::data;
    using BaseVector::memory;

    // memory op
    using BaseVector::clear;
    using BaseVector::release;
    using BaseVector::reserve;
    using BaseVector::shrink;
    using BaseVector::resize;
    using BaseVector::resize_unsafe;
    using BaseVector::resize_default;
    using BaseVector::resize_zeroed;

    // add
    // TODO. code point version
    using BaseVector::add;
    using BaseVector::add_unique;
    using BaseVector::add_unsafe;
    using BaseVector::add_default;
    using BaseVector::add_zeroed;

    // add at (insert)
    // TODO. code point version
    using BaseVector::add_at;
    using BaseVector::add_at_unsafe;
    using BaseVector::add_at_default;
    using BaseVector::add_at_zeroed;

    // append
    // TODO. code point version
    using BaseVector::append;

    // append at
    // TODO. code point version
    using BaseVector::append_at;

    // remove
    // TODO. code point version
    using BaseVector::remove_at;
    using BaseVector::remove;
    using BaseVector::remove_last;
    using BaseVector::remove_all;

    // remove if
    // TODO. code point version
    using BaseVector::remove_if;
    using BaseVector::remove_last_if;
    using BaseVector::remove_all_if;

    // modify
    // TODO. code point version
    using BaseVector::operator[];
    using BaseVector::at;
    using BaseVector::last;

    // find
    // TODO. code point version
    // TODO. strstr find, and alias
    using BaseVector::find;
    using BaseVector::find_last;

    // find if
    // TODO. code point version
    using BaseVector::find_if;
    using BaseVector::find_last_if;

    // contains
    // TODO. code point version
    // TODO. strstr find
    using BaseVector::contains;
    using BaseVector::contains_if;

    // TODO. cursor & iter

    // TODO. stl-style iterator

    // TODO. erase

    // sub string

    // trim

    // replace

    // split

    // syntax
    const RawString& readonly() const;
};
} // namespace skr::container