from typing import List, Dict, Tuple
from dataclasses import dataclass
from enum import Enum
from framework.error_tracker import ErrorTracker
from framework.json_parser import *
from framework.json_parser import JsonDict, JsonOverrideSolver


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

    # TODO. to object 的前后顺序与 cpp 的父子关系保持一致，并给给出父信息
    def parse_to_object(self, override_solve: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        pass


class RootParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()
        self.__functional_dict: Dict[str, FunctionalParser] = {}

    def add_functional(self, name, parser) -> None:
        if name in self.__functional_dict:
            raise Exception("functional name already exists!")
        self.__functional_dict[name] = parser

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        json_dict.expand_path_recursive()

        def __dispatch(k, dict):
            if k in self.__functional_dict:
                self.__functional_dict[k].dispatch_expand_shorthand_and_path(dict, error_tracker)

        def __expand(k, v):
            if k in self.__functional_dict:
                return self.__functional_dict[k].expand_shorthand_and_path(v, error_tracker)

        json_dict.expand_shorthand(__expand, __dispatch, error_tracker)

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is JsonDict:
            for (k, override) in value:
                override.push_path(error_tracker)
                if k in self.__functional_dict:
                    override.mark_recognized()
                    self.__functional_dict[k].check_structure(override.val, error_tracker)
                override.pop_path(error_tracker)
        else:
            error_tracker.error("value type error, must be JsonDict!")

    def parse_to_object(self, override_solve: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        for (k, functional) in self.__functional_dict.items():
            override_solve.push_node(k, error_tracker)
            functional.parse_to_object(override_solve, error_tracker)
            override_solve.pop_node(error_tracker)


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
        # expand enable
        if type(shorthand) is bool:
            return {"enable": shorthand}

        # expand shorthands
        for shorthand_parser in self.shorthands:
            mapping = shorthand_parser.expand_shorthand(shorthand, error_tracker)
            if mapping:  # TODO. 支持多 mapping 合并映射
                return mapping

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        def __dispatch(k, dict):
            if k in self.options:
                self.options[k].dispatch_expand_shorthand_and_path(dict, error_tracker)

        def __expand(k, v):
            if k in self.options:
                return self.options[k].expand_shorthand_and_path(v, error_tracker)

        json_dict.expand_shorthand(__expand, __dispatch, error_tracker)

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is JsonDict:
            for (k, override) in value:
                override.push_path(error_tracker)
                if k in self.options:
                    override.mark_recognized()
                    self.options[k].check_structure(override.val, error_tracker)
                override.pop_path(error_tracker)
        else:
            error_tracker.error(f"value type error, passed [{type(value)}]{value}, must be JsonDict!")

    def parse_to_object(self, override_solve: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        result_dict = {}
        # parse enable
        result_dict["enable"] = override_solve.solve_override("enable", error_tracker)

        # parse options
        for (option_key, parser) in self.options.items():
            override_solve.push_node(option_key, error_tracker)
            result_dict[option_key] = parser.parse_to_object(override_solve, error_tracker)
            override_solve.pop_node(error_tracker)

        return ObjDictTools.as_obj(result_dict)


class ValueParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()

    def parse_to_object(self, override_solve: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        return override_solve.solve_override(None, error_tracker)


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
    def __init__(self) -> None:
        self.owner: FunctionalParser

    def expand_shorthand(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        raise NotImplementedError("expand_shorthand is not implemented!")


class StrShorthand(Shorthand):
    def __init__(self, options_mapping: Dict[str, Dict]) -> None:
        super().__init__()
        self.options_mapping: Dict[str, Dict] = options_mapping

    def expand_shorthand(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        if type(shorthand) is str:
            if shorthand in self.options_mapping:
                return self.options_mapping[shorthand]

        return None


class ListShorthand(Shorthand):
    def __init__(self, options_mapping: Dict[str, Dict]) -> None:
        super().__init__()
        self.options_mapping: Dict[str, Dict] = options_mapping

    def expand_shorthand(self, shorthand: object, error_tracker: ErrorTracker) -> Dict:
        if type(shorthand) is list:
            result = {}
            for v in shorthand:
                if v in self.options_mapping:
                    result.update(self.options_mapping[v])
            return result

        return None


class FunctionalTarget(Enum):
    RECORD = 1
    FIELD = 2
    METHOD = 3
    PARAMETER = 4
    FUNCTION = 5
    ENUM = 6
    ENUM_VALUE = 7

# TODO. functional 原则上不能同时存在多个，但是可以加入扩展功能，通常情况下关闭扩展，需要再打开
# TODO. ListShorthand 可能需要由多个 Generator 提供 expand，可能也需要提供扩展选项？
# TODO. Functional 之间应当互不干扰（比如 default_value 的解析）


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
