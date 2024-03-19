from framework.attr_parser import ErrorTracker, ParserManager
from framework.database import ErrorTracker, ModuleDatabase, ParserManager
from framework.generator import *
from framework.database import *
from framework.attr_parser import *
from framework.generator import *
import os
import re


class LuaGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser(
            "lua",
            FunctionalParser()
        )

        parser_manager.add_method_parser(
            "lua",
            FunctionalParser()
        )

        parser_manager.add_enum_parser(
            "lua",
            FunctionalParser(
                options={
                    "category": StrParser(),
                }
            )
        )

        parser_manager.add_function_parser(
            "lua",
            FunctionalParser(
                options={
                    "category": StrParser(),
                    "binder": BoolParser(),
                }
            )
        )

        parser_manager.add_parameter_parser(
            "lua",
            FunctionalParser(
                options={
                    "out": BoolParser(),
                    "inout": BoolParser(),
                    "userdata": BoolParser(),
                }
            )
        )


def load_generators():
    return [
        LuaGenerator()
    ]
