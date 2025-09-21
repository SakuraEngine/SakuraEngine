import * as gen from "@framework/generator";
import * as db from "@framework/database";
import * as ml from "@framework/meta_lang";
import { CodeBuilder } from "@framework/utils";

class RecordConfig extends ml.WithEnable {
  comp: CompConfig = new CompConfig()
}
class CompConfig extends ml.WithEnable {
  @ml.array('string')
  flags: string[] = []

  @ml.value('number')
  array: number = 0

  @ml.value('string')
  custom: string = ""
}

class _Gen {
  static header(header: db.Header) {
    const b = header.gen_code
    const _gen_records = header.records.filter(record => record.ml_configs.ecs.enable);
    const _gen_api = `${header.parent.config.api}_API`;

    b.$line(`// BEGIN ECS GENERATED`)
    b.$line(`#include "SkrRuntime/sugoi/sugoi.h"`)
    _gen_records.forEach(record => {
      const record_config = record.ml_configs.ecs as RecordConfig;
      b.$line(`template<>`)
      b.$line(`struct sugoi_id_of<::${record.name}> {`)
      b.$indent(_b => {
        b.$line(`${_gen_api} static sugoi_type_index_t get();`)
      })
      b.$line(`};`)

      if (record_config.comp.array > 0) {
        b.$line(`template<> inline constexpr uint64_t sugoi_array_count<::${record.name}> = ${record_config.comp.array};`)
      }
    })
    b.$line(`// END ECS GENERATED`)
  }

  static source(main_db: db.Module) {
    const b = main_db.main_file

    const _gen_records = main_db.filter_record(record => record.ml_configs.ecs.enable);

    // title
    b.$line(`// BEGIN ECS GENERATED`)
    b.$line(`#include "SkrRuntime/sugoi/sugoi.h"`)
    b.$line(`#include "SkrRuntime/sugoi/array.hpp"`)
    b.$line(`#include "SkrRuntime/sugoi/serde.hpp"`)
    b.$line(`#include "SkrCore/exec_static.hpp"`)

    // sugoi id of
    b.$line(`// impl sugoi_id_of`)
    _gen_records.forEach(record => {
      b.$line(`static sugoi_type_index_t& _sugoi_id_${record.name.replaceAll("::", "_")}() { static sugoi_type_index_t val = SUGOI_NULL_TYPE; return val;  }`)
      b.$line(`sugoi_type_index_t sugoi_id_of<::${record.name}>::get() {`)
      b.$indent(_b => {
        b.$line(`SKR_ASSERT(_sugoi_id_${record.name.replaceAll("::", "_")}() != SUGOI_NULL_TYPE);`)
        b.$line(`return _sugoi_id_${record.name.replaceAll("::", "_")}();`)
      })
      b.$line(`}`)
    })

    // register component types
    b.$line(`// register component types`)
    b.$line(`SKR_EXEC_STATIC_CTOR {`)
    b.$indent(_b => {
      b.$line(`using namespace ::skr::literals;`)

      // register each record
      _gen_records.forEach(record => {
        const record_config = record.ml_configs.ecs as RecordConfig;

        b.$line(`// register ecs component type ${record.name}`)
        b.$scope(_b => {
          b.$line(`sugoi_type_description_t desc;`)
          b.$line(``)

          // name & guid
          b.$line(`// name & guid`)
          b.$line(`desc.name = u8"${record.name}";`)
          b.$line(`desc.guid = u8"${record.ml_configs.guid}"_guid;`)
          b.$line(`desc.guidStr = u8"${record.ml_configs.guid}";`)

          // size & alignment
          b.$line(`// size & alignment`)
          if (record_config.comp.array > 0) {
            b.$line(`desc.alignment = alignof(sugoi::ArrayComponent<${record.name}, ${record_config.comp.array}>);`)
            b.$line(`desc.size = sizeof(sugoi::ArrayComponent<${record.name}, ${record_config.comp.array}>);`)
            b.$line(`desc.elementSize = sizeof(${record.name});`)
          } else {
            b.$line(`desc.alignment = alignof(${record.name});`)
            b.$line(`desc.size = std::is_empty_v<${record.name}> ? 0 : sizeof(${record.name});`)
            b.$line(`desc.elementSize = 0;`)
          }

          // flags
          b.$line(`// flags`)
          if (record_config.comp.flags.length > 0) {
            b.$line(`desc.flags = ${this.#flag_expr(record_config.comp.flags)};`)
          } else {
            b.$line(`desc.flags = 0;`)
          }

          // entity fields
          const entity_fields_offset = this.#make_field_offset_list(record, this.#filter_entity, main_db.parent);
          b.$line(`// entity fields`)
          if (entity_fields_offset.length > 0) {
            b.$line(`desc.entityFieldsCount = ${entity_fields_offset.length};`)
            b.$line(`static intptr_t entityFields[] = {${entity_fields_offset.join(", ")}};`)
            b.$line(`desc.entityFields = entityFields;`)
          } else {
            b.$line(`desc.entityFieldsCount = 0;`)
            b.$line(`desc.entityFields = 0;`)
          }

          // resource fields
          const resource_fields_offset = this.#make_field_offset_list(record, this.#filter_resource_handle, main_db.parent);
          b.$line(`// resource fields`)
          if (resource_fields_offset.length > 0) {
            b.$line(`desc.resourceFieldsCount = ${resource_fields_offset.length};`)
            b.$line(`static intptr_t resourceFields[] = {${resource_fields_offset.join(", ")}};`)
            b.$line(`desc.resourceFields = resourceFields;`)
          } else {
            b.$line(`desc.resourceFieldsCount = 0;`)
            b.$line(`desc.resourceFields = 0;`)
          }

          // comp callbacks
          b.$line(`// component callbacks`)
          b.$line(`sugoi::SetSerdeCallback<${record.name}>(desc);`);

          // custom logic
          b.$line(`// custom logic`)
          if (record_config.comp.custom.length > 0) {
            b.$line(`${record_config.comp.custom}(desc, skr::type_t<${record.name}>{});`)
          }

          // check managed
          b.$line(`::sugoi::check_managed(desc, skr::type_t<${record.name}>{});`)

          // assign static id
          b.$line(`_sugoi_id_${record.name.replaceAll("::", "_")}() = sugoiT_register_type(&desc);`)
        })
      })
    })
    b.$line(`};`)

    // bottom
    b.$line(`// END ECS GENERATED`)
  }
  static #flag_expr(flags: string[]): string {
    return flags.map(flag => `SUGOI_TYPE_FLAG_${flag}`).join(" | ")
  }
  static #make_field_offset_list(record: db.Record, pred: (field: db.Field) => boolean, project_db: db.Project) {
    const recursive = (record: db.Record, field_stack: string[]) => {
      const result: string[] = []
      record.fields.forEach(field => {
        // push offset of expr
        field_stack.push(`offsetof(${record.name}, ${field.short_name})`)

        // find record and add offset
        const found_record = project_db.find_record(field.type)
        if (pred(field)) {
          // push offset
          result.push(field_stack.join(" + "))
        } else if (found_record) {
          // recursive call
          result.push(...recursive(found_record, field_stack))
        }

        // pop expr
        field_stack.pop()
      })
      return result
    }

    return recursive(record, [])
  }
  static #filter_entity(field: db.Field): boolean {
    return field.raw_type == "sugoi_entity_t"
  }
  static #filter_resource_handle(field: db.Field): boolean {
    return field.type == "SResourceHandle" || field.type.startsWith("skr::AsyncResource")
  }
}


class ECSGenerator extends gen.Generator {
  override inject_configs(): void {
    this.main_module_db.each_record(record => {
      record.ml_configs.ecs = new RecordConfig();
    })
  }

  override gen(): void {
    // generate header
    this.main_module_db.headers.forEach(header => {
      _Gen.header(header);
    })

    // generate source
    _Gen.source(this.main_module_db);
  }
}


export function load_generator(gm: gen.GenerateManager) {
  gm.add_generator("ecs", new ECSGenerator());
}
