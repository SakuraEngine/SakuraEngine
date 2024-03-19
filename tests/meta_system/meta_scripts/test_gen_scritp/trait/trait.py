from framework.attr_parser import ParserManager
from framework.database import ParserManager
from framework.generator import *
from framework.database import *
from framework.attr_parser import *
import os
from typing import List
import mako
import mako.template


class TraitGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser(
            "trait",
            FunctionalParser()
        )
        parser_manager.add_method_parser(
            "trait",
            FunctionalParser(
                options={
                    "getter": BoolParser(default_value=False),
                    "setter": BoolParser(default_value=False),
                }
            )
        )
