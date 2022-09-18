
#pragma once
#include "bitsery/brief_syntax.h"
#include "bitsery/deserializer.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/core/std_defaults.h"
#include "EASTL/vector.h"
#include "EASTL/string.h"
#include "EASTL/fixed_vector.h"
#include "gsl/span"
#include "platform/guid.h"
#include "platform/memory.h"
#include "utils/types.h"
#include "utils/te.hpp"
#include "resource/resource_handle.h"
#include <limits>
#include <memory>

namespace bitsery
{

namespace traits
{

template <typename T>
struct ContainerTraits<gsl::span<T>>
    : public StdContainer<gsl::span<T>, true, true> {
};

template <typename T>
struct ContainerTraits<eastl::basic_string<T>>
    : public StdContainer<eastl::basic_string<T>, true, true> {
};

template <typename T>
struct BufferAdapterTraits<eastl::basic_string<T>>
    : public StdContainerForBufferAdapter<eastl::basic_string<T>> {
};

template <typename T, typename Allocator>
struct ContainerTraits<eastl::vector<T, Allocator>>
    : public StdContainer<eastl::vector<T, Allocator>, true, true> {
};

template <typename T, typename Allocator>
struct BufferAdapterTraits<eastl::vector<T, Allocator>>
    : public StdContainerForBufferAdapter<eastl::vector<T, Allocator>> {
};

template <typename T, size_t count>
struct ContainerTraits<eastl::fixed_vector<T, count>>
    : public StdContainer<eastl::fixed_vector<T, count>, true, true> {
};

template <typename T, size_t count>
struct BufferAdapterTraits<eastl::fixed_vector<T, count>>
    : public StdContainerForBufferAdapter<eastl::fixed_vector<T, count>> {
};

template <typename T, size_t count>
struct ContainerTraits<gsl::span<T, count>>
    : public StdContainer<gsl::span<T, count>, false, true> {
    static std::enable_if_t<count == gsl::dynamic_extent, void>
    resize(gsl::span<T, count>& container, size_t size)
    {
        SKR_ASSERT(container.data() == nullptr);
        auto data = (T*)sakura_malloc(sizeof(T) * size);
        std::uninitialized_default_construct(data, data + size);
        container = { data, size };
    }
};

template <typename T, size_t count>
struct BufferAdapterTraits<gsl::span<T, count>> {
    using TIterator = typename gsl::span<T, count>::iterator;
    using TConstIterator = typename gsl::span<T, count>::iterator;
    using TValue = typename ContainerTraits<gsl::span<T, count>>::TValue;
};

} // namespace traits

} // namespace bitsery

namespace bitsery
{
template <class S>
void serialize(S& s, skr_guid_t& guid)
{
    s.value4b(guid.Data1);
    s.value2b(guid.Data2);
    s.value2b(guid.Data3);
    for (int i = 0; i < 8; ++i)
        s.value1b(guid.Data4[i]);
}
template <class S>
void serialize(S& s, skr_resource_handle_t& handle)
{
    if (s.is_serialize())
    {
        auto guid = handle.get_serialized();
        serialize(s, guid);
    }
    else
    {
        skr_guid_t guid;
        serialize(s, guid);
        handle.set_guid(guid);
    }
}
template <class S>
void serialize(S& s, const eastl::string& str)
{
    s.container(str, eastl::string::kMaxSize);
}
template <class S, class T, class Size>
void serializeBin(S& s, T*& pointer, Size& size, Size limit = std::numeric_limits<Size>::max())
{
    gsl::span<T> span{ pointer, (size_t)size };
    s.container(span, (size_t)limit);
    pointer = span.data();
    size = (Size)span.size();
}
template <class S>
void serialize(S& s, skr_blob_t& blob)
{
    serializeBin(s, blob.bytes, blob.size);
}
template <class S>
void serialize(S& s, skr_float3_t& f3)
{
    s.value4b(f3.x);
    s.value4b(f3.y);
    s.value4b(f3.z);
}
template <class S>
void serialize(S& s, skr_float4_t& f4)
{
    s.value4b(f4.x);
    s.value4b(f4.y);
    s.value4b(f4.z);
    s.value4b(f4.w);
}
} // namespace bitsery

namespace bitsery
{
struct OutputArchive : public boost::te::poly<OutputArchive>, public details::OutputAdapterBaseCRTP<OutputArchive> {
    using TValue = uint8_t;
    void writeInternalBuffer(const TValue* data, size_t size)
    {
        boost::te::call([](auto const& self, const TValue* data, size_t size) { self.writeInternalBuffer(data, size); }, *this, data, size);
    }
    void flush()
    {
        boost::te::call([](auto const& self) { self.flush(); }, *this);
    }
};
struct InputArchive : public boost::te::poly<InputArchive>, public details::OutputAdapterBaseCRTP<InputArchive> {
    using TValue = uint8_t;
    void readInternalBuffer(TValue* data, size_t size)
    {
        boost::te::call([](auto const& self, TValue* data, size_t size) { self.readInternalBuffer(data, size); }, *this, data, size);
    }

    ReaderError error() const
    {
        return boost::te::call<ReaderError>([](auto const& self) { return self.error(); }, *this);
    }

    bool isCompletedSuccessfully() const
    {
        return boost::te::call<bool>([](auto const& self) { return self.isCompletedSuccessfully(); }, *this);
    }

    void error(ReaderError error)
    {
        boost::te::call([](auto const& self, ReaderError error) { self.error(error); }, *this, error);
    }

    void currentReadPos(size_t size)
    {
        boost::te::call([](auto const& self, size_t size) { self.currentReadPos(size); }, *this, size);
    }

    size_t currentReadPos() const
    {
        return boost::te::call<size_t>([](auto const& self) { return self.currentReadPos(); }, *this);
    }

    void currentReadEndPos(size_t size)
    {
        boost::te::call([](auto const& self, size_t size) { self.currentReadPos(size); }, *this, size);
    }

    size_t currentReadEndPos() const
    {
        return boost::te::call<size_t>([](auto const& self) { return self.currentReadPos(); }, *this);
    }
};
} // namespace bitsery