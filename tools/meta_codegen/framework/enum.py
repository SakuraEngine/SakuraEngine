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