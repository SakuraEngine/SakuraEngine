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


@dataclass
class JsonOverrideData:
    val: object = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE
    is_recognized: bool = False


@dataclass
class JsonOverrideDataPath(JsonOverrideData):
    path_nodes: List[Tuple[str, JsonOverrideMark]] = None
    raw_value: object = None
    is_root: bool = False


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

    def add(self, key: str, override: JsonOverrideData) -> None:
        self.__sorted_overrides.append((key, override))

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
        # path_nodes = [parse_override(node) for node in k.split("::")]
        # if len(path_nodes) > 1:  # path case
        #     cur_dict: JsonDict = result
        #     index = 0
        #     while index < len(path_nodes):
        #         (key, mark) = path_nodes[index]
        #         if index == len(path_nodes) - 1:  # last node
        #             cur_dict.add(key, JsonOverrideDataPath(
        #                 val=v,
        #                 override_mark=mark,
        #                 path_nodes=path_nodes,
        #                 raw_value=v,
        #                 is_root=True if cur_dict is result else False
        #             ))
        #         else:  # path node
        #             new_dict = JsonDict()
        #             cur_dict.add(key, JsonOverrideDataPath(
        #                 val=new_dict,
        #                 override_mark=mark,
        #                 path_nodes=path_nodes,
        #                 raw_value=v,
        #                 is_root=True if cur_dict is result else False
        #             ))
        #             cur_dict = new_dict
        #         index = index + 1

        key, mark = parse_override(k)
        result.add(key, JsonOverrideData(
            val=v,
            override_mark=mark
        ))

    return result
