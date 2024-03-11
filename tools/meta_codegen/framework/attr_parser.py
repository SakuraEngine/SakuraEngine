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


class ParserBase:
    def __init__(self) -> None:
        self.owner: ParserBase

    def is_path_valid(self, path_nodes: List[str]):
        raise NotImplementedError("is_path_valid is not implemented!")

    # expand_path (遇到 path)
    # dispatch_expand_path (遇到 dict)
    # expand_shorthand (遇到 shorthand)
    # dispatch_expand_shorthand (遇到 dict)
    # check_structure (遇到非 dict)
    # dispatch_check_structure (遇到 dict)


class RootParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()
        self.functional: Dict[str, List[FunctionalParser]] = {}

    def add_functional(self, name, parser) -> None:
        if name not in self.functional:
            self.functional[name] = []
        self.functional[name].append(parser)


class FunctionalParser(ParserBase):
    def __init__(self, options={}, shorthands=[]) -> None:
        super().__init__()
        self.shorthands: List[Shorthand] = shorthands
        self.options: Dict[str, ParserBase] = options

        # update owner
        for v in self.shorthands:
            v.owner = self
        for (k, v) in self.options.items():
            v.owner = self


class ValueParser(ParserBase):
    def __init__(self, owner) -> None:
        super().__init__(owner)


class BoolParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


class StrParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


class IntParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


class FloatParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


class ListParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


class DictParser(ValueParser):
    def __init__(self) -> None:
        super().__init__()


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
        if not isinstance(parser, FunctionalParser):
            raise TypeError("parser must be a FunctionalParser!")
        self.functional[target].add_functional(key, parser)
