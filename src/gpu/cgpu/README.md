# CGPU
非常LowLevel的GPU计算/绘制层抽象。

# RenderPass & PipelineState 
- RenderPass是多个或者一个PipelineState的组合
- 在PC这种没有Subpass的平台上，一个Pass对应一个PipelineState
- Subpass等行为会导致PipelineState在Pass内产生变化与切换
- 在更加LowLevel的平台上，可以以寄存器组为Scope直接操控Pipeline上的状态
    - 例如，在CommandBuffer的寄存器区内直接嵌入SetScissor等状态切换代码
    - 这使得操控粒度进一步达到了传统API的级别
- 所以vkSetPass+vkSetPipelineState/SetPSO是一种Flush的行为
- 而一些平台进一步地提供SubPass等细粒度控制管线状态的API

# API
综上我们认为：
- 直接将Pass归类为一种Flush行为, 它有自己的RootSignature/Binder，并在开始时对GPU状态进行Flush
- 提供Pass内转换状态的扩展API，此API按照它的功用使用枚举对参数集分组
- 在进行Pass切换时，将状态分组并筛排可能会获得性能提升。虽然API（如D3D12 PSO）行为设计为Flush，但没有理由不认为驱动会diff PSO并在GPU上应用最少的状态切换