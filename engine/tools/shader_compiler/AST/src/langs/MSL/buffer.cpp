namespace skr::CppSL::MSL
{
const wchar_t* kMSLBufferIntrinsics = LR"__de___l___im__(
template <typename T> struct ConstantBuffer { constant T& cgpu_buffer_data; };
template <typename T> struct PushConstant { T cgpu_buffer_data; };

template <typename T, metal::access a> struct SBuffer;
template <typename T> struct SBuffer<T, metal::access::read> 
{ 
    constant T* cgpu_buffer_data; 
    uint64_t cgpu_buffer_size; 
};
template <typename T> struct SBuffer<T, metal::access::read_write> 
{ 
    device T* cgpu_buffer_data; 
    uint64_t cgpu_buffer_size; 
};
template <typename T> using StructuredBuffer = SBuffer<T, metal::access::read>;
template <typename T> using RWStructuredBuffer = SBuffer<T, metal::access::read_write>;

template <typename T> auto buffer_read(StructuredBuffer<T> buffer, uint index) { return buffer.cgpu_buffer_data[index]; }
template <typename T> auto buffer_read(RWStructuredBuffer<T> buffer, uint index) { return buffer.cgpu_buffer_data[index]; }
template <typename T> void buffer_write(RWStructuredBuffer<T> buffer, uint index, T value) { buffer.cgpu_buffer_data[index] = value; }

template <typename T, metal::access a = metal::access::read> struct TexelBuffer;
template <typename T> struct TexelBuffer<T, metal::access::read> 
{ 
    metal::texture_buffer<typename __ScalarType<T>::Type, metal::access::read> cgpu_buffer_data;
    uint64_t cgpu_buffer_size; 
};
template <typename T> struct TexelBuffer<T, metal::access::read_write> 
{ 
    metal::texture_buffer<typename __ScalarType<T>::Type, metal::access::read_write> cgpu_buffer_data;
    uint64_t cgpu_buffer_size; 
};
template <typename T> using RWTexelBuffer = TexelBuffer<T, metal::access::read_write>;

template <typename T> auto buffer_read(TexelBuffer<T> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).x; return v; }
template <typename T> auto buffer_read(RWTexelBuffer<T> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).x; return v;}
template <typename T, typename V> void buffer_write(RWTexelBuffer<T> buffer, uint index, V value) { buffer.cgpu_buffer_data[index] = value; }

template <typename T> auto buffer_read(TexelBuffer<metal::vec<T, 2>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xy; return v; }
template <typename T> auto buffer_read(RWTexelBuffer<metal::vec<T, 2>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xy; return v;}
template <typename T, typename V> void buffer_write(RWTexelBuffer<metal::vec<T, 2>> buffer, uint index, V value) { buffer.cgpu_buffer_data[index] = value; }

template <typename T> auto buffer_read(TexelBuffer<metal::vec<T, 3>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xyz; return v; }
template <typename T> auto buffer_read(RWTexelBuffer<metal::vec<T, 3>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xyz; return v;}
template <typename T, typename V> void buffer_write(RWTexelBuffer<metal::vec<T, 3>> buffer, uint index, V value) { buffer.cgpu_buffer_data[index] = value; }

template <typename T> auto buffer_read(TexelBuffer<metal::vec<T, 4>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xyzw; return v; }
template <typename T> auto buffer_read(RWTexelBuffer<metal::vec<T, 4>> buffer, uint index) { T v = buffer.cgpu_buffer_data.read(index).xyzw; return v;}
template <typename T, typename V> void buffer_write(RWTexelBuffer<metal::vec<T, 4>> buffer, uint index, V value) { buffer.cgpu_buffer_data[index] = value; }

// ByteAddressBuffer support
struct ByteAddressBuffer { ByteAddressBuffer() = default; constant uint* cgpu_buffer_data = nullptr; uint64_t cgpu_buffer_size; };
struct RWByteAddressBuffer { RWByteAddressBuffer() = default; device uint* cgpu_buffer_data = nullptr; uint64_t cgpu_buffer_size; };

// ByteAddressBuffer read operations
template <typename T> 
T byte_buffer_read(ByteAddressBuffer b, uint byte_offset) { 
    return *reinterpret_cast<constant T*>(reinterpret_cast<constant char*>(b.cgpu_buffer_data) + byte_offset);
}

template <typename T> 
T byte_buffer_read(RWByteAddressBuffer b, uint byte_offset) { 
    return *reinterpret_cast<device T*>(reinterpret_cast<device char*>(b.cgpu_buffer_data) + byte_offset);
}

// ByteAddressBuffer write operations
template <typename T> 
void byte_buffer_write(RWByteAddressBuffer b, uint byte_offset, T value) { 
    *reinterpret_cast<device T*>(reinterpret_cast<device char*>(b.cgpu_buffer_data) + byte_offset) = value;
}

// Convenience macros for MSL compatibility
#define byte_buffer_load(b, i)  byte_buffer_read<uint>((b), i)
#define byte_buffer_load2(b, i) byte_buffer_read<uint2>((b), i)
#define byte_buffer_load3(b, i) byte_buffer_read<uint3>((b), i)
#define byte_buffer_load4(b, i) byte_buffer_read<uint4>((b), i)

#define byte_buffer_store(b, i, v)  byte_buffer_write(b, i, v)
#define byte_buffer_store2(b, i, v) byte_buffer_write(b, i, v)
#define byte_buffer_store3(b, i, v) byte_buffer_write(b, i, v)
#define byte_buffer_store4(b, i, v) byte_buffer_write(b, i, v)
)__de___l___im__";

} // namespace CppSL::MSL