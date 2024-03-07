'''
所有 plugin 的 parse 结果最终都会被存入 database 中，一般来说 mako 模板的渲染只依赖于 database
plugin 只负责向 database 提供数据，并在渲染前进行检查

除了基本的类型信息外，还提供：
- module：模块名
- module_api：模块 api
- file_id：文件 id 的 marco

header_db 与 global_db：

'''

import os
import os.path
from typing import List, Dict
import itertools
import json
from framework.json_parser import JsonValue, parse_json_data, error_unrecognized
from framework.cpp_types import Record, Enumeration, Function
from framework.error_tracker import ErrorTracker
from framework.attr_parser import FunctionalManager, FunctionalTarget, FunctionalParser


class HeaderDatabase:
    def __init__(self) -> None:
        # header file path
        self.header_path: str = ""

        # all data
        self.records: List[Record] = []
        self.enums: List[Enumeration] = []
        self.functions: List[Function] = []

        # name to data
        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enumeration] = {}

    def load_header(self, path: str):
        self.header_path = os.path.normpath(path).replace(os.sep, "/")

        # load raw data
        raw_json_data: Dict[str, JsonValue]
        with open(path, encoding="utf-8") as f:
            raw_json_data = json.load(f, object_pairs_hook=parse_json_data)

        # extract cpp types
        self.__extract_cpp_types(raw_json_data)

    def attr_check_structure(self, error_tracker: ErrorTracker, functional_manager: FunctionalManager):
        def each(error_tracker: ErrorTracker, target: FunctionalTarget, raw_attrs:  Dict[str, JsonValue]):
            for k, v in raw_attrs.items():
                error_tracker.push_path(k)
                error_unrecognized(v, error_tracker)
                error_tracker.pop_path()
        self.__each_attrs_and_apply_functional(error_tracker, each)
        error_tracker.print_as_warning()

    def attr_expand_path(self):
        pass

    def attr_expand_shorthand(self):
        pass

    def attr_check_override(self):
        pass

    def attr_to_object(self):
        pass

    def __each_attrs_and_apply_functional(self, error_tracker: ErrorTracker, func):
        # record
        for record in self.records:
            error_tracker.set_source(record.file_name, record.line)
            error_tracker.push_path(f"[{record.name}]")

            # record attr
            func(error_tracker, FunctionalTarget.RECORD, record.raw_attrs)

            # field
            for field in record.fields:
                error_tracker.source_line = field.line
                error_tracker.push_path(f"[{field.name}]")
                func(error_tracker, FunctionalTarget.FIELD, field.raw_attrs)
                error_tracker.pop_path()

            # method
            for method in record.methods:
                error_tracker.source_line = method.line
                error_tracker.push_path(f"[{method.short_name}()]")

                # method attr
                func(error_tracker, FunctionalTarget.METHOD, method.raw_attrs)

                # parameter
                for (_, parameter) in method.parameters.items():
                    error_tracker.push_path(f"[{parameter.name}]")
                    func(error_tracker, FunctionalTarget.PARAMETER, parameter.raw_attrs)
                    error_tracker.pop_path()

                error_tracker.pop_path()

            error_tracker.pop_path()

        # enum
        for enum in self.enums:
            error_tracker.set_source(enum.file_name, enum.line)
            error_tracker.push_path(f"[{enum.name}]")

            # enum attr
            func(error_tracker, FunctionalTarget.ENUM, enum.raw_attrs)

            # enum value
            for (_, enum_value) in enum.values.items():
                error_tracker.source_line = enum_value.line
                error_tracker.push_path(f"[{enum_value.short_name}]")
                func(error_tracker, FunctionalTarget.ENUM_VALUE, enum_value.raw_attrs)
                error_tracker.pop_path()

            error_tracker.pop_path()

        # function
        for function in self.functions:
            error_tracker.set_source(function.file_name, function.line)
            error_tracker.push_path(f"[{function.name}()]")

            # function attrs
            func(error_tracker, FunctionalTarget.FUNCTION, function.raw_attrs)

            # parameters
            for (_, parameter) in function.parameters.items():
                error_tracker.push_path(f"[{parameter.name}]")
                func(error_tracker, FunctionalTarget.PARAMETER, parameter.raw_attrs)
                error_tracker.pop_path()

            error_tracker.pop_path()

    def __extract_cpp_types(self, raw_json_data: Dict[str, JsonValue]):
        # load records
        for (k, v) in raw_json_data["records"].unique_val().items():
            record = Record(k)
            record.load_from_raw_json(v.unique_val())
            self.records.append(record)
            self.__name_to_record[record.name] = record

        # load functions
        for v in raw_json_data["functions"].unique_val():
            function = Function()
            function.load_from_raw_json(v)
            self.functions.append(function)

        # load enums
        for (k, v) in raw_json_data["enums"].unique_val().items():
            enum = Enumeration(k)
            enum.load_from_raw_json(v.unique_val())
            self.enums.append(enum)
            self.__name_to_enum[enum.name] = enum


class ModuleDatabase:
    def __init__(self) -> None:
        # module name
        self.module_name: str = ""

        # header files
        self.header_files: List[str] = []

        # fast search
        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enumeration] = {}
