from typing import List, Dict, Tuple
from dataclasses import dataclass, field
from enum import Enum
from framework.error_tracker import ErrorTracker


class JsonOverrideMark(Enum):
    NONE = 0  # no mark
    OVERRIDE = 1  # end with mark '!'
    REWRITE = 2  # end with mark '!!'
    APPEND = 3  # end with mark '+'


def parse_override(key: str) -> Tuple[str, JsonOverrideMark]:
    if key.endswith("!!"):
        return (key[:-2], JsonOverrideMark.REWRITE)
    elif key.endswith("!"):
        return (key[:-1], JsonOverrideMark.OVERRIDE)
    elif key.endswith("+"):
        return (key[:-1], JsonOverrideMark.APPEND)
    else:
        return (key, JsonOverrideMark.NONE)


def apply_override(key: str, mark: JsonOverrideMark) -> str:
    if mark == JsonOverrideMark.REWRITE:
        return key + "!!"
    elif mark == JsonOverrideMark.OVERRIDE:
        return key + "!"
    elif mark == JsonOverrideMark.APPEND:
        return key + "+"
    else:
        return key


def merge_override_mark(parent: JsonOverrideMark, child: JsonOverrideMark, error_tracker: ErrorTracker) -> JsonOverrideMark:
    if parent == JsonOverrideMark.NONE:
        return child
    if parent == JsonOverrideMark.OVERRIDE:
        if child == JsonOverrideMark.REWRITE:
            return child
        else:
            return parent
    if parent == JsonOverrideMark.REWRITE:
        if child == JsonOverrideMark.OVERRIDE:
            error_tracker.warning("override mark '!' under rewrite mark '!!' will be ignored!")
        elif child == JsonOverrideMark.APPEND:
            error_tracker.warning("append mark '+' under rewrite mark '!!' will be ignored!")
        return parent
    if parent == JsonOverrideMark.APPEND:
        if child == JsonOverrideMark.OVERRIDE:
            error_tracker.warning("override mark '!' under append mark '+' will be ignored!")
        elif child == JsonOverrideMark.REWRITE:
            error_tracker.warning("rewrite mark '!!' under append mark '+' will be ignored!")
        elif child == JsonOverrideMark.APPEND:
            error_tracker.warning("append mark '+' under append mark '+' will be ignored!")
        return parent


@dataclass
class JsonOverrideData:
    val: object = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE
    is_recognized: bool = False

    def mark_recognized(self) -> None:
        self.is_recognized = True

    def push_path(self, error_tracker: ErrorTracker, key: str) -> None:
        error_tracker.path_push(key)

    def pop_path(self, error_tracker: ErrorTracker) -> None:
        error_tracker.path_pop()


@dataclass
class JsonOverrideDataPath(JsonOverrideData):
    path_nodes: List[Tuple[str, JsonOverrideMark]] = None
    raw_value: object = None
    is_root: bool = False

    def push_path(self, error_tracker: ErrorTracker, key: str) -> None:
        if self.is_root:
            error_tracker.path_push("::".join([apply_override(k, override) for (k, override) in self.path_nodes]))

    def pop_path(self, error_tracker: ErrorTracker) -> None:
        if self.is_root:
            error_tracker.path_pop()


@dataclass
class JsonOverrideDataShorthand(JsonOverrideData):
    key: str = ""
    shorthand: object = None
    raw_value: object = None
    raw_dict: Dict = None
    is_root: bool = False


class JsonDict:
    def __init__(self) -> None:
        self.__sorted_overrides: List[Tuple[str, JsonOverrideData]] = []

    def __getitem__(self, index) -> Tuple[str, JsonOverrideData]:
        return self.__sorted_overrides[index]

    def add(self, key: str, override: JsonOverrideData) -> None:
        self.__sorted_overrides.append((key, override))

    def expand_path(self) -> int:
        index = 0
        count = 0
        while index < len(self.__sorted_overrides):
            (k, override) = self.__sorted_overrides[index]
            path_nodes = [parse_override(node) for node in k.split("::")]
            if len(path_nodes) > 1:
                # build node dict
                root_dict = JsonDict()
                cur_dict = root_dict
                node_index = 1
                while node_index < len(path_nodes):
                    (key, mark) = path_nodes[node_index]
                    if node_index == len(path_nodes) - 1:  # last node
                        cur_dict.add(key, JsonOverrideDataPath(
                            val=override.val,
                            override_mark=mark,
                            path_nodes=path_nodes,
                            raw_value=override.val,
                            is_root=False
                        ))
                    else:
                        new_dict = JsonDict()
                        cur_dict.add(key, JsonOverrideDataPath(
                            val=new_dict,
                            override_mark=mark,
                            path_nodes=path_nodes,
                            raw_value=override.val,
                            is_root=False
                        ))
                        cur_dict = new_dict
                    node_index = node_index + 1

                # replace old data
                (root_key, root_mark) = path_nodes[0]
                self.__sorted_overrides[index] = (root_key, JsonOverrideDataPath(
                    val=root_dict,
                    override_mark=root_mark,
                    path_nodes=path_nodes,
                    raw_value=override.val,
                    is_root=True
                ))

                count = count + 1
            index = index + 1
        return count

    def expand_path_recursive(self) -> int:
        count = self.expand_path()
        for (_, override) in self.__sorted_overrides:
            if isinstance(override.val, JsonDict):
                count = count + override.val.expand_path_recursive()
        return count

    def expand_shorthand(self, expand_shorthand, dispatch_expand) -> None:
        index = 0
        while index < len(self.__sorted_overrides):
            (k, override) = self.__sorted_overrides[index]
            if isinstance(override.val, dict):
                dispatch_expand(k, override.val)
            else:
                mapping = expand_shorthand(k, override.val)
                if mapping:
                    # expand to new dict
                    json_dict = JsonDict()
                    json_dict.load_from_dict(mapping)
                    json_dict.expand_path_recursive()
                    dispatch_expand(k, json_dict)

                    # replace shorthand
                    self.__sorted_overrides[index] = (k, JsonOverrideData(
                        val=json_dict,
                        override_mark=override.override_mark
                    ))

            index = index + 1

    def warning_recognized_attr_recursive(self, error_tracker: ErrorTracker) -> None:
        for (key, override) in self.__sorted_overrides:
            override.push_path(error_tracker, key)
            if override.is_recognized:
                if type(override.val) is JsonDict:
                    override.val.warning_recognized_attr_recursive(error_tracker)
            else:
                error_tracker.warning(f"unrecognized attribute!!!")
            override.pop_path(error_tracker)

    def load_from_dict(self, data: Dict[str, object]) -> None:
        for (k, v) in data.items():
            key, mark = parse_override(k)
            if v is dict:
                json_dict = JsonDict()
                json_dict.load_from_dict(v)
                self.add(key, JsonOverrideData(
                    val=json_dict,
                    override_mark=mark
                ))
            else:
                self.add(key, JsonOverrideData(
                    val=v,
                    override_mark=mark
                ))

    def unique_dict(self) -> Dict[str, object]:
        result = {}
        for (k, override) in self.__sorted_overrides:
            if k in result:
                raise Exception(f"Key {k} already exists in dict!")
            result[k] = override.val
        return result


def parse_json_hook(data: List[Tuple[str, object]]) -> JsonDict:
    result = JsonDict()

    for (k, v) in data:
        key, mark = parse_override(k)
        result.add(key, JsonOverrideData(
            val=v,
            override_mark=mark
        ))

    return result


class ObjDictTools:
    @staticmethod
    def as_obj(dict: Dict) -> object:
        result = ObjDictTools()
        result.__dict__ = dict
        return result

    @staticmethod
    def as_dict(obj: object) -> Dict:
        return obj.__dict__

    @staticmethod
    def to_obj(dict: Dict) -> object:
        result = ObjDictTools()
        result.__dict__.update(dict)
        return result

    @staticmethod
    def to_dict(obj: object) -> Dict:
        return obj.__dict__.copy()


class JsonOverrideSolver:
    class OverrideData:
        # TODO. override data 拼接逻辑可以写在这里
        def __init__(self, override: JsonOverrideData, error_tracker: ErrorTracker, parent: 'JsonOverrideSolver.OverrideData' = None) -> None:
            self.override: JsonOverrideData = override
            self.passed_override_mark: JsonOverrideMark
            self.parent: 'JsonOverrideSolver.OverrideData' = parent
            self.ignored_by: 'JsonOverrideSolver.OverrideData' = None

            if parent:
                if parent.passed_override_mark == JsonOverrideMark.APPEND:
                    error_tracker.error("cannot solve value override under append mark '+'!")
                self.passed_override_mark = merge_override_mark(parent.passed_override_mark, override.override_mark, error_tracker)
                if parent.ignored_by:
                    self.ignored_by = parent.ignored_by
            else:
                self.passed_override_mark = override.override_mark

    @dataclass
    class Node:
        key: str = None
        override_data: List['JsonOverrideSolver.OverrideData'] = field(default_factory=lambda: [])

    def __init__(self) -> None:
        self.__node_stack: List[JsonOverrideSolver.Node] = []
        self.root_dict: JsonDict = None

    def push_node(self, key: str, error_tracker: ErrorTracker) -> None:
        # add node
        last_node = self.__node_stack[-1] if len(self.__node_stack) else None
        new_node = JsonOverrideSolver.Node(key=key)
        self.__node_stack.append(new_node)

        # expand override
        if not last_node:  # root node case
            for (k, override) in self.root_dict:
                if k == key:
                    new_node.override_data.append(JsonOverrideSolver.OverrideData(
                        override=override,
                        error_tracker=error_tracker,
                    ))
        else:  # usual case
            for parent_override in last_node.override_data:
                if isinstance(parent_override.override.val, JsonDict):
                    for (k, override) in parent_override.override.val:
                        if k == key:
                            # pass override mark
                            new_node.override_data.append(JsonOverrideSolver.OverrideData(
                                override=override,
                                error_tracker=error_tracker,
                                parent=parent_override,
                            ))

        # solve ignored
        index = len(new_node.override_data) - 1
        while index >= 0:
            override = new_node.override_data[index]
            if override.passed_override_mark == JsonOverrideMark.REWRITE:
                ignore_index = index - 1
                while ignore_index >= 0:
                    new_node.override_data[ignore_index].ignored_by = override
                    ignore_index = ignore_index - 1
                break
            index = index - 1

    def pop_node(self, error_tracker: ErrorTracker) -> None:
        self.__node_stack.pop()

    def solve_override(self, key: str, error_tracker: ErrorTracker) -> object:
        # push node
        if key:
            self.push_node(key, error_tracker)
        print(f"{self.__node_stack[-1].key}::{key}")

        # solve override
        cur_value = None
        cur_mark: JsonOverrideMark = JsonOverrideMark.NONE
        cur_ignored_by: JsonOverrideSolver.OverrideData = None
        for override_data in self.__node_stack[-1].override_data:
            if override_data.passed_override_mark == JsonOverrideMark.NONE and cur_value:  # bad override
                error_tracker.error(f"override value without '!' mark")
            elif override_data.passed_override_mark == JsonOverrideMark.APPEND:  # append case
                merge_source = None if cur_ignored_by else cur_value
                # make sure cur_value is list
                if type(merge_source) is list:
                    merge_source = merge_source.copy()
                elif not merge_source:
                    merge_source = []
                else:
                    merge_source = [merge_source]

                # append
                if type(override_data.override.val) is list:
                    merge_source.extend(override_data.override.val)
                else:
                    merge_source.append(override_data.override.val)

                # update mark
                cur_value = merge_source
                cur_mark = override_data.override.override_mark
                cur_ignored_by = override_data.ignored_by
            else:  # update current value
                cur_value = override_data.override.val
                cur_mark = override_data.override.override_mark
                cur_ignored_by = override_data.ignored_by

        # pop node
        if key:
            self.pop_node(error_tracker)

        # result
        if cur_ignored_by:
            return None
        else:
            return cur_value
