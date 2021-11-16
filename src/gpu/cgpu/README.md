# CGPU
非常LowLevel的GPU计算/绘制层抽象。

# RenderPass & PipelineState 
- RenderPass是多个或者一个PipelineState的组合
- 在PC这种没有Subpass的平台上，一个Pass对应一个PipelineState
- Subpass等行为会导致PipelineState在Pass内产生变化与切换


