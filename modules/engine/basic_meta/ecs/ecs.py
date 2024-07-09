import os
import re
import framework.generator as gen
import framework.scheme as sc
import framework.cpp_types as cpp
import typing as t
from dataclasses import dataclass, field


@enumerate
class EQueryAccess:
    IN = 0      # mark [in]      readonly = true
    OUT = 1     # mark [out]     readonly = false; phase = 0
    INOUT = 2   # mark [inout]   readonly = false
    ATOMIC = 3  # mark [atomic]  readonly = false; atomic = true
    HAS = 4     # mark [has]     filterOnly = true


@enumerate
class EQuerySeq:
    PAR = 0    # mark <par>
    SEQ = 1    # mark <seq>
    UNSEQ = 2  # mark <unseq>


@enumerate
class EQueryFilter:
    ALL = 0       # no mark
    ANY = 1       # mark "|"
    NONE = 2      # mark "!"
    OPTIONAL = 3  # mark "?"
    SHARED = 4    # mark "$"


@dataclass
class QueryComponent:
    type: str
    access: EQueryAccess
    seq: EQuerySeq
    filter: EQueryFilter


# TODO. bad design
@dataclass
class QueryAccess:
    readonly: bool  # true if access is [in]
    atomic: bool    # true if access is [atomic]
    order: str      # EQuerySeq
    optional: bool  # true if access is [has]


@dataclass
class Query:
    # extracted components
    all: t.List[str] = field(default_factory=lambda: [])
    none: t.List[str] = field(default_factory=lambda: [])
    all_shared: t.List[str] = field(default_factory=lambda: [])
    none_shared: t.List[str] = field(default_factory=lambda: [])
    any: t.List[str] = field(default_factory=lambda: [])

    # components
    components: t.List[QueryComponent] = field(default_factory=lambda: [])

    # raw literal
    raw_literal: str = ""

    def parse(self, literal: str):
        # remove empty chars
        literal = re.sub(re.compile(r"\s+"), "", literal)

        # split literal by ','
        literal_parts = literal.split(",")

        # parse each part
        for part in literal_parts:
            # parse access

            # parse order

            # parse filter

            # parse component

            pass

    def sequence(self):
        return [(idx, comp) for idx, comp in enumerate(self.components) if self.accesses[idx].order != "unseq"]

    def unsequence(self):
        return [(idx, comp) for idx, comp in enumerate(self.components) if self.accesses[idx].order != "seq"]


@dataclass
class ECSCompData:
    flags: t.List[str] = []
    array: int = 0
    custom: str = ""


@dataclass
class ECSQueryData:
    query: str = ""


class ECSGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        sc.Namespace({
            "ecs": sc.Namespace({
                "comp": sc.Functional({
                    "flags": sc.List(),  # SUGOI_TYPE_FLAG_XXXX
                    "array": sc.Int(),  # array component
                    "custom": sc.Str(),  # custom logic when register component
                }),
                "query": sc.Str(),  # query expression
            })
        })

    def solve_attrs(self):
        return super().solve_attrs()

    def generate_body(self):
        return super().generate_body()

    def generate(self):
        return super().generate()
