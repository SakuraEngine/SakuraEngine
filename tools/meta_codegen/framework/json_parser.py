from typing import List, Dict, Tuple
from dataclasses import dataclass, field
from enum import Enum
from framework.error_tracker import ErrorTracker


class JsonOverrideMark(Enum):
    NONE = 0        # no mark
    OVERRIDE = 1    # end with mark '!'
    REWRITE = 2     # end with mark '!!'
    APPEND = 3      # end with mark '+'


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


@dataclass
class JsonOverrideData:
    val: object = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE
    is_recognized: bool = False

    def push_path(self, error_tracker: ErrorTracker, key: str) -> None:
        error_tracker.push_path(key)

    def pop_path(self, error_tracker: ErrorTracker) -> None:
        error_tracker.pop_path()


@dataclass
class JsonOverrideDataPath(JsonOverrideData):
    path_nodes: List[Tuple[str, JsonOverrideMark]] = None
    raw_value: object = None
    is_root: bool = False

    def push_path(self, error_tracker: ErrorTracker, key: str) -> None:
        if self.is_root:
            error_tracker.push_path("::".join([apply_override(k, override) for (k, override) in self.path_nodes]))

    def pop_path(self, error_tracker: ErrorTracker) -> None:
        if self.is_root:
            error_tracker.pop_path()


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


@dataclass
class JsonListItem:
    val: object = None
    recognized: bool = False


class JsonList:
    def __init__(self) -> None:
        self.__override_list: List[JsonListItem] = []

    def __getitem__(self, index: int) -> JsonListItem:
        return self.__override_list[index].val

    def list(self) -> List[JsonListItem]:
        return self.__override_list


def parse_json_hook(data: List[Tuple[str, object]]) -> JsonDict:
    result = JsonDict()

    for (k, v) in data:
        key, mark = parse_override(k)
        result.add(key, JsonOverrideData(
            val=v,
            override_mark=mark
        ))

    return result
