'''
Parameter json structure
<name: str>: {
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
}
'''


from typing import Dict


class Parameter:
    def __init__(self) -> None:
        self.name: str
        self.type: str
        self.array_size: int
        self.raw_type: str
        self.is_functor: bool
        self.is_callback: bool
        self.is_anonymous: bool
        self.comment: str
        self.offset: int
        self.line: int
        self.generator_data: Dict[str, object]
