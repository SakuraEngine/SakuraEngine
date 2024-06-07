import framework.generator as gen
import framework.scheme as sc


class TestPathExpandGenerator(gen.GeneratorBase):
    def load_scheme(self):
        self.owner.add_record_scheme(
            sc.Namespace({
                "test_expand_path": sc.Functional({
                    "a": sc.Str(),
                    "b": sc.Functional({
                        "b": sc.Str(),
                    }),
                    "c": sc.Functional({
                        "c": sc.Functional({
                            "c": sc.Str()
                        })
                    })
                })
            })
        )

    def load_attrs(self):
        def __check(record):
            assert record is not None

            # find functional
            test_functional: sc.ParseResult = record.attrs["test_expand_path"]
            assert test_functional.is_functional()

            # check visited
            assert test_functional.is_visited()
            assert test_functional["a"].is_visited()
            assert test_functional["b"].is_visited()
            assert test_functional["b"]["b"].is_visited()
            assert test_functional["c"].is_visited()
            assert test_functional["c"]["c"].is_visited()
            assert test_functional["c"]["c"]["c"].is_visited()

            # check value
            assert test_functional["a"].parsed_value == "expand_a"
            assert test_functional["b"]["b"].parsed_value == "expand_b"
            assert test_functional["c"]["c"]["c"].parsed_value == "expand_c"

        def _check_if_exist(record):
            if record:
                __check(record)

        __check(self.owner.database.find_record("test_path_shorthand::PassCheck"))
        _check_if_exist(self.owner.database.find_record("test_path_shorthand::ErrorCase"))


class TestFunctionalShorthand(gen.GeneratorBase):
    def load_scheme(self):
        self.owner.add_record_scheme(
            sc.Namespace({
                "test_functional_shorthand": sc.Functional({
                    "test_enable": sc.Functional(),
                    "test_usual": sc.Functional({
                        "a": sc.Bool(),
                        "b": sc.Bool(),
                        "c": sc.Bool(),
                    }, shorthands=[sc.OptionShorthand({
                        "all": {
                            "a": True,
                            "b": True,
                            "c": True
                        }
                    })]),
                    "test_override": sc.Functional({
                        "cat_a": sc.Bool(),
                        "cat_b": sc.Bool(),
                        "cat_c": sc.Bool(),
                        "dog_a": sc.Bool(),
                        "dog_b": sc.Bool(),
                        "dog_c": sc.Bool(),
                    }, shorthands=[sc.OptionShorthand({
                        "all_cat": {
                            "cat_a": True,
                            "cat_b": True,
                            "cat_c": True
                        },
                        "all_dog": {
                            "dog_a": True,
                            "dog_b": True,
                            "dog_c": True
                        },
                        "except_c": {
                            "cat_c": False,
                            "dog_c": False
                        }
                    })])
                })
            })
        )

    def load_attrs(self):
        def __check(record):
            assert record is not None

            # find functional
            test_functional: sc.ParseResult = record.attrs["test_functional_shorthand"]
            assert test_functional.is_functional()

            # check visited
            assert test_functional.is_visited()
            assert test_functional["test_enable"].is_visited()
            assert test_functional["test_usual"].is_visited()
            assert test_functional["test_usual"]["a"].is_visited()
            assert test_functional["test_usual"]["b"].is_visited()
            assert test_functional["test_usual"]["c"].is_visited()
            assert test_functional["test_override"].is_visited()
            assert test_functional["test_override"]["cat_a"].is_visited()
            assert test_functional["test_override"]["cat_b"].is_visited()
            assert test_functional["test_override"]["cat_c"].is_visited()
            assert test_functional["test_override"]["dog_a"].is_visited()
            assert test_functional["test_override"]["dog_b"].is_visited()
            assert test_functional["test_override"]["dog_c"].is_visited()

            # check value
            assert test_functional["test_enable"]["enable"].parsed_value is True
            assert test_functional["test_usual"]["a"].parsed_value is True
            assert test_functional["test_usual"]["b"].parsed_value is True
            assert test_functional["test_usual"]["c"].parsed_value is True
            assert test_functional["test_override"]["cat_a"].parsed_value is True
            assert test_functional["test_override"]["cat_b"].parsed_value is True
            assert test_functional["test_override"]["cat_c"].parsed_value is False
            assert test_functional["test_override"]["dog_a"].parsed_value is True
            assert test_functional["test_override"]["dog_b"].parsed_value is True
            assert test_functional["test_override"]["dog_c"].parsed_value is True

        def __check_if_exist(record):
            if record:
                __check(record)

        __check(self.owner.database.find_record("test_functional_shorthand::PassCheck"))
        __check_if_exist(self.owner.database.find_record("test_functional_shorthand::ErrorCase"))
