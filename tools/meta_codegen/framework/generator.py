import os
import mako
import mako.template
import json
import sys
import importlib
import importlib.util
from dataclasses import dataclass, field
from typing import Dict, List
from enum import Enum

import framework.cpp_types as cpp
import framework.scheme as sc
import framework.database as db
import framework.log as log
import framework.config as config

# ------------------------------ generators ------------------------------
# 如果要支持 cpp 以外的 Generator 类型，需要提供更高一层的自定义 GeneratorManager 的抽象，来支持不同类型的代码生成
# 一般来说，很少会存在与 CPP 同等复杂的需求，因此暂时不提供这个抽象，而且如果有这类需求，在构建系统侧进行处理会更为合适
# 比方说封装一个 rule，并在 codegen target 中增加这个 rule 以完成 codegen 工作


class GeneratorBase:
    def __init__(self) -> None:
        self.name: str = ""
        self.owner: 'GenerateManager' = None

    # ---- generator lifecycle

    def require_include_dbs(self) -> bool:
        return False

    def load_scheme(self):
        pass

    def check_attrs(self):
        pass

    def pre_generate(self):
        pass

    def generate(self):
        pass

    def post_generate(self):
        pass

# ------------------------------ generate manager ------------------------------

# codegen workflow
#   --- generator load，加载生成器，做前期处理
#   1. load generators
#   2. init generators (在此处阶段可以检查 generator 依赖，向别的 generator 注入信息等)
#   3. load scheme
#   --- scheme parse，本阶段只进行语法解析，不进复杂的验证操作
#   4. expand shorthand and path
#   5. check structure
#   6. to object (with attr stack)，本阶段可以顺便将如 guid 等复杂属性解析出来
#   --- check attribute，检查属性和属性组合是否合法，如果有值域限制，也在此处检查
#   7. check attribute
#   --- generate code
#   8. pre generate，用于生成文件的头部内容，也可以在此阶段向其它 generator（如 generate body）注入内容
#   9. generate，用于生成文件的中段内容，通常在此阶段生成代码
#   10. post generate，用于生成文件的尾部内容


class GenerateManager:
    def __init__(self) -> None:
        self.__config: config.CodegenConfig = config.CodegenConfig()

        # generator
        self.__generators: Dict[str, GeneratorBase] = {}

        # scheme
        self.__schemes: Dict[type, sc.Namespace] = {}

        # database
        self.__database: db.CodegenDatabase = db.CodegenDatabase()

        # logger
        self.__logger: log.Logger = log.Logger()

        # file cache
        self.__file_content: Dict[str, str] = {}

    # ----- getter
    @property
    def config(self) -> config.CodegenConfig:
        return self.__config

    @property
    def logger(self) -> log.Logger:
        return self.__logger

    @property
    def database(self) -> db.CodegenDatabase:
        return self.__database

    # ----- generator
    def add_generator(self, name: str, generator: GeneratorBase):
        if name in self.__generators:
            raise Exception("generator %s already exists" % name)
        generator.name = name
        generator.owner = self
        self.__generators[name] = generator

    def get_generator(self, name: str) -> GeneratorBase:
        return self.__generators.get(name, None)

    # ----- scheme
    def add_record_scheme(self, scheme: sc.Scheme):
        if cpp.Record not in self.__schemes:
            self.__schemes[cpp.Record] = sc.Namespace()
        self.__schemes[cpp.Record].merge_scheme(scheme)

    def add_field_scheme(self, scheme: sc.Scheme):
        if cpp.Field not in self.__schemes:
            self.__schemes[cpp.Field] = sc.Namespace()
        self.__schemes[cpp.Field].merge_scheme(scheme)

    def add_method_scheme(self, scheme: sc.Scheme):
        if cpp.Method not in self.__schemes:
            self.__schemes[cpp.Method] = sc.Namespace()
        self.__schemes[cpp.Method].merge_scheme(scheme)

    def add_parameter_scheme(self, scheme: sc.Scheme):
        if cpp.Parameter not in self.__schemes:
            self.__schemes[cpp.Parameter] = sc.Namespace()
        self.__schemes[cpp.Parameter].merge_scheme(scheme)

    def add_function_scheme(self, scheme: sc.Scheme):
        if cpp.Function not in self.__schemes:
            self.__schemes[cpp.Function] = sc.Namespace()
        self.__schemes[cpp.Function].merge_scheme(scheme)

    def add_enum_scheme(self, scheme: sc.Scheme):
        if cpp.Enumeration not in self.__schemes:
            self.__schemes[cpp.Enumeration] = sc.Namespace()
        self.__schemes[cpp.Enumeration].merge_scheme(scheme)

    def add_enum_value_scheme(self, scheme: sc.Scheme):
        if cpp.EnumerationValue not in self.__schemes:
            self.__schemes[cpp.EnumerationValue] = sc.Namespace()
        self.__schemes[cpp.EnumerationValue].merge_scheme(scheme)

    # ----- file cache
    def load_template(self, template_path: str) -> mako.template.Template:
        with open(template_path, "rb") as f:
            return mako.template.Template(
                f.read(),
                filename=template_path,
                input_encoding="utf-8",
                strict_undefined=True,
            )

    def append_content(self, relative_path: str, content: str):
        solve_path = os.path.normpath(os.path.join(self.__config.output_dir, relative_path))
        if solve_path in self.__file_content:
            self.__file_content[solve_path] = self.__file_content[solve_path] + content
        else:
            self.__file_content[solve_path] = content

    def get_content(self, relative_path: str) -> str:
        solve_path = os.path.normpath(os.path.join(self.__config.output_dir, relative_path))
        return self.__file_content[solve_path]

    # ----- codegen workflow

    def load_config(self, config_file=None, config_content=None) -> None:
        json_data = None
        if config_content:
            json_data = json.loads(config_content)
        elif config_file:
            json_data = json.load(config_file)

        self.__config = config.CodegenConfig()
        self.__config.load(json_data)

    def load_generators(self) -> None:
        for i, generator_config in enumerate(self.__config.generators):
            if not generator_config.use_new_framework:
                continue

            # add import dir
            for path in generator_config.import_dirs:
                sys.path.insert(0, path)

            # load module
            spec = importlib.util.spec_from_file_location("Generator%d" % i, generator_config.entry_file)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            # load generator
            module.load_generators(self)
        self.__error_exit("load generators")

    def load_database(self) -> None:
        # check if require include dbs
        require_include_dbs = False
        for generator in self.__generators.values():
            require_include_dbs = require_include_dbs or generator.require_include_dbs()

        # load db
        self.__database.load(require_include_dbs, self.__config)
        self.__error_exit("load database")

    def parse_attr(self) -> None:
        # load scheme
        for generator in self.__generators.values():
            generator.load_scheme()
        self.__error_exit("load scheme")

        # expand shorthand and path
        def __expand_shorthand_and_path(cpp_type):
            scheme = self.__schemes.get(type(cpp_type), None)
            if scheme:
                with self.__logger.stack_scope(cpp_type.make_log_stack()):
                    scheme.dispatch_expand_shorthand(cpp_type.raw_attrs, self.__logger)
                    scheme.dispatch_expand_path(cpp_type.raw_attrs, self.__logger)
        self.__database.each_cpp_types_with_attr(__expand_shorthand_and_path)
        self.__error_exit("expand shorthand and path")

        # check structure
        def __check_structure(cpp_type):
            scheme = self.__schemes.get(type(cpp_type), None)
            if scheme:
                with self.__logger.stack_scope(cpp_type.make_log_stack()):
                    scheme.dispatch_check_structure(cpp_type.raw_attrs, self.__logger)
            with self.__logger.stack_scope(cpp_type.make_log_stack()):
                cpp_type.raw_attrs.check_unrecognized_attrs(self.__logger)
        self.__database.each_cpp_types_with_attr(__check_structure)
        self.__error_exit("check structure")

        # parse to object (with attr stack)
        def __parse_to_object(cpp_type):
            scheme = self.__schemes.get(type(cpp_type), None)
            override_solve = sc.JsonOverrideSolver(cpp_type.raw_attrs)
            if scheme:
                with self.__logger.stack_scope(cpp_type.make_log_stack()):
                    cpp_type.attrs = scheme.dispatch_parse_to_object(override_solve, self.__logger)
        self.__database.each_cpp_types_with_attr(__parse_to_object)
        self.__error_exit("parse to object")

    def check_attr(self) -> None:
        for generator in self.__generators.values():
            generator.check_attrs()
        self.__error_exit("check attribute")

    def pre_generate(self) -> None:
        for generator in self.__generators.values():
            generator.pre_generate()
        self.__error_exit("pre generate")

    def generate(self) -> None:
        for generator in self.__generators.values():
            generator.generate()
        self.__error_exit("generate")

    def post_generate(self) -> None:
        for generator in self.__generators.values():
            generator.post_generate()
        self.__error_exit("post generate")

    def output_content(self) -> None:
        for file, content in self.__file_content.items():
            directory = os.path.dirname(file)
            if not os.path.exists(directory):
                os.makedirs(directory, exist_ok=True)
            with open(file, "wb") as f:
                f.write(content.encode("utf-8"))

    def __error_exit(self, phase: str) -> None:
        if self.__logger.any_error():
            self.__logger.dump(error_mode=True)
            raise Exception("toggle error when %s" % phase)
