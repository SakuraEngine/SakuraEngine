import glob
import os
import os.path
import itertools
import json
import re
from typing import List, Dict
from enum import Enum
import framework.scheme as sc
import framework.cpp_types as cpp
import framework.log as log
import framework.config as config


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
        self.records: List[cpp.Record] = []
        self.enums: List[cpp.Enumeration] = []
        self.functions: List[cpp.Function] = []

        # name to data
        self.__name_to_record: Dict[str, cpp.Record] = {}
        self.__name_to_enum: Dict[str, cpp.Enumeration] = {}

    def get_records(self):
        return self.records

    def get_enums(self):
        return self.enums

    def get_functions(self):
        return self.functions

    def load_header(self, meta_file_path: str, config: config.CodegenConfig):
        self.meta_path = os.path.normpath(meta_file_path).replace(os.sep, "/")
        self.relative_meta_path = os.path.relpath(self.meta_path, self.module_db.meta_dir).replace(os.sep, "/")
        self.relative_target_header_path = re.sub(r"(.*?)\.(.*?)\.meta", r"\g<1>.generated.\g<2>", self.relative_meta_path).replace(os.sep, "/")
        self.file_id = f"FID_{self.module_db.module_name}_" + re.sub(r'\W+', '_', self.relative_meta_path)

        # load raw data
        raw_json: sc.JsonObject
        with open(meta_file_path, encoding="utf-8") as f:
            raw_json = json.load(f, object_pairs_hook=sc.json_object_pairs_hook)

        # extract cpp types
        unique_dict = raw_json.unique_dict()
        records = unique_dict["records"].unique_dict()
        functions = unique_dict["functions"]
        enums = unique_dict["enums"].unique_dict()

        # load records
        for (record_name, record_data) in records.items():
            record = cpp.Record(record_name)
            record.load_from_raw_json(record_data)
            self.records.append(record)
            self.__name_to_record[record.name] = record

        # load functions
        for function_data in functions:
            function = cpp.Function()
            function.load_from_raw_json(function_data)
            self.functions.append(function)

        # load enums
        for (enum_name, enum_data) in enums.items():
            enum = cpp.Enumeration(enum_name)
            enum.load_from_raw_json(enum_data)
            self.enums.append(enum)
            self.__name_to_enum[enum.name] = enum

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

    def each_cpp_types_with_attr(self, visitor) -> None:
        # record
        for record in self.records:
            visitor(record)

            # fields
            for field in record.fields:
                visitor(field)

            # methods
            for method in record.methods:
                visitor(method)
                # method parameters
                for parameters in method.parameters.values():
                    visitor(parameters)

        # enum
        for enum in self.enums:
            visitor(enum)

            # enum values
            for enum_value in enum.values.values():
                visitor(enum_value)

        # function
        for function in self.functions:
            visitor(function)

            # function parameters
            for parameter in function.parameters.values():
                visitor(parameter)


class ModuleDatabase:
    def __init__(self, ) -> None:
        # module info
        self.module_name: str = ""
        self.meta_dir: str = ""
        self.api: str = ""

        # header files
        self.header_dbs: List[HeaderDatabase] = []

        # fast search
        self.__name_to_record: Dict[str, cpp.Record] = {}
        self.__name_to_enum: Dict[str, cpp.Enumeration] = {}

    def load(self, module_config: config.ModuleConfig, config: config.CodegenConfig):
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

    def each_cpp_types_with_attr(self, visitor) -> None:
        for db in self.header_dbs:
            db.each_cpp_types_with_attr(visitor)

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

    def load(self, load_include_modules: bool, config: config.CodegenConfig):
        # load main module
        self.main_module = ModuleDatabase()
        self.main_module.load(config.main_module, config)

        # load include modules
        if load_include_modules:
            for include_module in config.include_modules:
                module_db = ModuleDatabase()
                module_db.load(include_module, config)
                self.include_modules.append(module_db)

    def each_cpp_types_with_attr(self, visitor) -> None:
        self.main_module.each_cpp_types_with_attr(visitor)
        for include_module in self.include_modules:
            include_module.each_cpp_types_with_attr(visitor)

    def get_records(self):
        return itertools.chain(self.main_module.get_records(), [module.get_records() for module in self.include_modules])

    def get_enums(self):
        return itertools.chain(self.main_module.get_enums(), [module.get_enums() for module in self.include_modules])

    def get_functions(self):
        return itertools.chain(self.main_module.get_functions(), [module.get_functions() for module in self.include_modules])
