
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

Record json structure
<record_name: str> : {
    "bases": <bases: List[str]>,
    "attrs": <user attributes: Object>,
    "fields": <fields: Dict[Fields]>,
    "methods": <methods: List[Methods]>,
    "comment": <comment: str>,
    "fileName": <file_name: str>
    "line": <line: int>
}

Field json structure
<field_name> : {
    "type": <type: str>,
    "rawType": <raw_type: str>,
    "arraySize": <array_size: int>,
    "attrs": <user attributes: Object>,
    "isFunctor": <is_functor: bool>,
    "isAnonymous": <is_anonymous: bool>,
    "comment": <comment: str>,
    "offset": <offset: int>,
    "line": <line: int>,
}

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
            "functor": <functor: Function>
        },
        ...
    },
    "retType": <return_type: str>,
    "rawRetType": <raw_return_type: str>,
    "line": <line: int>,
}

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
            "functor": <functor: Function>
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

from typing import List, Dict
import framework.scheme as scheme


class EnumValue:
    def __init__(self, name) -> None:
        split_name = str.rsplit(name, "::", 1)
        self.name: str = name
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.value: int
        self.comment: str
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.value = unique_dict["value"]
        self.comment = unique_dict["comment"]
        self.line = unique_dict["line"]

        # load fields
        self.raw_attrs = unique_dict["attrs"]


class Enumeration:
    def __init__(self, name) -> None:
        split_name = str.rsplit(name, "::", 1)
        self.name: str = name
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.values: Dict[str, EnumValue]
        self.is_scoped: bool
        self.underlying_type: str
        self.comment: str
        self.file_name: str
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.is_scoped = unique_dict["isScoped"]
        self.underlying_type = unique_dict["underlying_type"]
        self.comment = unique_dict["comment"]
        self.file_name = unique_dict["fileName"]
        self.line = unique_dict["line"]

        # load values
        self.values = {}
        for (enum_value_name, enum_value_data) in unique_dict["values"].unique_dict().items():
            value = EnumValue(enum_value_name)
            value.load_from_raw_json(enum_value_data)
            self.values[enum_value_name] = value

        # load attrs
        self.raw_attrs = unique_dict["attrs"]


class Record:
    def __init__(self, name) -> None:
        split_name = str.rsplit(name, "::", 1)
        self.name: str = name
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.bases: List[str]
        self.fields: List[Field]
        self.methods: List[Method]
        self.file_name: str
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.bases = unique_dict["bases"]
        self.comment = unique_dict["comment"]
        self.file_name = unique_dict["fileName"]
        self.line = unique_dict["line"]

        # load fields
        self.fields = []
        for (field_name, field_data) in unique_dict["fields"].unique_dict().items():
            field = Field(field_name)
            field.load_from_raw_json(field_data)
            self.fields.append(field)

        # load methods
        self.methods = []
        for method_data in unique_dict["methods"]:
            method = Method()
            method.load_from_raw_json(method_data)
            self.methods.append(method)

        # load attrs
        self.raw_attrs = unique_dict["attrs"]


class Field:
    def __init__(self, name) -> None:
        self.name: str = name

        self.type: str
        self.raw_type: str
        self.array_size: int
        self.is_functor: bool
        self.is_anonymous: bool
        self.comment: str
        self.offset: int
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.type = unique_dict["type"]
        self.raw_type = unique_dict["rawType"]
        self.array_size = unique_dict["arraySize"]
        self.is_functor = unique_dict["isFunctor"]
        self.is_anonymous = unique_dict["isAnonymous"]
        self.comment = unique_dict["comment"]
        self.offset = unique_dict["offset"]
        self.line = unique_dict["line"]

        # load attrs
        self.raw_attrs = unique_dict["attrs"]


class Method:
    def __init__(self) -> None:
        self.name: str
        self.short_name: str
        self.namespace: str

        self.is_static: bool
        self.is_const: bool
        self.is_nothrow: bool
        self.comment: str
        self.parameters: Dict[str, Parameter]
        self.ret_type: str
        self.raw_ret_type: str
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.name = unique_dict["name"]
        split_name = str.rsplit(self.name, "::", 1)
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.is_static = unique_dict["isStatic"]
        self.is_const = unique_dict["isConst"]
        self.is_nothrow = unique_dict["isNothrow"]
        self.comment = unique_dict["comment"]
        self.ret_type = unique_dict["retType"]
        self.raw_ret_type = unique_dict["rawRetType"]
        self.line = unique_dict["line"]

        # load parameters
        self.parameters = {}
        for (param_name, param_data) in unique_dict["parameters"].unique_dict().items():
            param = Parameter(param_name)
            param.load_from_raw_json(param_data)
            self.parameters[param_name] = param

        # load attrs
        self.raw_attrs = unique_dict["attrs"]


class Parameter:
    def __init__(self, name) -> None:
        self.name: str = name
        self.type: str
        self.array_size: int
        self.raw_type: str
        self.is_functor: bool
        self.is_callback: bool
        self.is_anonymous: bool
        self.functor: 'Function'
        self.comment: str
        self.offset: int
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.type = unique_dict["type"]
        self.array_size = unique_dict["arraySize"]
        self.raw_type = unique_dict["rawType"]
        self.is_functor = unique_dict["isFunctor"]
        self.is_anonymous = unique_dict["isAnonymous"]
        self.comment = unique_dict["comment"]
        self.offset = unique_dict["offset"]
        self.line = unique_dict["line"]

        # TODO. load functor

        # load attrs
        self.raw_attrs = unique_dict["attrs"]


class Function:
    def __init__(self) -> None:
        self.name: str
        self.short_name: str
        self.namespace: str

        self.is_static: bool
        self.is_const: bool
        self.comment: str
        self.parameters: Dict[str, Parameter]
        self.ret_type: str
        self.raw_ret_type: str
        self.file_name: str
        self.line: int

        self.raw_attrs: scheme.JsonObject
        self.attrs: object

    def load_from_raw_json(self, raw_json: scheme.JsonObject):
        unique_dict = raw_json.unique_dict()

        self.name = unique_dict["name"]
        split_name = str.rsplit(self.name, "::", 1)
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.is_static = unique_dict["isStatic"]
        self.is_const = unique_dict["isConst"]
        self.comment = unique_dict["comment"]
        self.ret_type = unique_dict["retType"]
        self.raw_ret_type = unique_dict["rawRetType"]
        self.file_name = unique_dict["fileName"]
        self.line = unique_dict["line"]

        # load parameters
        self.parameters = {}
        for (param_name, param_data) in unique_dict["parameters"].unique_dict().items():
            param = Parameter(param_name)
            param.load_from_raw_json(param_data)
            self.parameters[param_name] = param

        # load attrs
        self.raw_attrs = unique_dict["attrs"]
