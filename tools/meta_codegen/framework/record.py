'''
Record json structure
<record_name: str> : {
    "bases": <bases: List[str]>,
    "attrs": <user attributes: Object>,
    "fields": <fields: Object>,
    "methods": <methods: Object>,
    "comment": <comment: str>,
    "fileName": <file_name: str>
    "line": <line: int>
}
'''

from typing import List
from typing import Dict
from method import Method
from field import Field


class Record(object):
    def __init__(self) -> None:
        # code info
        self.name: str
        self.short_name: str
        self.namespace: str
        self.bases: List[str]
        self.fields: List[Field]
        self.methods: List[Method]

        # source info
        self.file_name: str
        self.line: int

        self.generator_data: Dict[str, object]
