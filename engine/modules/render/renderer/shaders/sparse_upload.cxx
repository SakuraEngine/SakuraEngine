#include <std/std.hxx>

// Upload operation descriptor (must match C++ side)
struct Upload
{
    uint32 src_offset; // Offset in upload_buffer (bytes)
    uint32 dst_offset; // Offset in target buffer (bytes)
    uint32 data_size;  // Data size (bytes)
};

ByteAddressBuffer upload_buffer;   // Source data buffer
ByteAddressBuffer upload_operations; // Copy operations list
RWByteAddressBuffer target_buffer;   // Target buffer (Core Data)

// Push constants
struct SparseUploadConstants
{
    uint num_operations;     // Number of operations
};

[[push_constant]]
ConstantBuffer<SparseUploadConstants> constants;

// Multi-threaded sparse upload - simplified but maintains parallelism
[[compute_shader("sparse_upload"), numthreads(256, 1, 1)]]
void sparse_upload_main([[sv_thread_id]] uint3 thread_id)
{
    uint operation_index = thread_id.x;
    if (operation_index >= constants.num_operations)
    {
        return;
    }

    // Load the operation handled by this thread
    Upload op = upload_operations.Load<Upload>(operation_index * sizeof(Upload));

    uint src = op.src_offset;
    uint dst = op.dst_offset;
    uint remaining = op.data_size;

    // Copy 16-byte blocks
    while (remaining >= 16)
    {
        uint4 v16 = upload_buffer.Load4(src);
        target_buffer.Store4(dst, v16);
        src += 16; dst += 16; remaining -= 16;
    }

    // Copy 8-byte block if present
    if (remaining >= 8)
    {
        uint2 v8 = upload_buffer.Load2(src);
        target_buffer.Store2(dst, v8);
        src += 8; dst += 8; remaining -= 8;
    }

    // Copy 4-byte block if present
    if (remaining >= 4)
    {
        uint v4 = upload_buffer.Load(src);
        target_buffer.Store(dst, v4);
        src += 4; dst += 4; remaining -= 4;
    }

    // Copy tail 1..3 bytes if present, via read-modify-write on destination word
    if (remaining > 0)
    {
        // Read source and destination 32-bit words (assumes 4-byte alignment of offsets)
        uint src_word = upload_buffer.Load(src);
        uint dst_word = target_buffer.Load(dst);

        // Create mask for lower 'remaining' bytes
        uint mask = (remaining == 1) ? 0x000000FFu : (remaining == 2) ? 0x0000FFFFu : 0x00FFFFFFu;
        uint new_word = (dst_word & ~mask) | (src_word & mask);
        target_buffer.Store(dst, new_word);
    }
}