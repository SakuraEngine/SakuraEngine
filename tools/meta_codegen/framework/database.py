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

    def expand_shorthand_and_path(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        def __expand_shorthand_and_path(error_tracker: ErrorTracker, target: FunctionalTarget, raw_attrs: JsonDict) -> None:
            parser = parser_manager.functional[target]
            parser.dispatch_expand_shorthand_and_path(raw_attrs, error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __expand_shorthand_and_path)

    def check_structure(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        # check structure
        def __check_structure(error_tracker: ErrorTracker, target: FunctionalTarget, raw_attrs: JsonDict) -> None:
            parser = parser_manager.functional[target]
            parser.check_structure(raw_attrs, error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __check_structure)

        # check unrecognized attr

    def to_object(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        pass

    def __each_attrs_and_apply_functional(self, error_tracker: ErrorTracker, func):
        # record
        for record in self.records:
            error_tracker.set_source(record.file_name, record.line)
            with error_tracker.path_guard(f"[{record.name}]"):
                # record attr
                func(error_tracker, FunctionalTarget.RECORD, record.raw_attrs)

                # field
                for field in record.fields:
                    error_tracker.source_line = field.line
                    with error_tracker.path_guard(f"[{field.type}]"):
                        func(error_tracker, FunctionalTarget.FIELD, field.raw_attrs)

                # method
                for method in record.methods:
                    error_tracker.source_line = method.line
                    with error_tracker.path_guard(f"[{method.short_name}]"):
                        # method attr
                        func(error_tracker, FunctionalTarget.METHOD, method.raw_attrs)

                        # parameter
                        for (_, parameter) in method.parameters.items():
                            with error_tracker.path_guard(f"[{parameter.name}]"):
                                func(error_tracker, FunctionalTarget.PARAMETER, parameter.raw_attrs)

        # enum
        for enum in self.enums:
            error_tracker.set_source(enum.file_name, enum.line)
            with error_tracker.path_guard(f"[{enum.name}]"):
                # enum attr
                func(error_tracker, FunctionalTarget.ENUM, enum.raw_attrs)

                # enum value
                for (_, enum_value) in enum.values.items():
                    error_tracker.source_line = enum_value.line
                    with error_tracker.path_guard(f"[{enum_value.short_name}]"):
                        func(error_tracker, FunctionalTarget.ENUM_VALUE, enum_value.raw_attrs)

        # function
        for function in self.functions:
            error_tracker.set_source(function.file_name, function.line)
            with error_tracker.path_guard(f"[{function.name}]"):
                # function attrs
                func(error_tracker, FunctionalTarget.FUNCTION, function.raw_attrs)

                # parameters
                for (_, parameter) in function.parameters.items():
                    with error_tracker.path_guard(f"[{parameter.name}]"):
                        func(error_tracker, FunctionalTarget.PARAMETER, parameter.raw_attrs)

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
        self.header_dbs: List[HeaderDatabase] = []

        # fast search
        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enumeration] = {}

    def load_header(self, path: str):
        self.header_files.append(path)
        db = HeaderDatabase()
        db.load_header(path)
        self.header_dbs.append(db)

    def expand_shorthand_and_path(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        for db in self.header_dbs:
            db.expand_shorthand_and_path(error_tracker, parser_manager)

    def check_structure(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        for db in self.header_dbs:
            db.check_structure(error_tracker, parser_manager)

    def to_object(self, error_tracker: ErrorTracker, parser_manager: FunctionalManager) -> None:
        for db in self.header_dbs:
            db.to_object(error_tracker, parser_manager)
