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

## 寄存器組
- ShRegisters：Shader状态，对应VkPipelineShaderStageCreateInfo
- CxRegisters: 调度管线状态
- UcRegisters：

## API
综上我们认为：
- 直接将Pass归类为一种Flush行为, 它有自己的RootSignature/Binder，并在开始时对GPU状态进行Flush
- 提供Pass内转换状态的扩展API，此API按照它的功用使用枚举对参数集分组
- 在进行Pass切换时，将状态分组并筛排可能会获得性能提升。虽然API（如D3D12 PSO）行为设计为Flush，但没有理由不认为驱动会diff PSO并在GPU上应用最少的状态切换

# Shader反射和管线反射
由于每个API的管线绑定部分天差地别，这部分我们必须借助运行时反射来辅助创建这些对象。

## 关于推送常量
- 推常量使用非常方便，对uber shader的多分支很有用
- NVIDIA硬件下像素着色器访问根常量非常快，而AMD也在GPUOpen表示RDNA架构下改变绘制的常量可以放在根常量下
暂时不对推送常量进行支持，但是方法如下：
- D3D12实际上要使用RootConstant来进行内联绑定，常量值实际上是在寄存器组bx上的，在shader中它和常规的cbuffer无二致
- VK的推送常量只需要着色器中写明attribute [[vk::push_constant]]
- 我认为着色器中应当采取vulkan的写法，在编译shader的时候对HLSL文件进行修补，自动生成寄存器bx，来弥补D3D12的失败设计
- 退而求其次的方案是使用宏：ROOT_CONSTANT(type, variable, register)