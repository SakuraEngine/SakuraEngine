import os
import os.path
from typing import List, Dict
import itertools
import json
from framework.json_parser import *
from framework.cpp_types import *
from framework.error_tracker import *
from framework.attr_parser import *


class HeaderDatabase:
    '''
    load step:
        1. load raw data
        2. load cpp types

    attr parse step (v0):
        1. check structure
        2. check unrecognized attr
        3. check path
        4. expand shorthand
        5. to object

    attr parse step:
        1. check structure -> longhand
            - error value type
        2. expand shorthand -> shorthand
            - unrecognized shorthand
            - [check structure]
        3. expand path -> longhand path, expanded path from shorthand
            - error value type
        4. check unrecognized attr -> longhand, shorthand, path (none expand)
        5. to object (and solve override)

    attr parse step (v2):
        1. expand shorthand (需要考虑 shorthand 中的 shorthand)
        2. expand path (需要考虑 path 的 末端为 shorthand)
        3. check structure (不同的 source 有不同的 path 组装风格)
        4. to object

    attr parse step(v3):
        0. expand path(在 json_parser 中完成)
        1. expand shorthand (shorthand 的映射不提供 path 解析，shorthand 中的 shorthand 和 path 在输出时直接完成展开)

        2. check structure (检查值是否正确的的同时，检查值是否被识别)
        3. to object
    '''

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
        raw_json: JsonDict
        with open(path, encoding="utf-8") as f:
            raw_json = json.load(f, object_pairs_hook=parse_json_hook)

        # extract cpp types
        self.__extract_cpp_types(raw_json)

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

    def __extract_cpp_types(self, raw_json: JsonDict):
        unique_dict = raw_json.unique_dict()
        records = unique_dict["records"].unique_dict()
        functions = unique_dict["functions"]
        enums = unique_dict["enums"].unique_dict()

        # load records
        for (record_name, record_data) in records.items():
            record = Record(record_name)
            record.load_from_raw_json(record_data)
            self.records.append(record)
            self.__name_to_record[record.name] = record

        # load functions
        for function_data in functions:
            function = Function()
            function.load_from_raw_json(function_data)
            self.functions.append(function)

        # load enums
        for (enum_name, enum_data) in enums.items():
            enum = Enumeration(enum_name)
            enum.load_from_raw_json(enum_data)
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
