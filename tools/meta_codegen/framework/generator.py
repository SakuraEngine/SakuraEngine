'''
parser_target: 
- record & field & method & method parameter
- enum & enum_value
- function (global)

调用：
由 xmake 给出某个具体功能的实现（.py），然后加载 .py 文件中具名的生成器。

Json 源数据处理：
1. 解析数据，同时记录覆写
2. 提取 CPP 对象，暂存 Attribute 原始数据
3. 使用 Parser 识别路径与词条，过滤掉无法认知的词条
4. 检查字面覆写
5. 处理简写，并检查覆写
6. 转换为 object，并录入

表层概念理解与 Parser 组装：
同一 key 下可以容纳多个 generator 给出的功能，只要保证不发生名称冲突即可
generator 以离散的形式给出（逐个 target 给出），联合判定（比如 field 的 rttr enable 默认值受到 record 的 rttr enable 影响）通过传入函数来实现
generator 本身通常不存储任何信息，只负责解析和向 db 写入信息

Generator 类型：
有时候在生成 CPP 文件的同时还要输出一些副产物（比如语言之间的胶水层等），需要给出一个 API 来自定义生成
'''


class Generator:
    pass
