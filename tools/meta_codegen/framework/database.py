import glob
import os
import os.path
from typing import List, Dict
import itertools
import json
from framework.json_parser import *
from framework.cpp_types import *
from framework.error_tracker import *
from framework.attr_parser import *
from framework.config import CodegenConfig, ModuleConfig
import re


class HeaderDatabase:
    def __init__(self, module_db: 'ModuleDatabase') -> None:
        self.module_db: 'ModuleDatabase' = module_db

        # header file path
        self.meta_path: str = ""
        self.relative_meta_path: str = ""
        self.relative_target_header_path: str = ""
        self.include_header_path: str = ""
        self.file_id: str = ""

        # all data
        self.records: List[Record] = []
        self.enums: List[Enumeration] = []
        self.functions: List[Function] = []

        # name to data
        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enumeration] = {}

    def get_records(self):
        return self.records

    def get_enums(self):
        return self.enums

    def get_functions(self):
        return self.functions

    def load_header(self, meta_file_path: str, config: CodegenConfig):
        self.meta_path = os.path.normpath(meta_file_path).replace(os.sep, "/")
        self.relative_meta_path = os.path.relpath(self.meta_path, self.module_db.meta_dir).replace(os.sep, "/")
        self.relative_target_header_path = re.sub(r"(.*?)\.(.*?)\.meta", r"\g<1>.generated.\g<2>", self.relative_meta_path).replace(os.sep, "/")
        self.file_id = f"FID_{self.module_db.module_name}_" + re.sub(r'\W+', '_', self.relative_meta_path)

        # load raw data
        raw_json: JsonDict
        with open(meta_file_path, encoding="utf-8") as f:
            raw_json = json.load(f, object_pairs_hook=parse_json_hook)

        # extract cpp types
        self.__extract_cpp_types(raw_json)

        # get include header path
        for record in self.records:
            include_path = record.file_name.replace(os.sep, "/")
            if self.include_header_path and self.include_header_path != include_path:
                raise Exception(f"Include header path is different: {self.include_header_path} != {include_path}")
            self.include_header_path = include_path
        for enum in self.enums:
            include_path = enum.file_name.replace(os.sep, "/")
            if self.include_header_path and self.include_header_path != include_path:
                raise Exception(f"Include header path is different: {self.include_header_path} != {include_path}")
            self.include_header_path = include_path
        for function in self.functions:
            include_path = function.file_name.replace(os.sep, "/")
            if self.include_header_path and self.include_header_path != include_path:
                raise Exception(f"Include header path is different: {self.include_header_path} != {include_path}")
            self.include_header_path = include_path

    def expand_shorthand_and_path(self, error_tracker: ErrorTracker, parser_manager: ParserManager) -> None:
        def __expand_shorthand_and_path(error_tracker: ErrorTracker, target: FunctionalTarget, cpp_obj: object) -> None:
            parser = parser_manager.parser_dict[target]
            parser.dispatch_expand_shorthand_and_path(cpp_obj.raw_attrs, error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __expand_shorthand_and_path)

    def check_structure(self, error_tracker: ErrorTracker, parser_manager: ParserManager) -> None:
        # check structure
        def __check_structure(error_tracker: ErrorTracker, target: FunctionalTarget, cpp_obj: object) -> None:
            parser = parser_manager.parser_dict[target]
            parser.check_structure(cpp_obj.raw_attrs, error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __check_structure)

        # check unrecognized attr
        def __check_unrecognized_attr(error_tracker: ErrorTracker, target: FunctionalTarget, cpp_obj: object) -> None:
            cpp_obj.raw_attrs.warning_recognized_attr_recursive(error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __check_unrecognized_attr)

    def to_object(self, error_tracker: ErrorTracker, parser_manager: ParserManager) -> None:
        def __to_object(error_tracker: ErrorTracker, target: FunctionalTarget, cpp_obj: object) -> None:
            parser = parser_manager.parser_dict[target]
            solver = JsonOverrideSolver()
            solver.root_dict = cpp_obj.raw_attrs
            cpp_obj.attrs = parser.parse_to_object(solver, error_tracker)
        self.__each_attrs_and_apply_functional(error_tracker, __to_object)

    def dump_attr_object(self):
        def __dump_object(error_tracker: ErrorTracker, target: FunctionalTarget, cpp_obj: object) -> None:
            print(f"===================={target}====================")
            print(cpp_obj.attrs.__dict__)
        self.__each_attrs_and_apply_functional(ErrorTracker(), __dump_object)

    def __each_attrs_and_apply_functional(self, error_tracker: ErrorTracker, func):
        # record
        for record in self.records:
            error_tracker.set_source(record.file_name, record.line)
            with error_tracker.path_guard(f"[{record.name}]"):
                # record attr
                func(error_tracker, FunctionalTarget.RECORD, record)

                # field
                for field in record.fields:
                    error_tracker.set_source_line(field.line)
                    with error_tracker.path_guard(f"[{field.type}]"):
                        func(error_tracker, FunctionalTarget.FIELD, field)

                # method
                for method in record.methods:
                    error_tracker.set_source_line(method.line)
                    with error_tracker.path_guard(f"[{method.short_name}]"):
                        # method attr
                        func(error_tracker, FunctionalTarget.METHOD, method)

                        # parameter
                        for (_, parameter) in method.parameters.items():
                            with error_tracker.path_guard(f"[{parameter.name}]"):
                                func(error_tracker, FunctionalTarget.PARAMETER, parameter)

        # enum
        for enum in self.enums:
            error_tracker.set_source(enum.file_name, enum.line)
            with error_tracker.path_guard(f"[{enum.name}]"):
                # enum attr
                func(error_tracker, FunctionalTarget.ENUM, enum)

                # enum value
                for (_, enum_value) in enum.values.items():
                    error_tracker.set_source_line(enum_value.line)
                    with error_tracker.path_guard(f"[{enum_value.short_name}]"):
                        func(error_tracker, FunctionalTarget.ENUM_VALUE, enum_value)

        # function
        for function in self.functions:
            error_tracker.set_source(function.file_name, function.line)
            with error_tracker.path_guard(f"[{function.name}]"):
                # function attrs
                func(error_tracker, FunctionalTarget.FUNCTION, function)

                # parameters
                for (_, parameter) in function.parameters.items():
                    with error_tracker.path_guard(f"[{parameter.name}]"):
                        func(error_tracker, FunctionalTarget.PARAMETER, parameter)

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
    def __init__(self, ) -> None:
        # module info
        self.module_name: str = ""
        self.meta_dir: str = ""
        self.api: str = ""

        # header files
        self.header_dbs: List[HeaderDatabase] = []

        # fast search
        self.__name_to_record: Dict[str, Record] = {}
        self.__name_to_enum: Dict[str, Enumeration] = {}

    def load(self, module_config: ModuleConfig, config: CodegenConfig):
        # load module info
        self.module_name = module_config.module_name
        self.meta_dir = module_config.meta_dir
        self.api = module_config.api

        # get files
        meta_files = glob.glob(os.path.join(module_config.meta_dir, "**", "*.h.meta"), recursive=True)

        # load meta files
        for meta_file in meta_files:
            # load header db
            db = HeaderDatabase(self)
            db.load_header(meta_file, config)
            self.header_dbs.append(db)

            # append to fast search
            for record in db.records:
                if record.name in self.__name_to_record:
                    raise Exception(f"Record name {record.name} is duplicated")
                self.__name_to_record[record.name] = record
            for enum in db.enums:
                if enum.name in self.__name_to_enum:
                    raise Exception(f"Enum name {enum.name} is duplicated")
                self.__name_to_enum[enum.name] = enum

    def solve_attr(self, config: CodegenConfig, error_tracker: ErrorTracker, parser_manager: ParserManager) -> None:
        # expand_shorthand_and_path
        error_tracker.set_phase("ExpandShorthandAndPath")
        for db in self.header_dbs:
            db.expand_shorthand_and_path(error_tracker, parser_manager)
        if error_tracker.any_error():
            error_tracker.dump()
            raise Exception("ExpandShorthandAndPath failed")

        # check_structure
        error_tracker.set_phase("CheckStructure")
        for db in self.header_dbs:
            db.check_structure(error_tracker, parser_manager)
        if error_tracker.any_error():
            error_tracker.dump()
            raise Exception("ExpandShorthandAndPath failed")

        # to_object
        error_tracker.set_phase("ToObjects")
        for db in self.header_dbs:
            db.to_object(error_tracker, parser_manager)
        if error_tracker.any_error():
            error_tracker.dump()
            raise Exception("ExpandShorthandAndPath failed")

    def get_records(self):
        return self.__name_to_record.values()

    def get_enums(self):
        return self.__name_to_enum.values()

    def get_functions(self):
        return itertools.chain.from_iterable([db.functions for db in self.header_dbs])


class CodegenDatabase:
    def __init__(self) -> None:
        self.main_module: ModuleDatabase = None
        self.include_modules: List[ModuleDatabase] = []

    def get_records(self):
        return itertools.chain(self.main_module.get_records(), [module.get_records() for module in self.include_modules])

    def get_enums(self):
        return itertools.chain(self.main_module.get_enums(), [module.get_enums() for module in self.include_modules])

    def get_functions(self):
        return itertools.chain(self.main_module.get_functions(), [module.get_functions() for module in self.include_modules])
