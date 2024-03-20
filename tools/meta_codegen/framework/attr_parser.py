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
    def parse_to_object(self, override_solver: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        pass


class RootParser(ParserBase):
    def __init__(self) -> None:
        super().__init__()
        self.__parser_dict: Dict[str, ParserBase] = {}

    def add_parser(self, name, parser) -> None:
        if name in self.__parser_dict:
            raise Exception(f"functional name [{name}] already exists!")
        self.__parser_dict[name] = parser

    def dispatch_expand_shorthand_and_path(self, json_dict: JsonDict, error_tracker: ErrorTracker) -> None:
        json_dict.expand_path_recursive()

        def __dispatch(k, dict):
            if k in self.__parser_dict:
                self.__parser_dict[k].dispatch_expand_shorthand_and_path(dict, error_tracker)

        def __expand(k, v):
            if k in self.__parser_dict:
                return self.__parser_dict[k].expand_shorthand_and_path(v, error_tracker)

        json_dict.expand_shorthand(__expand, __dispatch, error_tracker)

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is JsonDict:
            for (k, override) in value:
                override.push_path(error_tracker)
                if k in self.__parser_dict:
                    override.mark_recognized()
                    self.__parser_dict[k].check_structure(override.val, error_tracker)
                override.pop_path(error_tracker)
        else:
            error_tracker.error("value type error, must be JsonDict!")

    def parse_to_object(self, override_solver: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        result_dict = {}
        for (k, parser) in self.__parser_dict.items():
            override_solver.push_node(k, error_tracker)
            result_dict[k] = parser.parse_to_object(override_solver, error_tracker)
            override_solver.pop_node(error_tracker)
        return ObjDictTools.as_obj(result_dict)


class FunctionalParser(ParserBase):
    def __init__(self, options: Dict[str, ParserBase] = {}, shorthands: List['Shorthand'] = [], custom_to_object=None, default_enable=False) -> None:
        super().__init__()
        self.shorthands: List[Shorthand] = shorthands
        self.options: Dict[str, ParserBase] = options
        self.custom_to_object = custom_to_object
        self.default_enable = default_enable

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
                elif k == "enable":
                    override.mark_recognized()
                    if type(override.val) is not bool:
                        error_tracker.error(f"value type error, must be bool!")
                override.pop_path(error_tracker)
        else:
            error_tracker.error(f"value type error, passed [{type(value)}]{value}, must be JsonDict!")

    def parse_to_object(self, override_solver: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        result_dict = {}
        # parse enable
        solved_enable = override_solver.solve_override("enable", error_tracker)
        result_dict["enable"] = solved_enable if solved_enable else self.default_enable or override_solver.current_has_value()

        # parse options
        for (option_key, parser) in self.options.items():
            override_solver.push_node(option_key, error_tracker)
            result_dict[option_key] = parser.parse_to_object(override_solver, error_tracker)
            override_solver.pop_node(error_tracker)

        if self.custom_to_object:
            return self.custom_to_object(result_dict, error_tracker)
        else:
            return ObjDictTools.as_obj(result_dict)


class ValueParser(ParserBase):
    def __init__(self, type, type_name, default_value, custom_to_object) -> None:
        super().__init__()
        self.type = type
        self.type_name = type_name
        self.default_value = default_value
        self.custom_to_object = custom_to_object

    def check_structure(self, value: object, error_tracker: ErrorTracker) -> None:
        if type(value) is not self.type:
            error_tracker.error(f"value type error, must be {self.type_name}!")

    def parse_to_object(self, override_solver: JsonOverrideSolver, error_tracker: ErrorTracker) -> object:
        result = override_solver.solve_override(None, error_tracker)
        if result:
            if type(result) is not self.type:
                error_tracker.error(f"value type error, must be {self.type_name}!")
            if self.custom_to_object:
                return self.custom_to_object(result, error_tracker)
            else:
                return result
        else:
            return self.default_value


class BoolParser(ValueParser):
    def __init__(self, default_value=False, custom_to_object=None) -> None:
        super().__init__(bool, "bool", default_value, custom_to_object)


class StrParser(ValueParser):
    def __init__(self, default_value="", custom_to_object=None) -> None:
        super().__init__(str, "str", default_value, custom_to_object)


class IntParser(ValueParser):
    def __init__(self, default_value=0, custom_to_object=None) -> None:
        super().__init__(int, "int", default_value, custom_to_object)


class FloatParser(ValueParser):
    def __init__(self, default_value=0.0, custom_to_object=None) -> None:
        super().__init__(float, "float", default_value, custom_to_object)


class ListParser(ValueParser):
    def __init__(self, default_value=[], custom_to_object=None) -> None:
        super().__init__(list, "list", default_value, custom_to_object)


class DictParser(ValueParser):
    def __init__(self, default_value={}, custom_to_object=None) -> None:
        super().__init__(JsonDict, "dict", default_value, custom_to_object)


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


class ParserManager:
    def __init__(self) -> None:
        self.parser_dict: Dict[FunctionalTarget, RootParser] = {
            FunctionalTarget.RECORD: RootParser(),
            FunctionalTarget.FIELD: RootParser(),
            FunctionalTarget.METHOD: RootParser(),
            FunctionalTarget.PARAMETER: RootParser(),
            FunctionalTarget.FUNCTION: RootParser(),
            FunctionalTarget.ENUM: RootParser(),
            FunctionalTarget.ENUM_VALUE: RootParser(),
        }

    def add_record_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.RECORD].add_parser(key, parser)

    def add_field_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.FIELD].add_parser(key, parser)

    def add_method_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.METHOD].add_parser(key, parser)

    def add_parameter_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.PARAMETER].add_parser(key, parser)

    def add_function_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.FUNCTION].add_parser(key, parser)

    def add_enum_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.ENUM].add_parser(key, parser)

    def add_enum_value_parser(self, key: str, parser: ParserBase) -> None:
        self.parser_dict[FunctionalTarget.ENUM_VALUE].add_parser(key, parser)
