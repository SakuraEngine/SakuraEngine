'''
对 RTTR 系统的试写：
builder.add_functional(
    target = FunctionalTarget.RECORD,
    name = "rttr",
    parser = FunctionalParser(
        default_enable = _default_enable_parser,
        shorthand = [
            StrShorthand(
                options_mapping = {
                    "all": { reflect_fields: True, reflect_methods: True, reflect_bases: True },
                }
            ),
            ListShorthand(
                options_mapping = {
                    "field": { reflect_fields: True },
                    "method": { reflect_methods: True }
                    "no-bases": { reflect_bases: False }
                }
            )
        ],
        options = {
            "reflect_bases": ValueParser(bool),
            "exclude_bases": ListParser(str),
            "reflect_fields": ValueParser(bool),
            "reflect_methods": ValueParser(bool),
        }
    )
)
'''

from typing import List, Dict
from dataclasses import dataclass
from enum import Enum
from framework.error_tracker import ErrorTracker
from framework.json_parser import JsonValue, setup_recognized_recursive


class ParserBase:
    def __init__(self) -> None:
        pass

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        if json_value.is_recognized:
            raise Exception("JsonValue is already recognized!")
        json_value.mark_recognized()


class FunctionalParser(ParserBase):
    def __init__(self, options={}, shorthands=[]) -> None:
        super().__init__()
        self.shorthands: List[Shorthand] = shorthands
        self.options: Dict[str, ParserBase] = options

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if isinstance(override.val, bool):  # enable shorthand
                pass
            elif isinstance(override.val, Dict):  # dict options
                for (k, v) in self.options.items:
                    if k in override.val:
                        ErrorTracker.push_path(k)
                        v.check_structure(override.val[k], error_tracker)
                        ErrorTracker.pop_path()
            else:  # shorthand
                for v in self.shorthands:
                    ErrorTracker.push_path("[shorthand]")
                    v.check_structure(override.val, error_tracker)
                    ErrorTracker.pop_path()


class ValueParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()


class BoolParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if not isinstance(override.val, bool):
                error_tracker.error("Value must be bool!")


class StrParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if not isinstance(override.val, str):
                error_tracker.error("Value must be str!")


class IntParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if not isinstance(override.val, int):
                error_tracker.error("Value must be int!")


class FloatParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if not isinstance(override.val, float):
                error_tracker.error("Value must be float!")


class ListParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        for override in json_value.overrides:
            if not isinstance(override.val, list):
                error_tracker.error("Value must be list!")


class DictParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        super().check_structure(json_value, error_tracker)
        setup_recognized_recursive(json_value, True)
        for override in json_value.overrides:
            if not isinstance(override.val, dict):
                error_tracker.error("Value must be dict!")


class Shorthand:
    def __init__(self, options_mapping) -> None:
        self.options_mapping: Dict[str, Dict[str, object]] = options_mapping

    def check_structure(self, json_value: JsonValue, error_tracker: ErrorTracker) -> None:
        pass


class FunctionalTarget(Enum):
    RECORD = 1
    FIELD = 2
    METHOD = 3
    PARAMETER = 4
    FUNCTION = 5
    ENUM = 6
    ENUM_VALUE = 7


class FunctionalManager:
    def __init__(self) -> None:
        self.functional: Dict[FunctionalTarget, Dict[str, List[FunctionalParser]]] = {
            FunctionalTarget.RECORD: {},
            FunctionalTarget.FIELD: {},
            FunctionalTarget.METHOD: {},
            FunctionalTarget.PARAMETER: {},
            FunctionalTarget.FUNCTION: {},
            FunctionalTarget.ENUM: {},
            FunctionalTarget.ENUM_VALUE: {},
        }

    def add_functional(self, target: FunctionalTarget, key: str, parser: FunctionalParser) -> None:
        if not isinstance(parser, FunctionalParser):
            raise TypeError("parser must be a FunctionalParser!")
        if target not in self.functional:
            self.functional[target] = {}
        if key not in self.functional[target]:
            self.functional[target][key] = []
        self.functional[target][key].append(parser)
