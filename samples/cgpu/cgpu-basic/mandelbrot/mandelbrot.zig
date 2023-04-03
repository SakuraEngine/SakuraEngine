const std = @import("std");
const cgpu = @import("./cgpu.zig");

pub fn main() !void {
    const instanceDesc = cgpu.CGPUInstanceDescriptor {
        .chained = null,
        .backend = cgpu.CGPU_BACKEND_D3D12,
        .enable_debug_layer = true,
        .enable_gpu_based_validation = true,
        .enable_set_name = true,
    };
    const id = cgpu.cgpu_create_instance(&instanceDesc);
    defer cgpu.cgpu_free_instance(id);
    
    const stdout = std.io.getStdOut().writer();
    try stdout.print("Hello, {s}!\n", .{"world"});
}