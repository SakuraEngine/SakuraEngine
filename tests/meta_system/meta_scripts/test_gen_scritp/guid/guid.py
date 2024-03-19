from framework.attr_parser import *
from framework.database import *
from framework.generator import *


class GUIDGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser("guid", StrParser(default_value=""))
        parser_manager.add_enum_parser("guid", StrParser(default_value=""))


def load_generators():
    return [
        GUIDGenerator()
    ]
