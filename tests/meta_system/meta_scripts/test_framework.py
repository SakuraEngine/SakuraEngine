import framework.generator as gen
import framework.scheme as sc


class TestFrameworkGenerator(gen.GeneratorBase):
    def load_scheme(self):
        self.owner.add_record_scheme(
            sc.Namespace(
                options={
                    "test_expand_path": sc.Functional(
                        options={
                            "a": sc.Str(),
                            "b": sc.Functional(
                                options={
                                    "b": sc.Str(),
                                }
                            ),
                            "c": sc.Functional(
                                options={
                                    "c": sc.Functional(
                                        options={
                                            "c": sc.Str()
                                        }
                                    )
                                }
                            )
                        }
                    ),
                    "test_check_override": sc.Functional(
                        options={
                            "test_override": sc.Str(),
                            "test_rewrite": sc.Functional(
                                options={
                                    "a": sc.Str(),
                                    "b": sc.Str(),
                                    "c": sc.Str()
                                }
                            ),
                            "test_append": sc.List()
                        }
                    ),
                    "test_functional_shorthand": sc.Functional(
                        shorthands=[
                            sc.OptionShorthand(
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
                            "assert_10": sc.Int(),
                            "assert_fuck": sc.Str(),
                            "assert_11.1": sc.Float(),
                            "assert_false": sc.Bool(),
                            "assert_1_1_2": sc.List(),
                        }
                    ),
                    "test_check_structure": sc.Functional(
                        options={
                            "int": sc.Int(),
                            "float": sc.Float(),
                            "str": sc.Str(),
                            "bool": sc.Bool(),
                            "sub": sc.Functional(
                                options={
                                    "count": sc.Int()
                                },
                            )
                        }
                    ),
                    "test_recognized_attr": sc.Functional(
                        options={
                            "a": sc.Str(),
                            "b": sc.Str(),
                            "sub": sc.Functional(
                                options={
                                    "sub_a": sc.Str(),
                                    "sub_b": sc.Str(),
                                }
                            )
                        }
                    )
                }
            )
        )
