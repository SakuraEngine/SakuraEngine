
#pragma once
#include "bitsery/deserializer.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/core/std_defaults.h"
#include "EASTL/vector.h"
#include "EASTL/fixed_vector.h"
#include "gsl/span"
#include "platform/guid.h"
#include "utils/te.hpp"

namespace bitsery
{

namespace traits
{

template <typename T>
struct ContainerTraits<gsl::span<T>>
    : public StdContainer<gsl::span<T>, true, true> {
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