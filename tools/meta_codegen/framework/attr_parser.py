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

from typing import List, Dict, Tuple
from dataclasses import dataclass
from enum import Enum
from framework.error_tracker import ErrorTracker
from framework.json_parser import *
from framework.json_parser import JsonDict


class ParserBase:
    def __init__(self) -> None:
        self.owner: ParserBase

    def is_path_valid(self, path_nodes: List[str]):
        raise NotImplementedError("is_path_valid is not implemented!")

    def expand_shorthand_and_path(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        pass

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        pass

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        pass


class RootParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()
        self.functional: Dict[str, List[FunctionalParser]] = {}

    def add_functional(self, name, parser) -> None:
        if name not in self.functional:
            self.functional[name] = []
        self.functional[name].append(parser)

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        json_dict.expand_path_recursive()

        def __dispatch(k, dict):
            if k in self.functional:
                for functional in self.functional[k]:
                    functional.dispatch_expand_shorthand_and_path(dict, error_tracker)

        def __expand(k, v):
            if k in self.functional:
                for functional in self.functional[k]:
                    mapping = functional.expand_shorthand_and_path(v, error_tracker)
                    if mapping:  # TODO. 支持多 mapping 合并映射
                        return mapping

        json_dict.expand_shorthand(__expand, __dispatch)

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is JsonDict:
            for (k, override) in value:
                override.push_path(error_tracker, k)
                if k in self.functional:
                    for functional in self.functional[k]:
                        functional.check_structure(override.val, error_tracker)
                override.pop_path(error_tracker)
        else:
            error_tracker.error("value type error, must be JsonDict!")


class FunctionalParser(ParserBase):
    def __init__(self, options: Dict[str, ParserBase] = {}, shorthands: List['Shorthand'] = []) -> None:
        super().__init__()
        self.shorthands: List[Shorthand] = shorthands
        self.options: Dict[str, ParserBase] = options

        # update owner
        for v in self.shorthands:
            v.owner = self
        for (k, v) in self.options.items():
            v.owner = self

    def expand_shorthand_and_path(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        for shorthand_parser in self.shorthands:
            mapping = shorthand_parser.expand_shorthand(shorthand, error_tracker)
            if mapping:  # TODO. 支持多 mapping 合并映射
                return mapping

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        def __dispatch(k, dict):
            if k in self.options:
                for parser in self.options[k]:
                    parser.dispatch_expand_shorthand_and_path(dict, error_tracker)

        def __expand(k, v):
            if k in self.options:
                for parser in self.options[k]:
                    mapping = parser.expand_shorthand_and_path(v, error_tracker)
                    if mapping:  # TODO. 支持多 mapping 合并映射
                        return mapping

        json_dict.expand_shorthand(__expand, __dispatch)

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is JsonDict:
            for (k, override) in value:
                override.push_path(error_tracker, k)
                if k in self.options:
                    self.options[k].check_structure(override.val, error_tracker)
                override.pop_path(error_tracker)
        else:
            error_tracker.error("value type error, must be JsonDict!")


class ValueParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()


class BoolParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not bool:
            error_tracker.error("value type error, must be bool!")


class StrParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not str:
            error_tracker.error("value type error, must be str!")


class IntParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        print(value)
        if type(value) is not int:
            error_tracker.error("value type error, must be int!")


class FloatParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not float:
            error_tracker.error("value type error, must be float!")


class ListParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not list:
            error_tracker.error("value type error, must be list!")


class DictParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not dict:
            error_tracker.error("value type error, must be dict!")


class Shorthand:
    def __init__(self, options_mapping) -> None:
        self.owner: FunctionalParser
        self.options_mapping: Dict[str, Dict[str, object]] = options_mapping

    def expand_shorthand(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        raise NotImplementedError("expand_shorthand is not implemented!")


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
        self.functional: Dict[FunctionalTarget, RootParser] = {
            FunctionalTarget.RECORD: RootParser(),
            FunctionalTarget.FIELD: RootParser(),
            FunctionalTarget.METHOD: RootParser(),
            FunctionalTarget.PARAMETER: RootParser(),
            FunctionalTarget.FUNCTION: RootParser(),
            FunctionalTarget.ENUM: RootParser(),
            FunctionalTarget.ENUM_VALUE: RootParser(),
        }

    def add_functional(self, target: FunctionalTarget, key: str, parser: FunctionalParser) -> None:
        if type(parser) is not FunctionalParser:
            raise TypeError("parser must be a FunctionalParser!")
        self.functional[target].add_functional(key, parser)
