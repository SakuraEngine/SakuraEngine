# CGPU
非常LowLevel的GPU计算/绘制层抽象。

## 宗旨和入门摘要

### HAL

作为复杂的跨平台HAL，CGPU拥有相当细粒度的API设计，来确保在每个后端可以尽量高效率地运行。我们加入了ResourceBarrier相关的接口，并完整保留了Multi-Device和Multi-Queue相关的功能（Multi-Adapter还未支持，需要在Resource上开洞实现Share）。

但是这并不意味着CGPU是非常难用的接口。我们基本沿用webgpu的大量设计，并且使用shader反射辅助根签名的自动创建。对GPU功能熟悉的程序员可以非常快速地上手CGPU，因为CGPU的接口基本是对GPU功能的一对一映射。

### 理念

和很多自顶向上地、对各个API进行统计总结并设计接口的gfx库不同，CGPU中的大部分概念取自硬件或平台功能。我们首先选取特定的一个GPU功能，再对不同的平台接口进行软件封装相关的考量和兼容，最终呈现到API上。也就是说，CGPU基本上是以自底向下的方向进行设计的。

一个典型的例子是CommandBuffer / CommandEncoder / CommandList相关的设计。Vulkan和D3D12在Pool/Allocator中申请Command和State内存，再映射到实际的Buffer/List中，最终交给GPU处理，而这个过程是支持混录的。而Metal则是分离了不同Scope的Command Encoder，限制用户在每个Copy / Compute / Render Dispatch中的Command Call来强制保证命令流的紧凑性。

很多自顶向上设计的API会选择直接兼容Metal的软件接口，让用户创建多个List或者多个Encoder，以此实现对Metal API接口的兼容。但是这样会造成D3D和Vulkan后端的CommandBuffer/Pool管理的困难，因为事实上零散的Encoder很难对应到单一的Allocator上，而零散的List也很难映射到单一的D3D12List / VkCmdBuffer上（Spec要求每个线程一个Allocator，且CmdList要尽量长，不能太零散），这会造成潜在的性能问题。初此之外，用户手动创建的encoder也会增大gfx对象管理的难度。

其实从硬件/驱动的功能性角度上讲，事实上这些cmd会被送达不同的执行引擎，而且每个cmd的执行需要一定的上下文，这是造成这些设计差异的根源。CGPU从实际硬件调度的角度锁定Command录取的上下文：

- RenderDispatch（RenderPass）之中：可以调用SetScissor，SetVertexBuffer，SetGfxPSO等光栅化管线相关命令；
- ComputeDispatch相对比较自由，只要寄存器状态合理（所有binder都被设置好，资源barrier正确），且不在RenderPass内部，就可以开始调度；
- TransferDispatch（Copy）是最灵活的，所有调度引擎可以在任何时候执行transfer命令，唯一例外是在RenderPass打开状态下的Gfx引擎。

可以轻易发现其实Scope锁定发生在Begin/End Dispatch上。因此CGPU会在BeginPass时返回对应的Encoder，在EndPass时收回对应的Encoder，并确保CmdBuffer的最终录制。这样可以在维持独立Allocator和Buffer的同时，兼容Metal Encoder的设计，也不用维护额外的Encoder对象。并且，由于这是贴近调度引擎最佳工作状态的流程，即使在Vulkan和D3D12上也能保证更优的性能表现。

这样的设计理念带来非凡的性能和最符合HAL程序员直觉的使用体验。事实上，CGPU后台很少进行脏数据Cache之类的DirtyWork，这可以显著降低invoke的成本、增强上游封装的自由度，也最终降低了程序维护的难度。

## 功能完成度

### 通用
- Binding Table :heavy_check_mark:
- Fence & Semaphore :heavy_check_mark:
- Resource Barriers :heavy_check_mark:
- Multi Queue :heavy_check_mark:
- mGPU :heavy_exclamation_mark:

### 计算管线
- Dispatch :heavy_check_mark:
- CBV :heavy_check_mark:
- UAV :heavy_check_mark:
- Texture :heavy_check_mark:

### 光栅化管线
- Shader Reflection :heavy_check_mark:
- MSAA :heavy_check_mark: 
- Reflection Created RootSig :heavy_check_mark:
- Texture Sample :heavy_check_mark:
- CBV :heavy_check_mark:
- Static Sampler :heavy_check_mark:
- Vertex Layout :heavy_check_mark:
- Root Constant :heavy_check_mark:
- Constant Spec :heavy_exclamation_mark:
- Shading Rate :heavy_check_mark:

### MeshPipeline

:heavy_exclamation_mark:

### 光线追踪管线

:heavy_exclamation_mark:
