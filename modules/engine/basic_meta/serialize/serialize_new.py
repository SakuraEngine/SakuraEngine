import os
import re
import framework.generator as gen
import framework.scheme as sc
import framework.cpp_types as cpp
import framework.log as log
import framework.database as db
import typing as t
from dataclasses import dataclass, field


class SerializeGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional(),
                    "bin": sc.Functional(),
                    "blob": sc.Functional(),
                    "net": sc.Functional({  # TODO. unused
                        "config": sc.Str(),  # 网络序列化的额外参数
                    }),
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # field scheme
        self.owner.add_field_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional({
                        "no-default": sc.Bool(),  # 指定字段在 json 反序列化时，如果不存在，强制报错而不是不处理
                    }),
                    "bin": sc.Functional(),
                    "blob": sc.Functional({
                        "arena": sc.Str(),  # field 对应的 arena
                    }),
                    "net": sc.Functional(),
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # enum scheme
        self.owner.add_enum_scheme({
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Functional(),
                    "bin": sc.Functional(),
                    "net": sc.Functional(),
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        })

    def generate_body(self):
        # Blob generate body
        return super().generate_body()

    def generate(self):
        return super().generate()
