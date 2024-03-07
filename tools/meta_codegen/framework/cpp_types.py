
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
from framework.json_parser import JsonValue, JsonOverrideData, JsonOverrideMark


class EnumValue:
    def __init__(self, name) -> None:
        split_name = str.rsplit(name, "::", 1)
        self.name: str = name
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""

        self.value: int
        self.comment: str
        self.line: int
        self.generator_data: Dict[str, object]
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.value = raw_json["value"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load fields
        self.raw_attrs = raw_json["attrs"].unique_val()


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
        self.generator_data: Dict[str, object]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.is_scoped = raw_json["isScoped"].unique_val()
        self.underlying_type = raw_json["underlying_type"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.file_name = raw_json["fileName"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load values
        self.values = {}
        for (k, v) in raw_json["values"].unique_val().items():
            value = EnumValue(k)
            value.load_from_raw_json(v.unique_val())
            self.values[k] = value

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()


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
        self.generator_data: Dict[str, object]
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.bases = raw_json["bases"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.file_name = raw_json["fileName"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load fields
        self.fields = []
        for (k, v) in raw_json["fields"].unique_val().items():
            field = Field(k)
            field.load_from_raw_json(v.unique_val())
            self.fields.append(field)

        # load methods
        self.methods = []
        for v in raw_json["methods"].unique_val():
            method = Method()
            method.load_from_raw_json(v)
            self.methods.append(method)

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()


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
        self.generator_data: Dict[str, object]
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.type = raw_json["type"].unique_val()
        self.raw_type = raw_json["rawType"].unique_val()
        self.array_size = raw_json["arraySize"].unique_val()
        self.is_functor = raw_json["isFunctor"].unique_val()
        self.is_anonymous = raw_json["isAnonymous"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.offset = raw_json["offset"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()


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
        self.generator_data: Dict[str, object]
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.name = raw_json["name"].unique_val()
        split_name = str.rsplit(self.name, "::", 1)
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""
        self.is_static = raw_json["isStatic"].unique_val()
        self.is_const = raw_json["isConst"].unique_val()
        self.is_nothrow = raw_json["isNothrow"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.ret_type = raw_json["retType"].unique_val()
        self.raw_ret_type = raw_json["rawRetType"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load parameters
        self.parameters = {}
        for (k, v) in raw_json["parameters"].unique_val().items():
            param = Parameter(k)
            param.load_from_raw_json(v.unique_val())
            self.parameters[k] = param

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()


class Parameter:
    def __init__(self, name) -> None:
        self.name: str = name
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
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.type = raw_json["type"].unique_val()
        self.array_size = raw_json["arraySize"].unique_val()
        self.raw_type = raw_json["rawType"].unique_val()
        self.is_functor = raw_json["isFunctor"].unique_val()
        self.is_anonymous = raw_json["isAnonymous"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.offset = raw_json["offset"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()


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
        self.generator_data: Dict[str, object]
        self.raw_attrs: Dict[str, JsonValue]

    def load_from_raw_json(self, raw_json: Dict[str, JsonValue]):
        # load basic data
        self.name = raw_json["name"].unique_val()
        split_name = str.rsplit(self.name, "::", 1)
        self.short_name: str = split_name[-1]
        self.namespace: str = split_name[0] if len(split_name) > 1 else ""
        self.is_static = raw_json["isStatic"].unique_val()
        self.is_const = raw_json["isConst"].unique_val()
        self.comment = raw_json["comment"].unique_val()
        self.ret_type = raw_json["retType"].unique_val()
        self.raw_ret_type = raw_json["rawRetType"].unique_val()
        self.file_name = raw_json["fileName"].unique_val()
        self.line = raw_json["line"].unique_val()

        # load parameters
        self.parameters = {}
        for (k, v) in raw_json["parameters"].unique_val().items():
            param = Parameter(k)
            param.load_from_raw_json(v.unique_val())
            self.parameters[k] = param

        # load attrs
        self.raw_attrs = raw_json["attrs"].unique_val()
