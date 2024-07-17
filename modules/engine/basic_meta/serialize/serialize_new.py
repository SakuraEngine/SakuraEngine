import os
import re
import framework.generator as gen
import framework.scheme as sc
import framework.cpp_types as cpp
import framework.log as log
import framework.database as db
import typing as t
from dataclasses import dataclass, field
# TODO. serde::policy，利用 attribute 来控制具体序列化的细节行为


class SerializeGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional(),  # default: disable
                    "bin": sc.Functional(),   # default: disable
                    "field_default": sc.List(),  # default: follow above config
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # field scheme
        self.owner.add_field_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional(),  # default: follow record "field_default" config
                    "bin": sc.Functional(),  # default: follow record "field_default" config
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # enum scheme
        self.owner.add_enum_scheme({
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional(),  # default disable
                    "bin": sc.Functional(),  # default disable
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        })

    def generate_body(self):
        # Blob generate body
        return super().generate_body()

    def generate(self):
        return super().generate()
