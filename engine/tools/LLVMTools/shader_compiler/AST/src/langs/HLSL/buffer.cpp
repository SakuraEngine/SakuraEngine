namespace skr::CppSL::HLSL
{
const wchar_t* kHLSLBufferIntrinsics = LR"__de___l___im__(
template <typename T> void buffer_write(RWStructuredBuffer<T> buffer, uint index, T value) { buffer[index] = value; }
template <typename T> void buffer_write(RWTexelBuffer<T> buffer, uint index, T value) { buffer[index] = value; }
template <typename T> T buffer_read(RWStructuredBuffer<T> buffer, uint index) { return buffer[index]; }
template <typename T> T buffer_read(StructuredBuffer<T> buffer, uint index) { return buffer[index]; }
template <typename T> T buffer_read(RWTexelBuffer<T> buffer, uint index) { return buffer[index]; }
template <typename T> T buffer_read(TexelBuffer<T> buffer, uint index) { return buffer[index]; }

#define byte_buffer_load(b, i)  (b).Load((i))
#define byte_buffer_load2(b, i) (b).Load2((i))
#define byte_buffer_load3(b, i) (b).Load3((i))
#define byte_buffer_load4(b, i) (b).Load4((i))

#define byte_buffer_store(b, i, v)  (b).Store((i), (v))
#define byte_buffer_store2(b, i, v) (b).Store2((i), (v))
#define byte_buffer_store3(b, i, v) (b).Store3((i), (v))
#define byte_buffer_store4(b, i, v) (b).Store4((i), (v))

template <typename T> T byte_buffer_read(ByteAddressBuffer b, uint i) { return b.Load<T>(i); }
template <typename T> T byte_buffer_read(RWByteAddressBuffer b, uint i) { return b.Load<T>(i); }
template <typename T> void byte_buffer_write(RWByteAddressBuffer b, uint i, T v) { b.Store<T>(i, v); }
)__de___l___im__";

} // namespace CppSL::HLSL