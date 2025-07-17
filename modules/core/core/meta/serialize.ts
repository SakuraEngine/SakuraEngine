import * as gen from "@framework/generator";
import * as db from "@framework/database";
import * as ml from "@framework/meta_lang";

class ConfigBase {
  @ml.value("boolean")
  json: boolean = false;
  @ml.value("boolean")
  bin: boolean = false;

  @ml.preset("json")
  preset_json() {
    this.json = true;
  }
  @ml.preset("bin")
  preset_bin() {
    this.bin = true;
  }
  @ml.preset("disable")
  preset_disable() {
    this.json = false;
    this.bin = false;
  }
}

class RecordConfig extends ConfigBase {
}

class FieldConfig extends ConfigBase {
  @ml.value("string")
  alias: string = "";

  // default true, but blocked by record
  override json: boolean = true;
  override bin: boolean = true;
}

class EnumConfig extends ConfigBase {
}
class EnumValueConfig extends ConfigBase {
  // default true, but blocked by enum
  override json: boolean = true;
  override bin: boolean = true;
}

class _Gen {
  static header(header: db.Header) {
    const b = header.gen_code;
    const _gen_records_json = header.records.filter(record => record.ml_configs.serde.json);
    const _gen_records_bin = header.records.filter(record => record.ml_configs.serde.bin);
    const _gen_enum_json = header.enums.filter(enum_ => enum_.ml_configs.serde.json);
    const _gen_api = `${header.parent.config.api}_API`;

    b.$line(`// BEGIN SERIALIZE GENERATED`);
    b.$line(`#include "SkrSerde/bin_serde.hpp"`);
    b.$line(`#include "SkrSerde/json_serde.hpp"`);
    b.$line(``);

    // json
    b.$line(`// json serde`);
    b.$namespace("skr", (_b) => {
      // enum serde traits
      _gen_enum_json.forEach((enum_) => {
        b.$line(`template <>`);
        b.$line(`struct ${_gen_api} EnumSerdeTraits<${enum_.name}> {`);
        b.$indent((_b) => {
          b.$line(`static skr::StringView to_string(const ${enum_.name}& value);`);
          b.$line(`static bool from_string(skr::StringView str, ${enum_.name}& value);`);
        });
        b.$line(`};`);
      });

      // record serde
      _gen_records_json.forEach((record) => {
        b.$line(`template <>`);
        b.$line(`struct ${_gen_api} JsonSerde<${record.name}> {`);
        b.$indent((_b) => {
          b.$line(`static bool read_fields(skr::archive::JsonReader* r, ${record.name}& v);`);
          b.$line(`static bool write_fields(skr::archive::JsonWriter* w, const ${record.name}& v);`);
          b.$line(``);
          b.$line(`static bool read(skr::archive::JsonReader* r, ${record.name}& v);`);
          b.$line(`static bool write(skr::archive::JsonWriter* w, const ${record.name}& v);`);
        });
        b.$line(`};`);
      });
    });

    // bin
    b.$line(`// bin serde`);
    b.$namespace("skr", (_b) => {
      _gen_records_bin.forEach((record) => {
        b.$line(`template<>`);
        b.$line(`struct ${_gen_api} BinSerde<${record.name}> {`);
        b.$indent((_b) => {
          b.$line(`static bool read(SBinaryReader* r, ${record.name}& v);`);
          b.$line(`static bool write(SBinaryWriter* w, const ${record.name}& v);`);
        });
        b.$line(`};`);
      });
    });

    b.$line(``);
    b.$line(`// END SERIALIZE GENERATED`);
  }
  static source(main_db: db.Module) {
    const b = main_db.main_file
    const _gen_records_json = main_db.filter_record(record => record.ml_configs.serde.json);
    const _gen_records_bin = main_db.filter_record(record => record.ml_configs.serde.bin);
    const _gen_enum_json = main_db.filter_enum(enum_ => enum_.ml_configs.serde.json);

    // init datas
    b.$line(`// BEGIN SERIALIZE GENERATED`);
    b.$line(`#include "SkrProfile/profile.h"`);
    b.$line(`#include "SkrSerde/json_serde.hpp"`);
    b.$line(`#include "SkrSerde/bin_serde.hpp"`);
    b.$line(``);
    b.$line(`// debug`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonArrayJsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s[%d] failed: %s";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonArrayFieldArchiveWarnFormat = u8"[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s failed: %s";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonFieldNotFoundErrorFormat = u8"[SERDE/JSON] %s.%s not found";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonFieldNotFoundFormat = u8"[SERDE/JSON] %s.%s not found, using default value";`);
    b.$line(``);
    b.$line(`[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughErrorFormat = u8"[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughWarnFormat = u8"[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";`);
    b.$line(`[[maybe_unused]] static const char8_t* JsonBaseArchiveFailedFormat = u8"[SERDE/JSON] Archive %s base %s failed: %d";`);
    b.$line(``);
    b.$line(`[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";`);
    b.$line(`[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";`);
    b.$line(`[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";`);

    // json serde
    b.$line(`// json serde`);
    b.$namespace("skr", (_b) => {

      // enum serde traits
      _gen_enum_json.forEach((enum_) => {
        const _gen_enum_value_json = enum_.values.filter(enum_value => enum_value.ml_configs.serde.json);

        // to string
        b.$line(`skr::StringView EnumSerdeTraits<${enum_.name}>:: to_string(const ${enum_.name}& value){`);
        b.$indent((_b) => {
          b.$line(`switch (value) {`);
          b.$indent((_b) => {
            _gen_enum_value_json.forEach((enum_value) => {
              b.$line(`case ${enum_.name}::${enum_value.short_name}: return u8"${enum_value.short_name}";`);
            });
            b.$line(`default: SKR_UNREACHABLE_CODE(); return u8"${enum_.name}::__INVALID_ENUMERATOR__";`);
          });
          b.$line(`}`);
        });
        b.$line(`}`);

        // from string
        b.$function(
          `bool EnumSerdeTraits<${enum_.name}>::from_string(skr::StringView str, ${enum_.name}& value)`, (_b) => {
            b.$line(`const auto hash = skr_hash64(str.data(), str.size(), 0);`);
            b.$switch(`hash`, (_b) => {
              _gen_enum_value_json.forEach((enum_value) => {
                b.$line(`case skr::consteval_hash(u8"${enum_value.short_name}"): if(str == u8"${enum_value.short_name}") value = ${enum_value.name}; return true;`);
              });
              b.$line(`default: return false;`);
            });
          },
        );
      });

      // record serde
      _gen_records_json.forEach((record) => {
        const _gen_fields_json = record.fields.filter(field => !field.is_static && field.ml_configs.serde.json);

        // read fields
        b.$function(
          `bool JsonSerde<${record.name}>::read_fields(skr::archive::JsonReader* r, ${record.name}& v)`, (_b) => {
            // profiling
            b.$line(`SkrZoneScopedN("JsonSerde<${record.name}>::read_fields");`);
            b.$line(``);

            // read bases
            b.$line(`// read bases`);
            record.bases.forEach((base) => {
              b.$line(`JsonSerde<${base}>::read_fields(r, v);`);
            });
            b.$line(``);

            // read fields
            b.$line(`// read self fields`);
            _gen_fields_json.forEach((field) => {
              const field_config = field.ml_configs.serde as FieldConfig;
              const json_key = field_config.alias.length > 0
                ? field_config.alias
                : field.short_name;

              b.$scope((_b) => {
                // read key
                b.$line(`auto jSlot = r->Key(u8"${json_key}");`);
                b.$line(`jSlot.error_then([&](auto e) {`);
                b.$indent((_b) => {
                  b.$line(`SKR_ASSERT(e == skr::archive::JsonReadError::KeyNotFound);`);
                });
                b.$line(`});`);

                // read value
                b.$if([`jSlot.has_value()`, (_b) => {
                  b.$if([`!json_read<${field.signature()}>(r, v.${field.short_name})`, (_b) => {
                    b.$line(`SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${field.short_name}", "UNKNOWN ERROR");  \/\/ TODO: ERROR MESSAGE`);
                    b.$line(`return false;`);
                  },
                  ]);
                }]);
              });
            });

            // success
            b.$line(`return true;`);
          },
        );

        // write fields
        b.$function(`bool JsonSerde<${record.name}>::write_fields(skr::archive::JsonWriter* w, const ${record.name}& v)`, (_b) => {
          // bases
          b.$line(`// write bases`);
          record.bases.forEach((base) => {
            b.$line(`if (!JsonSerde<${base}>::write_fields(w, v)) return false;`);
          });

          // fields
          b.$line(`// write self fields`);
          _gen_fields_json.forEach((field) => {
            const field_config = field.ml_configs.serde as FieldConfig;
            const json_key = field_config.alias.length > 0
              ? field_config.alias
              : field.short_name;

            b.$line(`SKR_EXPECTED_CHECK(w->Key(u8"${json_key}"), false);`);
            b.$line(`if (!json_write<${field.signature()}>(w, v.${field.short_name})) return false;`);
          });

          // success
          b.$line(`return true;`);
        },
        );

        // read
        b.$function(
          `bool JsonSerde<${record.name}>::read(skr::archive::JsonReader* r, ${record.name}& v)`, (_b) => {
            b.$line(`SkrZoneScopedN("JsonSerde<${record.name}>::write");`);
            b.$line(`SKR_EXPECTED_CHECK(r->StartObject(), false);`);
            b.$line(`if (!read_fields(r, v)) return false;`);
            b.$line(`SKR_EXPECTED_CHECK(r->EndObject(), false);`);
            b.$line(`return true;`);
          },
        );

        // write
        b.$function(
          `bool JsonSerde<${record.name}>::write(skr::archive::JsonWriter* w, const ${record.name}& v)`, (_b) => {
            b.$line(`SkrZoneScopedN("JsonSerde<${record.name}>::write");`);
            b.$line(`SKR_EXPECTED_CHECK(w->StartObject(), false);`);
            b.$line(`if (!write_fields(w, v)) return false;`);
            b.$line(`SKR_EXPECTED_CHECK(w->EndObject(), false);`);
            b.$line(`return true;`);
          },
        );
      });
    });

    // bin serde
    b.$line(`// bin serde`);
    b.$namespace("skr", (_b) => {
      _gen_records_bin.forEach((record) => {
        const _gen_fields_bin = record.fields.filter(field => !field.is_static && field.ml_configs.serde.bin);

        // read
        b.$function(`bool BinSerde<${record.name}>::read(SBinaryReader* r, ${record.name}& v)`, (_b) => {
          b.$line(`SkrZoneScopedN("BinSerde<${record.name}>::read");`);
          b.$line(``);

          // serde bases
          b.$line(`// serde bases`);
          record.bases.forEach((base) => {
            b.$if([`!bin_read<${base}>(r, v)`, (_b) => {
              b.$line(`SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);`);
              b.$line(`return false;`);
            }]);
          });

          // serde fields
          b.$line(`// serde fields`);
          _gen_fields_bin.forEach((field) => {
            b.$if([`!bin_read<${field.signature()}>(r, v.${field.short_name})`, (_b) => {
              b.$line(`SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Read", "${record.name}", "${field.short_name}", -1);`);
              b.$line(`return false;`);
            }]);
          });

          // success
          b.$line(`return true;`);
        },
        );

        // write
        b.$function(`bool BinSerde<${record.name}>::write(SBinaryWriter* w, const ${record.name}& v)`, (_b) => {
          b.$line(`SkrZoneScopedN("BinSerde<${record.name}>::write");`);
          b.$line(``);

          // serde bases
          b.$line(`// serde bases`);
          record.bases.forEach((base) => {
            b.$if([`!bin_write<${base}>(w, v)`, (_b) => {
              b.$line(`SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);`);
              b.$line(`return false;`);
            }]);
          });

          // serde fields
          b.$line(`// serde fields`);
          _gen_fields_bin.forEach((field) => {
            b.$if([`!bin_write<${field.signature()}>(w, v.${field.short_name})`, (_b) => {
              b.$line(`SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Write", "${record.name}", "${field.short_name}", -1);`);
              b.$line(`return false;`);
            }]);
          });

          // success
          b.$line(`return true;`);
        },
        );
      });
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
      if (record.ml_configs.serde.json) {
        const _gen_fields_json = record.fields.filter(field => !field.is_static && field.ml_configs.serde.json);
        let _has_private_json_field = false;
        _gen_fields_json.forEach((field) => {
          if (field.access !== "public") {
            _has_private_json_field = true;
          }
        });
        if (_has_private_json_field) {
          record.generate_body_content.$line(`friend struct ::skr::JsonSerde<${record.name}>;`);
        }
      }
      if (record.ml_configs.serde.bin) {
        const _gen_fields_bin = record.fields.filter(field => !field.is_static && field.ml_configs.serde.bin);
        let _has_private_bin_field = false;
        _gen_fields_bin.forEach((field) => {
          if (field.access !== "public") {
            _has_private_bin_field = true;
          }
        });
        if (_has_private_bin_field) {
          record.generate_body_content.$line(`friend struct ::skr::BinSerde<${record.name}>;`);
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
