'''
Function json structure
{
    "name": <name: str>,
    "isStatic": <is_static: bool>,
    "isConst": <is_const: bool>,
    "attrs": <user attributes: Object>,
    "comment": <comment: str>,
    "parameters": {
        <parameter_name> : {
            "type": <type: str>,
            "arraySize": <array_size: int>,
            "rawType": <raw_type: str>,
            "attrs": <user attributes: Object>,
            "isFunctor": <is_functor: bool>,
            "isCallback": <is_callback: bool>,
            "isAnonymous": <is_anonymous: bool>,
            "comment": <comment: str>,
            "offset": <offset: int>,
            "line": <line: int>,
        },
        ...
    },
    "retType": <return_type: str>,
    "rawRetType": <raw_return_type: str>,
    "fileName": <file_name: str>
    "line": <line: int>
    }
}
'''

from typing import Dict
from typing import List
from parameter import Parameter


class Function:
    def __init__(self) -> None:
        self.name: str
        self.is_static: bool
        self.is_const: bool
        self.comment: str
        self.parameters: Dict[str, Parameter]
        self.ret_type: str
        self.raw_ret_type: str
        self.file_name: str
        self.line: int
        self.generator_data: Dict[str, object]
