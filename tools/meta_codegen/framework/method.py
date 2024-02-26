'''
Method json structure
{
    "name": <name: str>,
    "isStatic": <is_static: bool>,
    "isConst": <is_const: bool>,
    "isNothrow": <is_nothrow: bool>,
    "attrs": <user attributes: Object>,
    "comment": <comment: str>,
    "parameters": {
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
        },
        ...
    },
    "retType": <return_type: str>,
    "rawRetType": <raw_return_type: str>,
    "line": <line: int>,
}
'''