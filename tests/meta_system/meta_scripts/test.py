from framework.generator import *


class Generator:
    def load_functional(self, parser_manager: FunctionalManager):
        parser = FunctionalParser(
            options={
                "int": IntParser(),
                "float": FloatParser(),
                "str": StrParser(),
                "bool": BoolParser(),
                "sub": FunctionalParser(
                    options={
                        "count": IntParser()
                    },
                )
            }
        )

        # record
        parser_manager.add_functional(FunctionalTarget.RECORD, "test", parser)
        parser_manager.add_functional(FunctionalTarget.FIELD, "test", parser)
        parser_manager.add_functional(FunctionalTarget.METHOD, "test", parser)
        parser_manager.add_functional(FunctionalTarget.ENUM, "test", parser)
        parser_manager.add_functional(FunctionalTarget.ENUM_VALUE, "test", parser)
        parser_manager.add_functional(FunctionalTarget.FUNCTION, "test", parser)
        parser_manager.add_functional(FunctionalTarget.PARAMETER, "test", parser)
