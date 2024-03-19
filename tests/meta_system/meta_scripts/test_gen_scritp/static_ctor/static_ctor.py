from framework.attr_parser import ParserManager
from framework.database import ParserManager
from framework.generator import *
from framework.database import *
from framework.attr_parser import *
import os
from typing import List
import mako
import mako.template


class StaticCtorGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser(
            "static_ctor",
            StrParser(),
        )
