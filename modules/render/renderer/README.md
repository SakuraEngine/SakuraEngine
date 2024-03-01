# 渲染流程

ECS 渲染器遍历场景中的相机，裁剪产生 DrawCallList（下称 DCList）并提交到渲染管线中。

- 从相机开始 (每个相机可以作为一个 ParallelRoot）遍历场景中所有的 entities；
- 预先 reserve 一个足够大的 vec<ent_t, ${all_entities}>，用于存储裁剪过后的 entities；
- 并发裁剪，并把裁剪过后的 entities emplace 到 culled_list 中；
- 并发 access culled_list 中的 entities，生成 DCList；
- 并发执行 DCList 中的 DrawCall。