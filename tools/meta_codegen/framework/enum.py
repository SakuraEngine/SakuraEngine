'''
Enum json structure
<enum_name: str> : {
    "attrs": <user attributes: Object>,
    "values": {
        <enum_value_name> : {
            "attrs": <user attributes: Object>,
            "value": <value: int>,
            "comment": <comment: str>,
            "line": <line: int>,
        },
        ...
    },
    "isScoped": <is_scoped: bool>,
    "underlyingType": <underlying_type: str>,
    "comment": <comment: str>,
    "fileName": <file_name: str>
    "line": <line: int>
}
'''

from typing import List
from typing import Dict


class EnumValue:
    def __init__(self) -> None:
        self.name: str
        self.value: int
        self.comment: str
        self.line: int


class Enum:
    def __init__(self) -> None:
        self.name: str
        self.values: Dict[str, EnumValue]
        self.is_scoped: bool
        self.underlying_type: str
        self.comment: str
        self.file_name: str
        self.line: int
        self.generator_data: Dict[str, object]
