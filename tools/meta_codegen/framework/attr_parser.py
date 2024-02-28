'''
对 RTTR 系统的试写：
builder.add_functional(
    target = FunctionalTarget.RECORD,
    name = "rttr",
    parser = FunctionalParser(
        str_abbr = StrAbbrParser(
            options_mapping = {
                "all": { reflect_fields: True, reflect_methods: True, reflect_bases: True },
            }
        ),
        list_abbr = ListAbbrParser(
            options_mapping = {
                "field": { reflect_fields: True },
                "method": { reflect_methods: True }
                "no-bases": { reflect_bases: False }
            }
        ),
        options = {
            "reflect_bases": ValueParser(bool),
            "exclude_bases": ListParser(str),
            "reflect_fields": ValueParser(bool),
            "reflect_methods": ValueParser(bool),
        }
    )
)
'''

from enum import Enum


class ParserBase:
    def __init__(self) -> None:
        self.check_override = False


class ValueParser(ParserBase):
    def __init__(self) -> None:
        self.check_override = True


class ListParser(ParserBase):
    def __init__(self) -> None:
        self.check_override = True


class DictParser(ParserBase):
    def __init__(self) -> None:
        self.check_override = True


class FunctionalParser(ParserBase):
    def __init__(self, options) -> None:
        self.check_override = False
