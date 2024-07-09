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
class ECSComponentData:
    flags: t.List[int]
    array: int
    custom: str


class CodegenTools:
    def make_field_offset_list(record: cpp.Record,
                               pred: t.Callable[[cpp.Field], t.List[str]],
                               codegen_db: db.CodegenDatabase):
        def __recursive(record: cpp.Record,
                        field_stack: t.List[str]):
            result = []
            for field in record.fields:
                field_stack.append(f"offsetof({record.name}, {field.name})")
                found_record = codegen_db.find_record(field.type)
                if (pred(field)):  # append field pass pred
                    result.append(" + ".join(field_stack))
                elif found_record:  # recursive visit field
                    result.extend(__recursive(found_record, pred, field_stack))
                field_stack.pop()
            return result

        field_stack = []
        return __recursive(record, field_stack)

    def filter_entity(field: cpp.Field):
        return field.rawType == "sugoi_entity_t"

    def filter_resource_handle(field: cpp.Field):
        return field.type == "skr_resource_handle_t" or field.type.startswith("skr::resource::TResourceHandle")

    def guid_constant(guid: str):
        return "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18],
            guid[19:21], guid[21:23], guid[24:26],
            guid[26:28], guid[28:30], guid[30:32],
            guid[32:34], guid[34:36])

    def flag_expr(flags: t.List[int]):
        return " | ".join([f"SUGOI_TYPE_FLAG_{flag}" for flag in flags])


class ECSGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # record scheme
        sc.Namespace({
            "ecs": sc.Namespace({
                "comp": sc.Functional({
                    "flags": sc.List(),  # SUGOI_TYPE_FLAG_XXXX
                    "array": sc.Int(),  # array component
                    "custom": sc.Str(),  # custom logic when register component
                })
            })
        })

    def solve_attrs(self):
        records = self.owner.database.get_records()

        for record in records:
            # get ecs data
            record_ecs = record.attrs["ecs"]
            if not record_ecs.is_visited():
                continue

            # parse component
            record_ecs_component = record_ecs["comp"]
            if record_ecs_component.is_function_enable():
                record_ecs_component_data = ECSComponentData(
                    flags=record_ecs_component["flags"].visited_or([]),
                    array=record_ecs_component["array"].visited_or(0),
                    custom=record_ecs_component["custom"].visited_or("")
                )

                # add ecs component data
                record.generator_data["ecs_component"] = record_ecs_component_data

    def generate(self):
        # load template
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_template = self.owner.load_template(os.path.join(script_dir, "ecs.hpp.mako"))
        source_template = self.owner.load_template(os.path.join(script_dir, "ecs.cpp.mako"))

        # load data
        main_module = self.owner.database.main_module

        # gen header
        for header_db in main_module.header_dbs:
            records = [record for record in header_db.get_records() if "ecs_component" in record.generator_data]
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_template.render(records=records, api=f"{main_module.api}_API")
            )

        # gen source
        records = [record for record in main_module.get_records() if "ecs_component" in record.generator_data]
        self.owner.append_content(
            "generated.cpp",
            source_template.render(records=records, tools=CodegenTools, codegen_db=self.owner.database)
        )
