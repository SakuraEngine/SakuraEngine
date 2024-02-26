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

class Record(object):
    def __init__(self) -> None:
        # code info
        self.full_name:str
        self.short_name:str
        self.namespace:str
        # self.bases
        # self.fields
        # self.methods

        # source info
        self.file_name:str
        self.line:int

        # user attributes
        self.functional:Dict[str, object] = {}