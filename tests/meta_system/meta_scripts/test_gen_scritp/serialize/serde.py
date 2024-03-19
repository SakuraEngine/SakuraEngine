from framework.attr_parser import ParserManager
from framework.database import ParserManager
from framework.generator import *
from framework.database import *
from framework.attr_parser import *
import os
from typing import List
import mako
import mako.template


class SerdeGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser(
            "serde",
            FunctionalParser(
                options={
                    "blob": BoolParser(default_value=False),
                    "bin": BoolParser(default_value=False),
                    "json": BoolParser(default_value=False),
                    "transient": BoolParser(default_value=False),
                    "no-text": BoolParser(default_value=False),
                }
            )
        )
