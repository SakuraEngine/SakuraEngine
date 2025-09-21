#pragma once
#include "std/type_traits/member_info.hxx"
#include "type_usings.hxx" // IWYU pragma: export

#define gpu_struct(...) struct
#define gpu_table(...) struct

namespace skr::gpu
{

template <typename T> 
struct GPUDatablock;

template <typename T>
struct AOSOAInfo 
{
    inline static constexpr bool IsSOA = false;
    inline static constexpr uint32_t SOAPageSize = 16 * 1024;
};

#define SubBlock(T, M) &T::M, __builtin_offsetof(skr::gpu::GPUDatablock<T>, _##M)

template <typename T>
struct Row
{
public:
    Row() = default;
    Row(uint32_t instance, uint32_t buffer_offset = 0, uint32_t bindless_loc = ~0)
        : _instance_index(instance), _buffer_offset(buffer_offset), _bindless_index(bindless_loc)
    {

    }

    template <auto Member, uint64_t MemberOffset, typename ByteBufferType>
    auto Load(ByteBufferType buffer) const
    {
        using MemberType = typename cppsl::MemberInfo<Member>::Type;
        if constexpr (AOSOAInfo<T>::IsSOA)
        {
            constexpr auto InstanceCountInPage = AOSOAInfo<T>::SOAPageSize;
            constexpr auto PageSizeInBytes = InstanceCountInPage * GPUDatablock<T>::Size;
            const auto PageIndex = (_instance_index / InstanceCountInPage);
            const auto InstanceOffsetInPage = (_instance_index % InstanceCountInPage);
            const auto ByteAddress = PageIndex * PageSizeInBytes + MemberOffset * InstanceCountInPage + InstanceOffsetInPage * GPUDatablock<MemberType>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
        else
        {
            const auto ByteAddress = MemberOffset + _buffer_offset + _instance_index * GPUDatablock<T>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
    }

    template <typename ByteBufferType>
    T Load(ByteBufferType buffer) const
    {
        if constexpr (!AOSOAInfo<T>::IsSOA)
        {
            const auto ByteAddress = _buffer_offset + _instance_index * GPUDatablock<T>::Size;
            return buffer.template Load<GPUDatablock<T>>(ByteAddress);
        }
        else
        {
            return GPUDatablock<T>::LoadAllSOAElements(buffer, _instance_index, _buffer_offset);
        }
    }

    template <typename ByteBufferType>
    void Store(ByteBufferType buffer, const T& v) requires (!AOSOAInfo<T>::IsSOA)
    {
        const auto ByteAddress = _buffer_offset + _instance_index * GPUDatablock<T>::Size;
        buffer.Store(ByteAddress, GPUDatablock<T>(v));
    }

    template <auto Member, uint64_t MemberOffset, typename ByteBufferType>
    auto Load(ByteBufferType buffers[]) const { return Load(buffers[_bindless_index]); }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    T Load(ByteBufferType buffers[]) const { return Load(buffers[_bindless_index]); }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    void Store(ByteBufferType buffers[], const T& v) { Store(buffers[_bindless_index], v); }
    
    uint32_t BindlessIndex() const { return _bindless_index; }
    uint32_t InstanceIndex() const { return _instance_index; }
    bool IsValidBindlessBuffer() const { return _bindless_index != ~0; }

private:
    friend struct GPUDatablock<Row<T>>;
    uint32_t _instance_index = 0;
    uint32_t _buffer_offset = 0;
    uint32_t _bindless_index = ~0;
};

template <typename T, uint32_t N>
struct FixedRange
{
public:
    FixedRange() = default;
    FixedRange(uint32_t first_instance, uint32_t buffer_offset = 0, uint32_t bindless_loc = ~0)
        : _first_instance(first_instance), _buffer_offset(buffer_offset), _bindless_index(bindless_loc)
    {

    }

    template <auto Member, uint64_t MemberOffset, typename ByteBufferType>
    auto Load(ByteBufferType buffer, uint32_t instance_index) const
    {
        using MemberType = typename cppsl::MemberInfo<Member>::Type;
        if constexpr (AOSOAInfo<T>::IsSOA)
        {
            const auto _instance_index = _first_instance + instance_index;
            constexpr auto InstanceCountInPage = AOSOAInfo<T>::SOAPageSize;
            constexpr auto PageSizeInBytes = InstanceCountInPage * GPUDatablock<T>::Size;
            const auto PageIndex = (_instance_index / InstanceCountInPage);
            const auto InstanceOffsetInPage = (_instance_index % InstanceCountInPage);
            const auto ByteAddress = PageIndex * PageSizeInBytes + MemberOffset * InstanceCountInPage + InstanceOffsetInPage * GPUDatablock<MemberType>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
        else
        {
            using MemberType = typename cppsl::MemberInfo<Member>::Type;
            const auto ByteAddress = MemberOffset + _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
    }

    template <typename ByteBufferType>
    T Load(ByteBufferType buffer, uint32_t instance_index) const
    {
        if constexpr (!AOSOAInfo<T>::IsSOA)
        {
            const auto ByteAddress = _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
            return buffer.template Load<GPUDatablock<T>>(ByteAddress);
        }
        else
        {
            return GPUDatablock<T>::LoadAllSOAElements(buffer, _first_instance + instance_index, _buffer_offset);
        }
    }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    void Store(ByteBufferType buffer, uint32_t instance_index, const T& v)
    {
        const auto ByteAddress = _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
        buffer.Store(ByteAddress, GPUDatablock<T>(v));
    }

    template <auto Member, uint64_t MemberOffset, typename ByteBufferType>
    auto Load(ByteBufferType buffers[], uint32_t instance_index) const { return Load(buffers[_bindless_index], instance_index); }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    T Load(ByteBufferType buffers[], uint32_t instance_index) const { return Load(buffers[_bindless_index], instance_index); }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    void Store(ByteBufferType buffers[], uint32_t instance_index, const T& v) { return Store(buffers[_bindless_index], instance_index); }

    uint32_t BindlessIndex() const { return _bindless_index; }
    bool IsValidBindlessBuffer() const { return _bindless_index != ~0; }

private:
    friend struct GPUDatablock<FixedRange<T, N>>;
    uint32_t _first_instance = 0;
    uint32_t _buffer_offset = 0;
    uint32_t _bindless_index = ~0;
};

template <typename T>
struct Range
{
public:
    Range() = default;
    Range(uint32_t first_instance, uint32_t count = ~0, uint32_t buffer_offset = 0, uint32_t bindless_loc = ~0)
        : _first_instance(first_instance), _count(count), _buffer_offset(buffer_offset), _bindless_index(bindless_loc)
    {

    }

    template <auto Member, uint64_t MemberOffset, typename ByteBufferType>
    auto Load(ByteBufferType buffer, uint32_t instance_index) const
    {
        using MemberType = typename cppsl::MemberInfo<Member>::Type;
        if constexpr (AOSOAInfo<T>::IsSOA)
        {
            const auto _instance_index = _first_instance + instance_index;
            constexpr auto InstanceCountInPage = AOSOAInfo<T>::SOAPageSize;
            constexpr auto PageSizeInBytes = InstanceCountInPage * GPUDatablock<T>::Size;
            const auto PageIndex = (_instance_index / InstanceCountInPage);
            const auto InstanceOffsetInPage = (_instance_index % InstanceCountInPage);
            const auto ByteAddress = PageIndex * PageSizeInBytes + MemberOffset * InstanceCountInPage + InstanceOffsetInPage * GPUDatablock<MemberType>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
        else
        {
            const auto ByteAddress = MemberOffset + _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
            return (MemberType)buffer.template Load<GPUDatablock<MemberType>>(ByteAddress);
        }
    }

    template <typename ByteBufferType> requires (is_bbuffer_v<ByteBufferType>)
    T Load(ByteBufferType buffer, uint32_t instance_index) const
    {
        if constexpr (!AOSOAInfo<T>::IsSOA)
        {
            const auto ByteAddress = _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
            return buffer.template Load<GPUDatablock<T>>(ByteAddress);
        }
        else
        {
            return GPUDatablock<T>::LoadAllSOAElements(buffer, _first_instance + instance_index, _buffer_offset);
        }
    }

    T Load(TexelBuffer<T> buffer, uint32_t instance_index) const requires (!AOSOAInfo<T>::IsSOA)
    {
        return buffer.Load(_first_instance + instance_index);
    }

    template <typename ByteBufferType> requires (!AOSOAInfo<T>::IsSOA)
    void Store(ByteBufferType buffer, uint32_t instance_index, const T& v)
    {
        const auto ByteAddress = _buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size;
        buffer.Store(ByteAddress, GPUDatablock<T>(v));
    }

    template <auto Member, uint64_t MemberOffset, typename BufferType>
    auto Load(BufferType buffers[], uint32_t instance_index) const { return Load(buffers[_bindless_index], instance_index); }

    template <typename BufferType> requires (!AOSOAInfo<T>::IsSOA)
    T Load(BufferType buffers[], uint32_t instance_index) const { return Load(buffers[_bindless_index], instance_index); }

    template <typename BufferType> requires (!AOSOAInfo<T>::IsSOA)
    void Store(BufferType buffers[], uint32_t instance_index, const T& v) { return Store(buffers[_bindless_index], instance_index); }

    uint32_t Start() const { return _first_instance; }
    uint32_t Count() const { return _count; }
    uint32_t BindlessIndex() const { return _bindless_index; }
    bool IsValidBindlessBuffer() const { return _bindless_index != ~0; }

private:
    friend struct GPUDatablock<Range<T>>;
    uint32_t _first_instance = 0;
    uint32_t _count = 0;
    uint32_t _buffer_offset = 0;
    uint32_t _bindless_index = ~0;
};

// template <typename T, uint32_t Idx, uint32_t PageSize>
// struct SOAAccessor
// {
// public:
//     void operator=(const T& v)
//     {
//         GPUDatablock<T> data(v);
//     }

// private:
//     ByteAddressBuffer _buffer;
//     uint32_t _buffer_offset = 0;
// };

// template <typename T, uint32_t Idx, uint32_t PageSize>
// struct SOARow
// {
// public:
//     SOAAccessor<T, Idx, PageSize> operator[](uint32_t idx)
//     {

//     }

// private:
//     ByteAddressBuffer _buffer;
// };


} // namespace skr::gpu