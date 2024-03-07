from typing import List, Dict, Tuple
from dataclasses import dataclass, field
from enum import Enum
from framework.error_tracker import ErrorTracker


class JsonOverrideMark(Enum):
    NONE = 0        # no mark
    OVERRIDE = 1    # end with mark '!'
    REWRITE = 2     # end with mark '!!'
    APPEND = 3      # end with mark '+'


@dataclass
class JsonOverrideData:
    val: object = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE


@dataclass
class JsonValue:
    overrides: List[JsonOverrideData] = field(default_factory=lambda: [])
    is_recognized: bool = False

    def add_override(self, val: object, mark: JsonOverrideMark):
        self.overrides.append(JsonOverrideData(val, mark))

    def unique_val(self) -> object:
        if len(self.overrides) != 1:
            raise Exception("Multiple overrides found!")
        return self.overrides[0].val

    def mark_recognized(self):
        self.is_recognized = True


def setup_recognized_recursive(json_value: JsonValue, recognized: bool):
    json_value.is_recognized = recognized
    for override in json_value.overrides:
        if isinstance(override.val, dict):
            for (k, v) in override.val.items():
                setup_recognized_recursive(v, recognized)


def error_unrecognized(json_value: JsonValue, error_tracker: ErrorTracker):
    if not json_value.is_recognized:
        error_tracker.error("Unrecognized json value!")
    for override in json_value.overrides:
        if isinstance(override.val, dict):
            for (k, v) in override.val.items():
                error_tracker.push_path(k)
                error_unrecognized(v, error_tracker)
                error_tracker.pop_path()


def parse_json_data(data: List[Tuple[str, object]]):
    vals: Dict[str, JsonValue] = {}

    def get_or_add(key: str) -> JsonValue:
        if key not in vals:
            vals[key] = JsonValue()
        return vals[key]

    for (k, v) in data:
        if k.endswith("!!"):
            true_key = k[:-2]
            get_or_add(true_key).add_override(v, JsonOverrideMark.REWRITE)
        elif k.endswith("!"):
            true_key = k[:-1]
            get_or_add(true_key).add_override(v, JsonOverrideMark.OVERRIDE)
        elif k.endswith("+"):
            true_key = k[:-1]
            get_or_add(true_key).add_override(v, JsonOverrideMark.APPEND)
        else:
            get_or_add(k).add_override(v, JsonOverrideMark.NONE)

    return vals


if __name__ == '__main__':
    data = '''{
        "test": {
            "a": 1,
            "b": 2
        },
        "test!": {
            "b": 114514,
            "c": 3
        },
        "test!!": {
            "a": 114514
        },
        "test": {
            "a+": "shit"
        }
    }'''
    import json
    val: Dict[str, JsonValue] = json.loads(data, object_pairs_hook=parse_json_data)

    for override in val["test"].overrides:
        print(override.val)
