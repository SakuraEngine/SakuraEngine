'''
所有 plugin 的 parse 结果最终都会被存入 database 中，一般来说 mako 模板的渲染只依赖于 database
plugin 只负责向 database 提供数据，并在渲染前进行检查

除了基本的类型信息外，还提供：
- module：模块名
- module_api：模块 api
- file_id：文件 id 的 marco

header_db 与 global_db：

'''

from typing import List
from typing import Dict
from record import Record
from enum import Enum
from function import Function


class TinyDatabase:
    def __init__(self) -> None:
        self.records: List[Record] = []
        self.enums: List[Enum] = []
        self.functions: List[Function] = []

        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enum] = {}


class Database:
    def __init__(self) -> None:
        # full data
        self.records: List[Record] = []
        self.enums: List[Enum] = []
        self.functions: List[Function] = []

        # per header data
