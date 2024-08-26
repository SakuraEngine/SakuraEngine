import os
import framework.generator as gen
import framework.scheme as sc
import framework.cpp_types as cpp
import typing as t
from dataclasses import dataclass

# TODO. based & method & field expand 功能，展平所有基类的方法和字段，加速反射和脚本接入的性能


@dataclass
class ParamData:
    flags: t.List[str]
    attrs: t.List[str]


@dataclass
class MethodData:
    flags: t.List[str]
    attrs: t.List[str]


@dataclass
class FieldData:
    flags: t.List[str]
    attrs: t.List[str]


@dataclass
class RecordData:
    reflect_bases: t.List[str]
    reflect_fields: t.List[cpp.Field]
    reflect_methods: t.List[cpp.Method]

    flags: t.List[str]
    attrs: t.List[str]


@dataclass
class EnumItemData:
    flags: t.List[str]
    attrs: t.List[str]


@dataclass
class EnumData:
    flags: t.List[str]
    attrs: t.List[str]


class CodegenTools:
    def flag_enum_name_of(self, cpp_type) -> str:
        if type(cpp_type) is cpp.EnumerationValue:
            return "EEnumItemFlag"
        elif type(cpp_type) is cpp.Enumeration:
            return "EEnumFlag"
        elif type(cpp_type) is cpp.Record:
            return "ERecordFlag"
        elif type(cpp_type) is cpp.Method:
            return "EStaticMethodFlag" if cpp_type.is_static else "EMethodFlag"
        elif type(cpp_type) is cpp.Field:
            return "EStaticFieldFlag" if cpp_type.is_static else "EFieldFlag"
        elif type(cpp_type) is cpp.Parameter:
            return "EParamFlag"
        else:
            raise ValueError(f"Unknown cpp type: {cpp_type}")

    def flags_expr(self, cpp_type, flags: t.List[str]) -> str:
        return " | ".join(f"{self.flag_enum_name_of(cpp_type)}::{flag}" for flag in flags)

    def function_signature_of(self, method) -> str:
        if method.is_static:
            return f"{method.ret_type}(*)({', '.join(f'{param.type}' for param in method.parameters.values())})"
        else:
            return f"{method.ret_type}({method.parent.name}::*)({', '.join(f'{param.type}' for param in method.parameters.values())})"


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
                "rttr": sc.Functional({
                    "flags": sc.List(),
                    "attrs": sc.List(),
                })
            })
        )

        # field rttr scheme
        self.owner.add_field_scheme(
            sc.Namespace({
                "rttr": sc.Functional({
                    "flags": sc.List(),
                    "attrs": sc.List(),
                })
            })
        )

        # param rttr scheme
        self.owner.add_parameter_scheme(
            sc.Namespace({
                "rttr": sc.Functional({
                    "flags": sc.List(),
                    "attrs": sc.List(),
                })
            })
        )

        # enum rttr scheme
        self.owner.add_enum_scheme(
            sc.Namespace({
                "rttr": sc.Functional({
                    "flags": sc.List(),
                    "attrs": sc.List(),
                })
            })
        )

        # enum value rttr scheme
        self.owner.add_enum_value_scheme(
            sc.Namespace({
                "rttr": sc.Functional({
                    "flags": sc.List(),
                    "attrs": sc.List(),
                })
            })
        )

    def solve_attrs(self):
        records = self.owner.database.get_records()
        enums = self.owner.database.get_enums()

        # parse record
        for record in records:
            # parse guid
            record_guid = record.attrs["guid"]
            if record_guid.is_visited():
                record.generator_data["guid"] = record_guid.parsed_value

            # parse rttr
            record_rttr = record.attrs["rttr"]
            record_rttr_enable = record_rttr.is_function_enable(record_guid.is_visited(), True)
            if record_rttr_enable:
                # append generator data
                record_data = RecordData(
                    reflect_bases=[],
                    reflect_fields=[],
                    reflect_methods=[],
                    flags=record_rttr["flags"].visited_or([]),
                    attrs=record_rttr["attrs"].visited_or([])
                )
                record.generator_data["rttr"] = record_data

                # parse config
                reflect_bases = record_rttr["reflect_bases"].visited_or(True)
                exclude_bases = record_rttr["exclude_bases"].visited_or([])
                default_reflect_fields = record_rttr["reflect_fields"].visited_or(False)
                default_reflect_methods = record_rttr["reflect_methods"].visited_or(False)

                # parse bases
                if reflect_bases:
                    for base in record.bases:
                        if base not in exclude_bases:
                            record_data.reflect_bases.append(base)

                # parse methods
                for method in record.methods:
                    method_rttr = method.attrs["rttr"]
                    method_rttr_enable = method_rttr.is_function_enable(default_reflect_methods, True)
                    if method_rttr_enable:
                        record_data.reflect_methods.append(method)

                        # append generator data
                        method.generator_data["rttr"] = MethodData(
                            flags=method_rttr["flags"].visited_or([]),
                            attrs=method_rttr["attrs"].visited_or([]),
                        )

                        # parse params
                        for param in method.parameters.values():
                            param_rttr = param.attrs["rttr"]
                            method.generator_data["rttr"] = ParamData(
                                flags=param_rttr["flags"].visited_or([]),
                                attrs=param_rttr["attrs"].visited_or([]),
                            )

                # parse fields
                for field in record.fields:
                    field_rttr = field.attrs["rttr"]
                    field_rttr_enable = field_rttr.is_function_enable(default_reflect_fields, True)
                    if field_rttr_enable:
                        record_data.reflect_fields.append(field)

                        # append generator data
                        field.generator_data["rttr"] = FieldData(
                            flags=field_rttr["flags"].visited_or([]),
                            attrs=field_rttr["attrs"].visited_or([]),
                        )

        # parse enum
        for enum in enums:
            # parse guid
            enum_guid = enum.attrs["guid"]
            if enum_guid.is_visited():
                enum.generator_data["guid"] = enum_guid.parsed_value

            # parse rttr
            enum_rttr = enum.attrs["rttr"]
            enum_rttr_enable = enum_rttr.is_function_enable(enum_guid.is_visited(), True)
            if enum_rttr_enable:
                # append generator data
                enum.generator_data["rttr"] = EnumData(
                    flags=enum_rttr["flags"].visited_or([]),
                    attrs=enum_rttr["attrs"].visited_or([]),
                )

                # parse items
                for item in enum.values.values():
                    item_rttr = item.attrs["rttr"]
                    item_rttr_enable = item_rttr.is_function_enable(True, True)
                    if item_rttr_enable:
                        item.generator_data["rttr"] = EnumItemData(
                            flags=item_rttr["flags"].visited_or([]),
                            attrs=item_rttr["attrs"].visited_or([]),
                        )

    def generate_body(self):
        db = self.owner.database
        for record in db.main_module.get_records():
            if db.is_derived(record, "skr::rttr::IObject"):
                record.generated_body_content += '''
GUID iobject_get_typeid() const override
{
    using namespace skr::rttr;
    using ThisType = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>;
    return type_id_of<ThisType>();
}
void* iobject_get_head_ptr() const override { return const_cast<void*>((const void*)this); }
void embedded_rc_delete() override { SkrDelete(this); }
'''

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
            guid_records = [record for record in records if "guid" in record.generator_data]
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_template.render(
                    enums=enums,
                    records=records,
                    guid_records=guid_records,
                    api=f"{main_module.api}_API"
                )
            )

        # gen source
        enums = [enum for enum in main_module.get_enums() if "rttr" in enum.generator_data]
        records = [record for record in main_module.get_records() if "rttr" in record.generator_data]
        self.owner.append_content(
            "generated.cpp",
            source_template.render(
                enums=enums,
                records=records,
                tools=CodegenTools()
            )
        )


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("rttr", RTTRGenerator())
