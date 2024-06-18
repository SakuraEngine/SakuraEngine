import os
import framework.generator as gen
import framework.scheme as sc


class RTTRGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record guid scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "guid": sc.Str()
            })
        )

        # enum guid scheme
        self.owner.add_enum_scheme(
            sc.Namespace({
                "guid": sc.Str()
            })
        )

        # record rttr scheme
        self.owner.add_record_scheme(
            sc.Namespace({
                "rttr": sc.Functional({
                    "reflect_bases": sc.Bool(),  # default: True
                    "exclude_bases": sc.List(),  # default: []
                    "reflect_fields": sc.Bool(),  # default: False
                    "reflect_methods": sc.Bool(),  # default: False
                    "flags": sc.List(),
                    "attrs": sc.List(),
                }, shorthands=[sc.OptionShorthand({
                    "full": {
                        "reflect_bases": True,
                        "reflect_fields": True,
                        "reflect_methods": True,
                    },
                    "fields": {"reflect_fields": True},
                    "methods": {"reflect_methods": True},
                    "minimal": {"reflect_bases": True},
                })])
            })
        )

        # method rttr scheme
        self.owner.add_method_scheme(
            sc.Namespace({
                "rttr": sc.Functional()
            })
        )

        # field rttr scheme
        self.owner.add_field_scheme(
            sc.Namespace({
                "rttr": sc.Functional()
            })
        )

        # enum rttr scheme
        self.owner.add_enum_scheme(
            sc.Namespace({
                "rttr": sc.Functional()
            })
        )

    def load_attrs(self):
        records = self.owner.database.get_records()
        enums = self.owner.database.get_enums()

        # parse record
        for record in records:
            # parse guid
            guid = record.attrs["guid"]
            if guid.is_visited():
                record.generator_data["guid"] = guid.parsed_value

            # parse rttr
            rttr = record.attrs["rttr"]
            rttr_enable = rttr.is_function_enable(guid.is_visited(), guid.is_visited())
            if rttr_enable:
                reflect_bases = rttr["reflect_bases"] if rttr["reflect_bases"].is_visited() else True
                exclude_bases = rttr["exclude_bases"] if rttr["exclude_bases"].is_visited() else []
                reflect_fields = rttr["reflect_fields"] if rttr["reflect_fields"].is_visited() else False
                reflect_methods = rttr["reflect_methods"] if rttr["reflect_methods"].is_visited() else False
                flags = rttr["flags"] if rttr["flags"].is_visited() else []
                attrs = rttr["attrs"] if rttr["attrs"].is_visited() else []

                # solve flags

                # TODO. solve attrs

                # parse methods
                rttr_methods = []
                for method in record.methods:
                    method_rttr = method.attrs["rttr"]
                    method_rttr_enable = method_rttr.is_function_enable(reflect_methods, reflect_methods)
                    if method_rttr_enable:
                        method.generator_data["rttr"] = {}
                        rttr_methods.append(method)

                # parse fields
                rttr_fields = []
                for field in record.fields:
                    field_rttr = field.attrs["rttr"]
                    field_rttr_enable = field_rttr.is_function_enable(reflect_fields, reflect_fields)
                    if field_rttr_enable:
                        field.generator_data["rttr"] = {}
                        rttr_fields.append(field)

                record.generator_data["rttr"] = {
                    "reflect_bases": reflect_bases,
                    "exclude_bases": exclude_bases,
                    "reflect_fields": reflect_fields,
                    "reflect_methods": reflect_methods,
                    "rttr_methods": rttr_methods,
                    "rttr_fields": rttr_fields
                }

        # parse enum
        for enum in enums:
            # parse guid
            guid = enum.attrs["guid"]
            if guid.is_visited():
                enum.generator_data["guid"] = guid.parsed_value

            # parse rttr
            rttr = enum.attrs["rttr"]
            rttr_enable = rttr.is_function_enable(guid.is_visited(), guid.is_visited())
            if rttr_enable:
                enum.generator_data["rttr"] = {}

    def generate_body(self):
        # TODO. body generate
        pass

    def generate(self):
        # load template
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_template = self.owner.load_template(os.path.join(script_dir, "rttr.hpp.mako"))
        source_template = self.owner.load_template(os.path.join(script_dir, "rttr.cpp.mako"))

        # load data
        main_module = self.owner.database.main_module

        # gen header
        for header_db in main_module.header_dbs:
            enums = [enum for enum in header_db.get_enums() if "rttr" in enum.generator_data]
            records = [record for record in header_db.get_records() if "rttr" in record.generator_data]
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_template.render(enums=enums, records=records, api=f"{main_module.api}_API")
            )

        # gen source
        enums = [enum for enum in main_module.get_enums() if "rttr" in enum.generator_data]
        records = [record for record in main_module.get_records() if "rttr" in record.generator_data]
        self.owner.append_content(
            "generated.cpp",
            source_template.render(enums=enums, records=records)
        )


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("rttr", RTTRGenerator())
