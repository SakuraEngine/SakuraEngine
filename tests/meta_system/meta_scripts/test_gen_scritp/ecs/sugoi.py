from framework.attr_parser import ErrorTracker, ParserManager
from framework.database import ErrorTracker, ModuleDatabase, ParserManager
from framework.generator import *
from framework.database import *
from framework.attr_parser import *
from framework.generator import *
import os
import re


@dataclass
class SugoiAccess:
    readonly: bool
    atomic: bool
    order: str
    optional: bool


class SugoiQuery(object):
    def __init__(self, literal: str):
        self.all = []
        self.any = []
        self.none = []
        self.components = []
        self.accesses = []
        self.literal = literal
        # parse literal
        pattern = re.compile(r'\s+')
        literal = re.sub(pattern, '', literal)
        parts = literal.split(",")
        for part in parts:
            # [access]
            end_pos = part.find("]")
            access = part[1:end_pos]
            part = part[end_pos+1:]
            optional = False
            # <order>
            order = "seq"
            if part[0] == "<":
                end_pos = part.find(">")
                order = part[1:end_pos]
                part = part[end_pos+1:]
            if part[0] == "!":
                cat = self.none
                part = part[1:]
            elif part[0] == "?":
                cat = None
                part = part[1:]
                optional = True
            elif part[0] == "|":
                cat = self.any
                part = part[1:]
            else:
                cat = self.all
            # type
            end_pos = part.find("@")
            if (end_pos == -1):
                type = part
            else:
                type = part[:end_pos]
            # add to list
            if access == "out":
                count = 0
            if access != "has" and cat is not self.none:
                self.components.append(type)
                acc = SugoiAccess(access == "in", access == "atomic", order, optional)
                self.accesses.append(acc)
            if cat is not None:
                cat.append(type)

    def sequence(self):
        return [(i, c) for i, c in enumerate(self.components) if self.accesses[i].order != "unseq"]

    def unsequence(self):
        return [(i, c) for i, c in enumerate(self.components) if self.accesses[i].order != "seq"]


class SugoiGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        parser_manager.add_record_parser(
            "sugoi",
            FunctionalParser(
                options={
                    "query": StrParser(),
                    "component": FunctionalParser(
                        options={
                            "pin": BoolParser(),
                            "chunk": BoolParser(),
                            "buffer": IntParser(),
                            "custom": StrParser(),
                            "unsafe": BoolParser(),
                        }
                    )
                }
            )
        )

    def generate(self, env: GenerateCodeEnv):
        return super().generate(env)
