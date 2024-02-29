'''
parser_target: 
- record & field & method & method parameter
- enum & enum_value
- function (global)

调用：
由 xmake 给出某个具体功能的实现（.py），然后加载 .py 文件中具名的生成器。

Json 源数据处理：
1. 通过 list 的形式加载（允许重复），同时带有以下数据
    - visited：记录该条目是否被访问
    - assigned：记录该条目是否被赋过值（还是默认值）
    - override_history：记录该条目的覆写历史信息
2. 清洗 attrs 下的数据，解析覆写和路径简写，覆写通常只针对末端的字段生效   【！在这一步处理非法覆写】
3. 处理数组式简写（应当给出接收一个 Dict 的主动覆写 API）                 【！在这一步处理数组式简写的错误】
4. 交给 generator 进行解析，从 json 数据转换为 object                    【！在这一步处理未被访问到的条目，并进行 warning】
5. 将解析后的 generator-data 交给 generator 进行 codegen

表层概念理解与 Parser 组装：
同一 key 下可以容纳多个 generator 给出的功能，只要保证不发生名称冲突即可
generator 以离散的形式给出（逐个 target 给出），联合判定（比如 field 的 rttr enable 默认值受到 record 的 rttr enable 影响）通过传入函数来实现
generator 本身通常不存储任何信息，只负责解析和向 db 写入信息

Generator 类型：
有时候在生成 CPP 文件的同时还要输出一些副产物（比如语言之间的胶水层等），需要给出一个 API 来自定义生成
'''
