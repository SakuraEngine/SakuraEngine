#pragma once
#include "type_usings.hpp" // IWYU pragma: export
#include "SkrRenderer/graphics/gpu_table.hpp"

#define gpu_struct(...) struct
#define gpu_table(...) struct

namespace skr::gpu
{
    
template <typename T>
struct AOSOAInfo 
{
    inline static constexpr bool IsSOA = false;
    inline static constexpr uint32_t SOAPageSize = 16 * 1024;
};

template <typename T>
struct Row
{
public:
    Row() = default;
    Row(uint32_t instance_index, uint32_t buffer_offset = 0, uint32_t bindless_index = ~0)
        : _instance_index(instance_index), _buffer_offset(buffer_offset), _bindless_index(bindless_index)
    {

    }
    
    void Store(TableInstance& buffer, const T& v)
    {
        _bindless_index = buffer.bindless_index();
        buffer.Store(_buffer_offset + _instance_index * GPUDatablock<T>::Size, GPUDatablock<T>(v));
    }
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
    FixedRange(uint32_t first_instance, uint32_t buffer_offset = 0, uint32_t bindless_index = ~0)
        : _first_instance(first_instance), _buffer_offset(buffer_offset), _bindless_index(bindless_index)
    {

    }

    void Store(TableInstance& buffer, uint32_t instance_index, const T& v)
    {
        _bindless_index = buffer.bindless_index();
        buffer.Store(_buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size, GPUDatablock<T>(v));
    }
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
    Range(uint32_t first_instance, uint32_t count, uint32_t buffer_offset = 0, uint32_t bindless_index = ~0)
        : _first_instance(first_instance), _count(count), _buffer_offset(buffer_offset), _bindless_index(bindless_index)
    {

    }

    void Store(TableInstance& buffer, uint32_t instance_index, const T& v)
    {
        _bindless_index = buffer.bindless_index();
        buffer.Store(_buffer_offset + (_first_instance + instance_index) * GPUDatablock<T>::Size, GPUDatablock<T>(v));
    }

private:
    friend struct GPUDatablock<Range<T>>;
    uint32_t _first_instance = 0;
    uint32_t _count = 0;
    uint32_t _buffer_offset = 0;
    uint32_t _bindless_index = ~0;
};

} // namespace skr::gpu