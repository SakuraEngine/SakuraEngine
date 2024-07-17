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


@dataclass
class RecordSerdeData:
    enable_json: bool
    enable_bin: bool
    json_fields: t.List[cpp.Field]
    bin_fields: t.List[cpp.Field]


@dataclass
class FieldSerdeData:
    enable_json: bool
    enable_bin: bool


@dataclass
class EnumSerdeData:
    enable_json: bool


class SerdeGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Bool(),  # default: disable
                    "bin": sc.Bool(),   # default: disable
                    "field_default": sc.List(),  # default: follow above config
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # field scheme
        self.owner.add_field_scheme(
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Bool(),  # default: follow record "field_default" config
                    "bin": sc.Bool(),  # default: follow record "field_default" config
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        )

        # enum scheme
        # TODO. use EnumSerdeTraits directly?
        self.owner.add_enum_scheme({
            sc.Namespace({
                "serde": sc.Functional({
                    "json": sc.Bool(),  # default disable
                    # "bin": sc.Functional(),  # default disable
                }, shorthands=sc.FunctionalOptionShorthand())
            })
        })

    def solve_attrs(self):
        records = self.owner.database.get_records()
        enums = self.owner.database.get_enums()

        # parse record
        for record in records:
            record_serde = record.attrs["serde"]
            if not record_serde.is_function_enable():
                continue

            # parse basic data
            record_serde_json = record_serde["json"]
            record_serde_bin = record_serde["bin"]
            record_serde_data = RecordSerdeData(
                enable_json=record_serde_json.visited_or(False),
                enable_bin=record_serde_bin.visited_or(False),
                json_fields=[],
                bin_fields=[]
            )

            # parse field default config
            record_field_default = record_serde["field_default"]
            if record_field_default.is_visited():
                record_field_default_json = "json" in record_field_default.parsed_value
                record_field_default_bin = "bin" in record_field_default.parsed_value
            else:
                record_field_default_json = record_serde_data.enable_json
                record_field_default_bin = record_serde_data.enable_bin

            # parse fields
            for field in record.fields:
                field_serde = field.attrs["serde"]
                field_serde_json = field_serde["json"]
                field_serde_bin = field_serde["bin"]

                field_serde_data = FieldSerdeData(
                    enable_json=field_serde_json.visited_or(record_field_default_json),
                    enable_bin=field_serde_bin.visited_or(record_field_default_bin),
                )

                # add to record serde data
                if field_serde_data.enable_json:
                    record_serde_data.json_fields.append(field)
                if field_serde_data.enable_bin:
                    record_serde_data.bin_fields.append(field)

                # save generator data
                field.generator_data["serde"] = field_serde_data

            # save generator data
            record.generator_data["serde"] = record_serde_data

        # parse enum
        for enum in enums:
            enum_serde = record.attrs["serde"]
            if not enum_serde.is_function_enable():
                continue

            # solve basic data
            enum_serde_json = enum_serde["json"]
            enum_serde_data = EnumSerdeData(
                enable_json=enum_serde_json.visited_or(False),
            )

            # save generator data
            enum.generator_data["serde"] = enum_serde_data

    def generate(self):
        # load template
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_template = self.owner.load_template(os.path.join(script_dir, "serialize.hpp.mako"))
        source_template = self.owner.load_template(os.path.join(script_dir, "serialize.cpp.mako"))

        # load data
        main_module = self.owner.database.main_module

        # gen header
        for header_db in main_module.header_dbs:
            json_enums = [enum for enum in header_db.get_enums() if "serde" in enum.generator_data and enum.generator_data["serde"].enable_json]
            json_records = [record for record in header_db.get_records() if "serde" in record.generator_data and record.generator_data["serde"].enable_json]
            bin_records = [record for record in header_db.get_records() if "serde" in record.generator_data and record.generator_data["serde"].enable_bin]

            self.owner.append_content(
                header_db.relative_target_header_path,
                header_template.render(
                    json_enums=json_enums,
                    json_records=json_records,
                    bin_records=bin_records,
                    api=f"{main_module.api}_API",
                )
            )

        # gen source
        json_enums = [enum for enum in main_module.get_enums() if "serde" in enum.generator_data and enum.generator_data["serde"].enable_json]
        json_records = [record for record in main_module.get_records() if "serde" in record.generator_data and record.generator_data["serde"].enable_json]
        bin_records = [record for record in main_module.get_records() if "serde" in record.generator_data and record.generator_data["serde"].enable_bin]
        self.owner.append_content(
            "generated.cpp",
            source_template.render(
                json_enums=json_enums,
                json_records=json_records,
                bin_records=bin_records,
            )
        )


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("serde", SerdeGenerator())
