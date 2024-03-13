from framework.generator import *


class Generator:
    def load_functional(self, parser_manager: FunctionalManager):
        # test expand path
        # TODO. 使用 check_value 和 to_object 联合检查
        parser_manager.add_functional(
            FunctionalTarget.RECORD,
            "test_expand_path",
            FunctionalParser(
                options={
                    "a": StrParser(),
                    "b": FunctionalParser(
                        options={
                            "b": StrParser(),
                        }
                    ),
                    "c": FunctionalParser(
                        options={
                            "c": FunctionalParser(
                                options={
                                    "c": StrParser()
                                }
                            )
                        }
                    )
                }
            ))

        # test check override
        parser_manager.add_functional(
            FunctionalTarget.RECORD,
            "test_check_override",
            FunctionalParser(
                options={
                    "test_override": StrParser(),
                    "test_rewrite": FunctionalParser(
                        options={
                            "a": StrParser(),
                            "b": StrParser(),
                            "c": StrParser()
                        }
                    ),
                    "test_append": ListParser()
                }
            )
        )

        # record
        parser_manager.add_functional(
            FunctionalTarget.RECORD,
            "test_check_structure",
            FunctionalParser(
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
            ))
        parser_manager.add_functional(
            FunctionalTarget.RECORD,
            "test_unrecognized_attr",
            FunctionalParser(
                options={
                    "a": StrParser(),
                    "b": StrParser(),
                    "sub": FunctionalParser(
                        options={
                            "sub_a": StrParser(),
                            "sub_b": StrParser(),
                        }
                    )
                }
            ))
