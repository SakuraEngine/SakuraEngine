import * as gen from "@framework/generator";
import * as db from "@framework/database";
import * as ml from "@framework/meta_lang";

class ConfigBase extends ml.WithEnable {
}

class RecordConfig extends ConfigBase {
}

class FieldConfig extends ConfigBase {
  @ml.value("string")
  alias: string = "";

  @ml.value("boolean")
  required: boolean = false;

  // default true, but blocked by record
  override enable: boolean = true;
}

class EnumConfig extends ConfigBase {
}
class EnumValueConfig extends ConfigBase {
  // default true, but blocked by enum
  override enable: boolean = true;
}

class _Gen {
  static header(header: db.Header) {
    const b = header.gen_code;
    const _gen_records = header.records.filter(record => record.ml_configs.serde.enable);
    const _gen_api = `${header.parent.config.api}_API`;

    b.$line(`// BEGIN SERIALIZE GENERATED`);
    b.$line(`#include <SkrCore/serialize/serialize_traits.hpp>`);
    b.$namespace("skr", (_b) => {
      _gen_records.forEach((record) => {
        b.$line(`template<>`);
        b.$line(`struct ${_gen_api} Serialize<${record.name}> {`);
        b.$indent((_b) => {
          b.$line(`static void read(ArchiveRead& r, ${record.name}& v);`);
          b.$line(`static void write(ArchiveWrite& w, const ${record.name}& v);`);
          b.$line(`static void read_fields(ArchiveRead& r, ${record.name}& v);`);
          b.$line(`static void write_fields(ArchiveWrite& w, const ${record.name}& v);`);
        });
        b.$line(`};`);
      });
    });
    b.$line(`// END SERIALIZE GENERATED`);
  }
  static source(main_db: db.Module) {
    const b = main_db.main_file
    const _gen_records = main_db.filter_record(record => record.ml_configs.serde.enable);

    // init datas
    b.$line(`// BEGIN SERIALIZE GENERATED`);

    b.$line(`#include "SkrProfile/profile.h"`);
    b.$namespace("skr", (_b) => {
      _gen_records.forEach((record) => {
        const _gen_fields_uniform = record.fields.filter(field => !field.is_static && field.ml_configs.serde.enable);

        // read
        b.$function(`void Serialize<${record.name}>::read(ArchiveRead& r, ${record.name}& v)`, (_b) => {
          b.$line(`ArchiveRead::ObjectScope scope(r);`);
          b.$line(`SKR_FAST_CHECK(scope.is_success(), );`);
          b.$line(`read_fields(r, v);`);
        });

        // write
        b.$function(`void Serialize<${record.name}>::write(ArchiveWrite& w, const ${record.name}& v)`, (_b) => {
          b.$line(`ArchiveWrite::ObjectScope scope(w);`);
          b.$line(`SKR_FAST_CHECK(scope.is_success(), );`);
          b.$line(`write_fields(w, v);`);
        });

        // read fields
        b.$function(`void Serialize<${record.name}>::read_fields(ArchiveRead& r, ${record.name}& v)`, (_b) => {
          // profiling
          b.$line(`SkrZoneScopedN("Serialize<${record.name}>::read_fields");`);
          b.$line(``);

          // serde bases
          b.$line(`// serde bases`);
          record.bases.forEach((base) => {
            b.$line(`if constexpr (::skr::concepts::HasSerdeReadFields<${base}>) {`)
            b.$indent(_b => {
              b.$line(`serde_read_fields<${base}>(r, v);`);
              b.$line(`SKR_FAST_CHECK(r.checkpoint(), );`);
            })
            b.$line(`}`);
          });
          b.$line(``);

          // serde fields
          b.$line(`// serde fields`);
          _gen_fields_uniform.forEach((field) => {
            const field_config = field.ml_configs.serde as FieldConfig;
            const field_key = field_config.alias.length > 0
              ? field_config.alias
              : field.short_name;
            const required_suffix = field_config.required ? "_required" : "";
            b.$line(`SKR_FAST_CHECK((r.key_value${required_suffix}<${field.signature()}>(u8"${field_key}", v.${field.short_name})), );`);
          });
        });

        // write fields
        b.$function(`void Serialize<${record.name}>::write_fields(ArchiveWrite& w, const ${record.name}& v)`, (_b) => {
          // profiling
          b.$line(`SkrZoneScopedN("Serialize<${record.name}>::write_fields");`);
          b.$line(``);

          // serde bases
          b.$line(`// serde bases`);
          record.bases.forEach((base) => {
            b.$line(`if constexpr (::skr::concepts::HasSerdeWriteFields<${base}>) {`)
            b.$indent(_b => {
              b.$line(`serde_write_fields<${base}>(w, v);`);
              b.$line(`SKR_FAST_CHECK(w.checkpoint(), );`);
            })
            b.$line(`}`);
          });
          b.$line(``);

          // serde fields
          b.$line(`// serde fields`);
          _gen_fields_uniform.forEach((field) => {
            const field_config = field.ml_configs.serde as FieldConfig;
            const field_key = field_config.alias.length > 0
              ? field_config.alias
              : field.short_name;
            b.$line(`SKR_FAST_CHECK((w.key_value<${field.signature()}>(u8"${field_key}", v.${field.short_name})), );`);
          });
        });

      })
    });

    b.$line(`// END SERIALIZE GENERATED`);
  }
}

class SerializeGenerator extends gen.Generator {
  override inject_configs(): void {
    // records
    this.main_module_db.each_record((record) => {
      record.ml_configs.serde = new RecordConfig();

      // fields
      record.fields.forEach((field) => {
        field.ml_configs.serde = new FieldConfig();
      });
    });

    // enums
    this.main_module_db.each_enum((enum_) => {
      enum_.ml_configs.serde = new EnumConfig();

      // enum values
      enum_.values.forEach((enum_value) => {
        enum_value.ml_configs.serde = new EnumValueConfig();
      });
    });
  }

  override gen_body(): void {
    this.main_module_db.each_record((record, header) => {
      // serialize
      if (record.ml_configs.serde.enable) {
        const _gen_fields = record.fields.filter(field => !field.is_static && field.ml_configs.serde.enable);
        const _has_private_json_field = _gen_fields.some(field => field.access !== "public");
        if (_has_private_json_field) {
          record.generate_body_content.$line(`friend struct ::skr::Serialize<${record.name}>;`);
        }
      }
    });
  }
  override gen(): void {
    this.main_module_db.headers.forEach((header) => {
      _Gen.header(header);
    });
    _Gen.source(this.main_module_db);
  }
}

export function load_generator(gm: gen.GenerateManager) {
  gm.add_generator("serialize", new SerializeGenerator());
}
