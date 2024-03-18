from framework.generator import *


class TestFrameworkGenerator(GeneratorBase):
    def load_functional(self, parser_manager: FunctionalManager):
        def assert_value_to_object(target_value, value, error_tracker: ErrorTracker):
            if value != target_value:
                error_tracker.add_error("value not match")
            return value

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

        # test check expand
        parser_manager.add_functional(
            FunctionalTarget.RECORD,
            "test_functional_shorthand",
            FunctionalParser(
                shorthands=[
                    StrShorthand(
                        options_mapping={
                            "all": {
                                "assert_10": 10,
                                "assert_fuck": "fuck",
                                "assert_11.1": 11.1,
                                "assert_false": False,
                                "assert_1_1_2": [1, 1, 2]
                            },
                            "bad_type": {
                                "assert_10": [1, 1, 2],
                                "assert_fuck": "fuck",
                                "assert_11.1": False,
                                "assert_false": 114514,
                                "assert_1_1_2": "fuck"
                            }
                        }
                    )
                ],
                options={
                    "assert_10": IntParser(),
                    "assert_fuck": StrParser(),
                    "assert_11.1": FloatParser(),
                    "assert_false": BoolParser(),
                    "assert_1_1_2": ListParser(),
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
