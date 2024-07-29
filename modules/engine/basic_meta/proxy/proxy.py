import os
import re
import framework.generator as gen
import framework.scheme as sc
import framework.cpp_types as cpp
import framework.log as log
import framework.database as db
import typing as t
from dataclasses import dataclass, field


@dataclass
class RecordProxyData:
    methods: t.List[cpp.Method]


@dataclass
class MethodProxyData:
    getter: str
    setter: str


class ProxyGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "proxy": sc.Functional()
            })
        )

        # method scheme
        self.owner.add_method_scheme(
            sc.Namespace({
                "proxy": sc.Functional({
                    "getter": sc.Str(),
                    "setter": sc.Str(),
                })
            })
        )

    def solve_attrs(self):
        records = self.owner.database.main_module.get_records()

        for record in records:
            # filter proxy
            record_proxy = record.attrs["proxy"]
            if not record_proxy.is_function_enable():
                continue

            record_proxy_data = RecordProxyData(
                methods=[]
            )

            # each method
            for method in record.methods:
                # filter proxy
                method_proxy = method.attrs["proxy"]
                if not method_proxy.is_function_enable():
                    continue

                # parse getter/setter data
                method_proxy_getter = method_proxy["getter"]
                method_proxy_setter = method_proxy["setter"]
                method_proxy_data = MethodProxyData(
                    getter=method_proxy_getter.visited_or(None),
                    setter=method_proxy_setter.visited_or(None),
                )

                # save generator data
                method.generator_data["proxy"] = method_proxy_data

                # add to record data
                record_proxy_data.methods.append(method)

            # save generator data
            record.generator_data["proxy"] = record_proxy_data

    def generate_body(self):
        # load template
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        body_template = self.owner.load_template(os.path.join(script_dir, "proxy.body.mako"))

        db = self.owner.database
        for record in db.main_module.get_records():
            if "proxy" in record.generator_data:
                record.generated_body_content += body_template.render(
                    record=record
                )

    def generate(self):
        # load template
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_template = self.owner.load_template(os.path.join(script_dir, "proxy.hpp.mako"))
        source_template = self.owner.load_template(os.path.join(script_dir, "proxy.cpp.mako"))

        # load data
        main_module = self.owner.database.main_module

        # gen header
        for header_db in main_module.header_dbs:
            records = [record for record in header_db.get_records() if "proxy" in record.generator_data]
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_template.render(
                    records=records
                )
            )

        # gen source
        records = [record for record in main_module.get_records() if "proxy" in record.generator_data]
        self.owner.append_content(
            "generated.cpp",
            source_template.render(
                records=records
            )
        )


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("proxy", ProxyGenerator())
