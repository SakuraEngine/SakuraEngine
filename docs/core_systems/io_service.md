# SakuraEngine I/O 服务系统

## 概述

SakuraEngine 的 I/O 服务系统提供了高性能的异步文件访问能力，支持传统文件系统、DirectStorage 硬件加速以及直接到 GPU 内存的传输。系统采用组件化设计，支持批处理、压缩、优先级调度等高级特性。

## 核心概念

### 系统架构

```
┌──────────────────────────────────────────────────────┐
│                    I/O Service                        │
├──────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ Request Pool │  │   Resolver  │  │   Reader    │ │
│  └──────────────┘  └─────────────┘  └─────────────┘ │
│                           │                 │         │
│  ┌──────────────────────────────────────────┐       │
│  │            Processing Pipeline            │       │
│  │  ENQUEUED → RESOLVING → LOADING → DONE   │       │
│  └──────────────────────────────────────────┘       │
├──────────────────────────────────────────────────────┤
│         Platform Layer (VFS/DirectStorage)            │
└──────────────────────────────────────────────────────┘
```

### 核心组件

#### I/O 服务接口

```cpp
// 基础 I/O 服务接口
struct IIOService {
    // 生命周期管理
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void drain() = 0;
    
    // 请求管理
    virtual IIORequest* open_request() = 0;
    virtual void request(IIORequest* request) = 0;
    virtual void cancel(IIORequest* request) = 0;
};

// RAM I/O 服务（系统内存）
struct IRAMService : public IIOService {
    virtual IBlocksRAMRequest* open_ram_request() = 0;
};

// VRAM I/O 服务（GPU 内存）
struct IVRAMService : public IRAMService {
    virtual ISlicesVRAMRequest* open_texture_request() = 0;
    virtual IBlocksVRAMRequest* open_buffer_request() = 0;
};
```

#### I/O 请求组件

```cpp
// 请求的组件化设计
struct IIORequest {
    // 基础组件
    PathSrcComponent* get_path_component();
    IOStatusComponent* get_status_component();
    
    // 数据组件
    BlocksComponent* get_blocks_component();
    CompressedBlocksComponent* get_compressed_component();
};
```

## 基本使用

### 创建和配置 I/O 服务

```cpp
// 创建 RAM I/O 服务
skr_ram_io_service_desc_t desc = {
    .name = u8"MainIOService",
    .sleep_on_await = true,
    .runner_affinity = 0xFF,  // CPU 亲和性掩码
    .allow_direct_storage = true
};
auto io_service = skr_io_ram_service_t::create(&desc);

// 启动服务
io_service->run();

// 创建 VRAM I/O 服务（用于 GPU 资源）
skr_vram_io_service_desc_t vram_desc = {
    .name = u8"GPUIOService",
    .ram_service = io_service,
    .device = gpu_device,
    .upload_queue = transfer_queue
};
auto vram_service = skr_io_vram_service_t::create(&vram_desc);
```

### 异步文件读取

```cpp
// 简单的异步读取
void read_file_async(const char8_t* path) {
    // 创建请求
    auto request = io_service->open_ram_request();
    
    // 设置路径
    request->set_path(path);
    
    // 设置回调
    request->set_callback(
        SKR_IO_STAGE_COMPLETED,
        [](skr_io_request_t* request, void* data) {
            auto buffer = request->get_buffer();
            printf("File loaded: %zu bytes\n", buffer->get_size());
            
            // 处理数据...
            process_data(buffer->get_data(), buffer->get_size());
            
            // 释放请求
            request->release();
        },
        nullptr  // userdata
    );
    
    // 提交请求
    io_service->request(request);
}
```

### 批量 I/O 操作

```cpp
// 批量读取多个文件
void batch_read_files(const std::vector<const char8_t*>& paths) {
    // 创建批次
    auto batch = io_service->create_batch();
    
    for (auto& path : paths) {
        auto request = io_service->open_ram_request();
        request->set_path(path);
        
        // 添加到批次
        batch->add_request(request);
    }
    
    // 设置批次完成回调
    batch->set_callback([](IIOBatch* batch, void* data) {
        printf("Batch completed: %u files\n", batch->get_count());
        
        // 处理所有文件
        for (uint32_t i = 0; i < batch->get_count(); ++i) {
            auto request = batch->get_request(i);
            process_file(request);
        }
        
        batch->release();
    });
    
    // 提交批次
    io_service->request_batch(batch);
}
```

### 压缩文件支持

```cpp
// 读取压缩文件
void read_compressed_file(const char8_t* path) {
    auto request = io_service->open_ram_request();
    
    // 设置压缩文件路径
    request->set_path(path);
    
    // 配置解压
    auto compressed = request->get_compressed_component();
    compressed->set_enable(true);
    compressed->set_format(SKR_IO_COMPRESSION_LZ4);
    
    request->set_callback(
        SKR_IO_STAGE_DECOMPRESSED,
        [](skr_io_request_t* request, void* data) {
            // 获取解压后的数据
            auto buffer = request->get_decompressed_buffer();
            printf("Decompressed size: %zu bytes\n", buffer->get_size());
        },
        nullptr
    );
    
    io_service->request(request);
}
```

## 高级特性

### DirectStorage 集成

```cpp
// 检查 DirectStorage 可用性
auto availability = skr_query_dstorage_availability();

if (availability == SKR_DSTORAGE_AVAILABILITY_HARDWARE) {
    printf("Hardware DirectStorage available!\n");
    
    // DirectStorage 会自动启用
    // 无需额外配置
} else if (availability == SKR_DSTORAGE_AVAILABILITY_SOFTWARE) {
    printf("Software DirectStorage fallback\n");
}

// 使用 DirectStorage 优化的读取
void read_with_dstorage(const char8_t* path) {
    auto request = io_service->open_ram_request();
    
    // 设置较大的缓冲区以利用 DirectStorage
    request->set_buffer_size(16 * 1024 * 1024);  // 16MB
    
    // 设置高优先级
    request->set_priority(SKR_ASYNC_SERVICE_PRIORITY_HIGH);
    
    request->set_path(path);
    io_service->request(request);
}
```

### VRAM 直接加载

```cpp
// 直接加载纹理到 GPU
void load_texture_to_gpu(const char8_t* path, CGPUDeviceId device) {
    auto request = vram_service->open_texture_request();
    
    // 设置纹理描述
    CGPUTextureDescriptor tex_desc = {
        .width = 2048,
        .height = 2048,
        .format = CGPU_FORMAT_BC7_UNORM,
        .mip_levels = 1
    };
    request->set_texture_desc(&tex_desc);
    
    // 设置文件路径
    request->set_path(path);
    
    // 设置 GPU 传输完成回调
    request->set_callback(
        SKR_IO_STAGE_VRAM_UPLOADED,
        [](skr_io_request_t* request, void* data) {
            auto texture = request->get_texture();
            printf("Texture uploaded to GPU: %p\n", texture);
            
            // 纹理现在可以直接使用
            bind_texture(texture);
        },
        nullptr
    );
    
    vram_service->request(request);
}

// 流式加载纹理 mipmap
void stream_texture_mips(CGPUTextureId texture) {
    auto request = vram_service->open_slice_request();
    
    // 设置目标纹理
    request->set_destination_texture(texture);
    
    // 加载高精度 mip levels
    for (uint32_t mip = 0; mip < 5; ++mip) {
        request->add_slice(
            mip,     // mip level
            0,       // array layer
            0, 0,    // offset
            width >> mip, height >> mip  // size
        );
    }
    
    request->set_path(u8"textures/high_res_mips.dds");
    vram_service->request(request);
}
```

### 优先级和取消

```cpp
// 优先级管理
class PriorityIOManager {
    skr_io_ram_service_t* service;
    std::vector<skr_io_request_t*> pending_requests;
    
public:
    void load_critical_resource(const char8_t* path) {
        auto request = service->open_ram_request();
        request->set_path(path);
        request->set_priority(SKR_ASYNC_SERVICE_PRIORITY_HIGH);
        
        // 取消低优先级请求为关键资源让路
        cancel_low_priority_requests();
        
        service->request(request);
    }
    
    void cancel_low_priority_requests() {
        for (auto req : pending_requests) {
            if (req->get_priority() == SKR_ASYNC_SERVICE_PRIORITY_LOW) {
                service->cancel(req);
            }
        }
    }
};
```

### 自定义处理器

```cpp
// 实现自定义解析器
class CustomResolver : public IIOResolver {
public:
    void resolve(IIORequest* request) override {
        // 自定义路径解析
        auto path = request->get_path();
        
        // 例如：处理虚拟文件系统
        if (strncmp(path, "vfs://", 6) == 0) {
            resolve_vfs_path(request, path + 6);
        } else {
            // 默认处理
            default_resolver->resolve(request);
        }
    }
};

// 实现自定义读取器
class EncryptedReader : public IIOReader {
public:
    void read(IIORequest* request) override {
        // 读取加密文件
        auto buffer = request->get_buffer();
        
        // 执行读取
        read_encrypted_file(
            request->get_file_handle(),
            buffer->get_data(),
            buffer->get_size()
        );
        
        // 解密数据
        decrypt_buffer(buffer->get_data(), buffer->get_size());
    }
};

// 注册自定义处理器
io_service->add_resolver(std::make_unique<CustomResolver>());
io_service->add_reader(std::make_unique<EncryptedReader>());
```

## 性能优化

### 缓冲区管理

```cpp
// 预分配缓冲区池
class IOBufferPool {
    skr::ConcurrentQueue<skr_io_buffer_t*> small_buffers;   // < 1MB
    skr::ConcurrentQueue<skr_io_buffer_t*> medium_buffers;  // 1-16MB
    skr::ConcurrentQueue<skr_io_buffer_t*> large_buffers;   // > 16MB
    
public:
    skr_io_buffer_t* acquire(size_t size) {
        skr_io_buffer_t* buffer = nullptr;
        
        if (size < 1024 * 1024) {
            small_buffers.try_dequeue(buffer);
        } else if (size < 16 * 1024 * 1024) {
            medium_buffers.try_dequeue(buffer);
        } else {
            large_buffers.try_dequeue(buffer);
        }
        
        if (!buffer) {
            buffer = skr_io_buffer_t::create(size);
        }
        
        return buffer;
    }
    
    void release(skr_io_buffer_t* buffer) {
        buffer->reset();
        
        size_t size = buffer->get_capacity();
        if (size < 1024 * 1024) {
            small_buffers.enqueue(buffer);
        } else if (size < 16 * 1024 * 1024) {
            medium_buffers.enqueue(buffer);
        } else {
            large_buffers.enqueue(buffer);
        }
    }
};
```

### 请求合并

```cpp
// 合并小文件请求
class IORequestMerger {
    struct MergedRequest {
        std::vector<skr_io_request_t*> requests;
        size_t total_size;
        char8_t* merged_path;
    };
    
    void merge_adjacent_requests(std::vector<skr_io_request_t*>& requests) {
        // 按文件位置排序
        std::sort(requests.begin(), requests.end(), 
            [](auto a, auto b) {
                return a->get_offset() < b->get_offset();
            });
        
        // 合并相邻请求
        MergedRequest merged;
        for (auto req : requests) {
            if (can_merge(merged, req)) {
                merged.requests.push_back(req);
                merged.total_size += req->get_size();
            } else {
                // 提交合并的请求
                submit_merged(merged);
                merged = MergedRequest{};
            }
        }
    }
};
```

### 预读取策略

```cpp
// 智能预读取
class PredictiveLoader {
    skr_io_ram_service_t* service;
    std::unordered_map<std::string, std::vector<std::string>> dependencies;
    
public:
    void load_with_prediction(const char8_t* primary_resource) {
        // 加载主资源
        load_resource(primary_resource, SKR_ASYNC_SERVICE_PRIORITY_HIGH);
        
        // 预测并预加载相关资源
        auto deps = dependencies[primary_resource];
        for (auto& dep : deps) {
            load_resource(dep.c_str(), SKR_ASYNC_SERVICE_PRIORITY_LOW);
        }
    }
    
    void update_predictions(const char8_t* resource, 
                          const std::vector<std::string>& used_resources) {
        // 更新依赖关系图
        dependencies[resource] = used_resources;
    }
};
```

## 最佳实践

### DO - 推荐做法

1. **使用异步 I/O**
   ```cpp
   // 好：异步加载，不阻塞
   request->set_callback(on_loaded);
   service->request(request);
   
   // 避免：同步等待
   service->request_and_wait(request);
   ```

2. **批量处理请求**
   ```cpp
   // 批量提交相关资源
   auto batch = service->create_batch();
   add_related_resources(batch);
   service->request_batch(batch);
   ```

3. **合理设置优先级**
   ```cpp
   // 关键路径资源
   request->set_priority(SKR_ASYNC_SERVICE_PRIORITY_HIGH);
   
   // 预加载资源
   request->set_priority(SKR_ASYNC_SERVICE_PRIORITY_LOW);
   ```

### DON'T - 避免做法

1. **避免小文件频繁 I/O**
   ```cpp
   // 不好：大量小文件
   for (auto& file : thousand_small_files) {
       load_file(file);
   }
   
   // 好：打包或合并
   load_file("packed_assets.dat");
   ```

2. **不要忽略错误处理**
   ```cpp
   request->set_callback(
       SKR_IO_STAGE_COMPLETED,
       [](auto req, auto data) {
           if (req->get_status() != SKR_IO_STATUS_OK) {
               handle_error(req->get_error());
           }
       }
   );
   ```

## 调试和监控

### 性能统计

```cpp
// 获取 I/O 统计信息
void print_io_stats(skr_io_ram_service_t* service) {
    auto stats = service->get_statistics();
    
    printf("=== I/O Statistics ===\n");
    printf("Total requests: %u\n", stats.total_requests);
    printf("Pending requests: %u\n", stats.pending_requests);
    printf("Bytes read: %.2f MB\n", stats.bytes_read / 1048576.0);
    printf("Average latency: %.2f ms\n", stats.avg_latency_ms);
    
    // DirectStorage 统计
    if (stats.dstorage_enabled) {
        printf("DirectStorage hits: %u (%.1f%%)\n", 
            stats.dstorage_requests,
            100.0 * stats.dstorage_requests / stats.total_requests);
    }
}
```

### 请求追踪

```cpp
// 启用请求追踪
service->set_trace_callback(
    [](const skr_io_trace_event_t* event) {
        printf("[%s] %s: %s (%.2fms)\n",
            event->timestamp,
            event->request_id,
            event->stage_name,
            event->duration_ms);
    }
);

// 自定义标记
request->set_debug_name(u8"MainMenu_Background");
```

## 总结

SakuraEngine 的 I/O 服务系统提供了现代游戏引擎所需的高性能异步 I/O 能力。通过支持 DirectStorage、VRAM 直接传输、压缩和批处理等特性，系统能够充分利用现代硬件的能力。组件化的设计使得系统易于扩展，而完善的优先级和取消机制确保了良好的响应性。合理使用 I/O 服务可以显著提升游戏的加载性能和运行时资源管理效率。