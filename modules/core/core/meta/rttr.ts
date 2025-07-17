import * as gen from "@framework/generator";
import * as db from "@framework/database";
import * as ml from "@framework/meta_lang";
import { CodeBuilder } from "@framework/utils";

class ConfigBase extends ml.WithEnable {
  @ml.array("string")
  flags: string[] = [];

  @ml.array("string")
  attrs: string[] = [];
}

class GuidConfig {
  @ml.value("string")
  value: string = "";

  @ml.value_proxy("string")
  proxy_value(v: string) {
    this.value = v;
  }

  is_empty() { return this.value.length === 0; }

  as_constant() {
    let result = ""

    // 32bit
    result += `0x${this.value.slice(0, 8)}, `   // 1st comp

    // 16bit * 2
    result += `0x${this.value.slice(9, 13)}, `  // 2st comp
    result += `0x${this.value.slice(14, 18)}, ` // 3st comp

    // 8bit * 8
    result += "{"
    result += `0x${this.value.slice(19, 21)}, ` // 4st comp
    result += `0x${this.value.slice(21, 23)}, ` // 4st comp
    result += `0x${this.value.slice(24, 26)}, ` // 5st comp
    result += `0x${this.value.slice(26, 28)}, ` // 5st comp
    result += `0x${this.value.slice(28, 30)}, ` // 5st comp
    result += `0x${this.value.slice(30, 32)}, ` // 5st comp
    result += `0x${this.value.slice(32, 34)}, ` // 5st comp
    result += `0x${this.value.slice(34, 36)}`   // 5st comp
    result += "}"


    return result
  }

  toString() {
    return this.value;
  }
}
type CppConfigTypes =
  | RecordConfig
  | FieldConfig
  | MethodConfig
  | ParamConfig
  | EnumConfig
  | EnumValueConfig;
class RecordConfig extends ConfigBase {
  // default is true
  override enable: boolean = true;

  @ml.value("boolean")
  reflect_bases: boolean = true;

  @ml.array("string")
  exclude_bases: string[] = [];

  @ml.value("boolean")
  reflect_fields(v: boolean) {
    this.#record.fields.forEach(field => {
      field.ml_configs.rttr.enable = v;
    })
  }

  @ml.value("boolean")
  reflect_methods(v: boolean) {
    this.#record.methods.forEach(method => {
      method.ml_configs.rttr.enable = v;
    })
  }

  @ml.value("boolean")
  reflect_ctors(v: boolean) {
    this.#record.ctors.forEach(ctor => {
      ctor.ml_configs.rttr.enable = v;
    })
  }

  @ml.preset("full")
  preset_full() {
    this.reflect_bases = true;
    this.reflect_fields(true);
    this.reflect_methods(true);
  }

  @ml.preset("fields")
  preset_fields() {
    this.reflect_fields(true);
  }

  @ml.preset("methods")
  preset_methods() {
    this.reflect_methods(true);
  }

  @ml.preset("ctors")
  preset_ctors() {
    this.reflect_ctors(true);
  }

  @ml.preset("minimal")
  preset_minimal() {
    this.reflect_bases = false;
    this.reflect_fields(false);
    this.reflect_methods(false);
  }

  #record: db.Record;
  constructor(record: db.Record) {
    super();
    this.#record = record;
  }
}
class CtorConfig extends ConfigBase {
}
class FieldConfig extends ConfigBase {
}
class MethodConfig extends ConfigBase {
  @ml.value("boolean")
  script_mixin: boolean = false;
}
class ParamConfig extends ConfigBase {
}
class EnumConfig extends ConfigBase {
  override enable: boolean = true;
}
class EnumValueConfig extends ConfigBase {
  override enable: boolean = true;
}

class _Gen {
  static header(header: db.Header) {
    const b = header.gen_code;
    const _gen_records = header.records.filter(_Gen.filter_record);
    const _gen_enums = header.enums.filter(_Gen.filter_enum);

    b.$line(`// BEGIN RTTR GENERATED`);
    b.$line(`// RTTRTraits`)
    b.$line(`#include "SkrRTTR/rttr_traits.hpp"`);
    _gen_records.forEach((record) => {
      b.$line(`SKR_RTTR_TYPE(${record.name}, "${record.ml_configs.guid}")`,);
    });
    _gen_enums.forEach((enum_) => {
      b.$line(`SKR_RTTR_TYPE(${enum_.name}, "${enum_.ml_configs.guid}")`,);
    });
    b.$line(``)
    b.$line(`// forward export functions for friend`)
    b.$line(`namespace skr {`);
    header.each_record((record, header) => {
      // skip if rttr is disabled
      if (record.ml_configs.rttr.enable === false) return;

      // gen friend for export
      {
        const _gen_fields: db.Field[] = record.fields.filter((field) => field.ml_configs.rttr.enable);
        const _gen_methods: db.Method[] = record.methods.filter((method) => method.ml_configs.rttr.enable);
        const _gen_ctors: db.Ctor[] = record.ctors.filter(ctor => ctor.ml_configs.rttr.enable);
        let _any_private_export = false;
        _gen_fields.forEach((field) => {
          if (field.access !== "public") {
            _any_private_export = true;
          }
        });
        _gen_methods.forEach((method) => {
          if (method.access !== "public") {
            _any_private_export = true;
          }
        });
        _gen_ctors.forEach((ctor) => {
          if (ctor.access !== "public") {
            _any_private_export = true;
          }
        });

        if (_any_private_export) {
          b.$line(`void zz_register_record_${record.name.replaceAll('::', '_')}(struct ::skr::RTTRType* type);`);
        }
      }
    })
    b.$line(`}`);
    b.$line(`// END RTTR GENERATED`);
  }
  static source(main_db: db.Module) {
    const b = main_db.main_file
    const _gen_records = main_db.filter_record(_Gen.filter_record);
    const _gen_enums = main_db.filter_enum(_Gen.filter_enum);
    this.source_batched(b, _gen_records, _gen_enums, main_db.config.module_name);
  }
  static source_batched(b: CodeBuilder, in_records: db.Record[], in_enums: db.Enum[], module_name: string) {
    const _gen_records = in_records;
    const _gen_enums = in_enums;

    // header
    b.$line(`// BEGIN RTTR GENERATED`);
    b.$line(`#include "SkrBase/misc/hash.h"`);
    b.$line(`#include "SkrRTTR/type.hpp"`);
    b.$line(`#include "SkrCore/exec_static.hpp"`);
    b.$line(`#include "SkrContainers/tuple.hpp"`);
    b.$line(`#include "SkrRTTR/export/export_builder.hpp"`);
    b.$line(``);

    // static register functions
    b.$line(`// static register functions`);
    b.$line(`namespace skr {`);
    {
      // export records
      _gen_records.forEach((record, _header) => {
        const record_config = record.ml_configs.rttr as RecordConfig;

        // collect export data
        const _gen_bases: string[] = record_config.reflect_bases ?
          record.bases.filter((base) => !record_config.exclude_bases.includes(base)) :
          [];
        const _gen_fields: db.Field[] = record.fields.filter((field) => field.ml_configs.rttr.enable);
        const _gen_methods: db.Method[] = record.methods.filter((method) => method.ml_configs.rttr.enable);
        const _gen_ctors: db.Ctor[] = record.ctors.filter(ctor => ctor.ml_configs.rttr.enable);

        // register function
        b.$line(`static void zz_register_record_${record.name.replaceAll('::', '_')}(RTTRType* type) {`);
        b.$indent((_b) => {
          // module info
          b.$line(`// setup module`);
          b.$line(`type->set_module(u8"${module_name}");`);
          b.$line(``);

          // build scope
          b.$line(`// build scope`);
          b.$line(`type->build_record([&](RTTRRecordData* record_data) {`);
          b.$indent((_b) => {
            // basic info
            b.$line(`// reserve`);
            b.$line(`record_data->bases_data.reserve(${_gen_bases.length});`);
            b.$line(`record_data->fields.reserve(${_gen_fields.length});`);
            b.$line(`record_data->methods.reserve(${_gen_methods.length});`);
            b.$line(``);
            b.$line(`RTTRRecordBuilder<${record.name}> builder(record_data);`);
            b.$line(``);
            b.$line(`// basic`);
            b.$line(`builder.basic_info();`);
            b.$line(``);

            // bases
            b.$line(`// bases`);
            if (_gen_bases.length > 0) {
              b.$line(`builder.bases<${_gen_bases.join(", ")}>();`);
            }

            // ctors
            b.$line(`// ctors`);
            _gen_ctors.forEach(ctor => {
              b.$line(`{ // ${record.name}(${ctor.dump_params_type_only()})`);
              b.$indent(_b => {
                b.$line(`[[maybe_unused]] auto ctor_builder = builder.ctor<${ctor.dump_params_type_only()}>();`,);

                // build params
                b.$line(`// params`);
                ctor.parameters.forEach((param, idx) => {
                  b.$scope((_b) => {
                    const param_config = param.ml_configs.rttr as ParamConfig;
                    b.$line(`[[maybe_unused]] auto param_builder = ctor_builder.param_at(${idx});`,);
                    b.$line(`param_builder.name(u8"${param.name}");`);
                    b.$line(``);
                    this.#flags_and_attrs(
                      b,
                      param,
                      param_config,
                      "param_builder",
                    );
                  });
                });

                this.#flags_and_attrs(b, ctor, ctor.ml_configs.rttr as CtorConfig, "ctor_builder");
              })
              b.$line(`}`);
            })

            // fields
            b.$line(`// fields`);
            _gen_fields.forEach((field) => {
              const field_config = field.ml_configs.rttr as FieldConfig;
              b.$line(`{ // ${record.name}::${field.short_name}`);
              b.$indent((_b) => {
                if (field.is_static) {
                  b.$line(`[[maybe_unused]] auto field_builder = builder.static_field<&${record.name}::${field.short_name}>(u8"${field.short_name}");`,);
                } else {
                  b.$line(`[[maybe_unused]] auto field_builder = builder.field<&${record.name}::${field.short_name}>(u8"${field.short_name}");`,);
                }

                this.#flags_and_attrs(b, field, field_config, "field_builder");
              });
              b.$line(`}`);
            });

            // methods
            b.$line(`// methods`);
            _gen_methods.forEach((method) => {
              const method_config = method.ml_configs.rttr as MethodConfig;
              b.$line(`{ // ${record.name}::${method.name}`);
              b.$indent((_b) => {
                if (method.is_static) {
                  b.$line(`[[maybe_unused]] auto method_builder = builder.static_method<${method.signature()}, &${record.name}::${method.short_name}>(u8"${method.short_name}");`,);
                } else {
                  b.$line(`[[maybe_unused]] auto method_builder = builder.method<${method.signature()}, &${record.name}::${method.short_name}>(u8"${method.short_name}");`,);
                }

                // build params
                b.$line(`// params`);
                method.parameters.forEach((param, idx) => {
                  b.$scope((_b) => {
                    const param_config = param.ml_configs.rttr as ParamConfig;
                    b.$line(`[[maybe_unused]] auto param_builder = method_builder.param_at(${idx});`,);
                    b.$line(`param_builder.name(u8"${param.name}");`);
                    b.$line(``);
                    this.#flags_and_attrs(
                      b,
                      param,
                      param_config,
                      "param_builder",
                    );
                  });
                });

                this.#flags_and_attrs(
                  b,
                  method,
                  method_config,
                  "method_builder",
                );
              });
              b.$line(`}`);
            });

            // flags & attrs
            this.#flags_and_attrs(b, record, record_config, "builder");
          });
          b.$line(`});`);
        });
        b.$line(`}`);
      });

      // export enums
      _gen_enums.forEach((enum_) => {
        const enum_config = enum_.ml_configs.rttr as EnumConfig;
        const _gen_enum_values = enum_.values.filter((enum_value) => enum_value.ml_configs.rttr.enable);

        // register function
        b.$line(`static void zz_register_enum_${enum_.name.replaceAll('::', '_')}(RTTRType * type){`,);
        b.$indent((_b) => {
          // module info
          b.$line(`// setup module`);
          b.$line(`type->set_module(u8"${module_name}");`);
          b.$line(``);

          // build scope
          b.$line(`// build scope`);
          b.$line(`type->build_enum([&](RTTREnumData* enum_data) {`);
          b.$indent((_b) => {
            // basic
            b.$line(`// reserve`);
            b.$line(`enum_data->items.reserve(${enum_.values.length});`);
            b.$line(``);
            b.$line(`RTTREnumBuilder<${enum_.name}> builder(enum_data);`);
            b.$line(``);
            b.$line(`// basic`);
            b.$line(`builder.basic_info();`);
            b.$line(``);

            // items
            b.$line(`// items`);
            _gen_enum_values.forEach((enum_value) => {
              const enum_value_config = enum_value.ml_configs.rttr as EnumValueConfig;

              b.$line(`{ // ${enum_.name}::${enum_value.name}`);
              b.$indent((_b) => {
                b.$line(`[[maybe_unused]] auto item_builder = builder.item(u8"${enum_value.short_name}", ${enum_value.name});`,);
                b.$line(``);
                this.#flags_and_attrs(b, enum_value, enum_value_config, "item_builder");
              });
              b.$line(`}`);
            });

            this.#flags_and_attrs(b, enum_, enum_config, "builder");
          });
          b.$line(`});`);
        });
        b.$line(`}`);
      });
    }
    b.$line(`}`);

    // static register
    b.$line(`SKR_EXEC_STATIC_CTOR`);
    b.$line(`{`);
    b.$indent(() => {
      b.$line(`using namespace ::skr;`);
      b.$line(``);

      // export records
      _gen_records.forEach((record, _header) => {
        b.$line(`register_type_loader(type_id_of<${record.name}>(), zz_register_record_${record.name.replaceAll('::', '_')});`,);
      });

      // export enums
      _gen_enums.forEach((enum_) => {
        b.$line(`register_type_loader(type_id_of<${enum_.name}> (), zz_register_enum_${enum_.name.replaceAll('::', '_')});`);
      });
    });
    b.$line(`};`);

    // impl script mixin
    b.$line(``);
    in_records.forEach(record => {
      const mixin_methods = record.methods.filter(method => method.ml_configs.rttr.script_mixin);
      if (mixin_methods.length > 0) {
        b.$line(`// mixin methods of ${record.name}`);
        b.$namespace(record.namespace.join("::"), _b => {
          mixin_methods.forEach(method => {
            b.$line(`${method.ret_type} ${record.short_name}::${method.short_name}(${method.dump_params()}){`)
            b.$indent(_b => {
              b.$line(`auto mixin_result = try_invoke_mixin_method<${method.ret_type}${method.dump_params_type_only_with_comma()}>(u8"${method.short_name}"${method.dump_params_name_only_with_comma()});`);

              // has value case
              if (method.ret_type !== "void") {
                b.$line(`if (mixin_result.has_value()) {`)
                b.$indent(_b => {
                  b.$line(`return mixin_result.value();`)
                })
              } else {
                b.$line(`if (mixin_result) {`)
                b.$indent(_b => {
                  b.$line(`return;`)
                })
              }

              // no value case
              b.$line(`} else {`)
              b.$indent(_b => {
                b.$line(`return this->${method.short_name}_impl(${method.dump_params_name_only()});`)
              })
              b.$line(`}`)

            })
            b.$line(`}`)
          })
        })
      }
    });

    // bottom
    b.$line(`// END RTTR GENERATED`);
    b.$line(``);
  }
  static #flags_and_attrs(
    b: CodeBuilder,
    cpp_type: db.CppTypes,
    config: CppConfigTypes,
    builder_name: string,
  ) {
    b.$line(`// flags`);
    if (config.flags.length > 0) {
      b.$line(`${builder_name}.flag(${this.#flags_expr(cpp_type, config.flags)});`,);
    }

    b.$line(`// attrs`);
    if (config.attrs.length > 0) {
      b.$line(`${builder_name}`);
      b.$indent((_b) => {
        config.attrs.forEach((attr) => {
          b.$line(`.attribute(::skr::attr::${attr})`);
        });
      });
      b.$line(`;`);
    }
  }
  static #flag_enum_name_of(cpp_type: db.CppTypes) {
    if (cpp_type instanceof db.Record) {
      return "ERTTRRecordFlag";
    } else if (cpp_type instanceof db.Field) {
      return cpp_type.is_static ? "ERTTRStaticFieldFlag" : "ERTTRFieldFlag";
    } else if (cpp_type instanceof db.Method) {
      return cpp_type.is_static ? "ERTTRStaticMethodFlag" : "ERTTRMethodFlag";
    } else if (cpp_type instanceof db.Parameter) {
      return "ERTTRParamFlag";
    } else if (cpp_type instanceof db.Enum) {
      return "ERTTREnumFlag";
    } else if (cpp_type instanceof db.EnumValue) {
      return "ERTTREnumItemFlag";
    } else if (cpp_type instanceof db.Ctor) {
      return "ERTTRCtorFlag";
    } else {
      throw new Error(`Unknown cpp_type: ${cpp_type.constructor.name}`);
    }
  }
  static #flags_expr(cpp_type: db.CppTypes, flags: string[]) {
    return flags
      .filter((value, index, self) => self.indexOf(value) === index)    // make list unique
      .map((flags) => `${this.#flag_enum_name_of(cpp_type)}::${flags}`) // map to name
      .join(" | ");
  }

  // filters
  static filter_record(record: db.Record) {
    return record.ml_configs.rttr.enable && !record.ml_configs.guid.is_empty();
  }
  static filter_enum(enum_: db.Enum) {
    return enum_.ml_configs.rttr.enable && !enum_.ml_configs.guid.is_empty();
  }
}

class RttrGenerator extends gen.Generator {
  override inject_configs(): void {
    // record
    this.main_module_db.each_record((record) => {
      record.ml_configs.rttr = new RecordConfig(record);
      record.ml_configs.guid = new GuidConfig();

      // ctors
      record.ctors.forEach((ctor) => {
        ctor.ml_configs.rttr = new CtorConfig();

        // params
        ctor.parameters.forEach((param) => {
          param.ml_configs.rttr = new ParamConfig();
        });
      });

      // methods
      record.methods.forEach((method) => {
        method.ml_configs.rttr = new MethodConfig();

        // params
        method.parameters.forEach((param) => {
          param.ml_configs.rttr = new ParamConfig();
        });
      });

      // fields
      record.fields.forEach((field) => {
        field.ml_configs.rttr = new FieldConfig();
      });
    });

    // enums
    this.main_module_db.each_enum((enum_) => {
      enum_.ml_configs.rttr = new EnumConfig();
      enum_.ml_configs.guid = new GuidConfig();
      enum_.values.forEach((enum_value) => {
        enum_value.ml_configs.rttr = new EnumValueConfig();
      });
    });
  }

  override gen_body(): void {
    this.main_module_db.each_record((record, header) => {
      // skip if rttr is disabled
      if (record.ml_configs.rttr.enable === false) return;

      // gen friend for export
      {
        const _gen_fields: db.Field[] = record.fields.filter((field) => field.ml_configs.rttr.enable);
        const _gen_methods: db.Method[] = record.methods.filter((method) => method.ml_configs.rttr.enable);
        const _gen_ctors: db.Ctor[] = record.ctors.filter(ctor => ctor.ml_configs.rttr.enable);
        let _any_private_export = false;
        _gen_fields.forEach((field) => {
          if (field.access === "private") {
            _any_private_export = true;
          }
        });
        _gen_methods.forEach((method) => {
          if (method.access === "private") {
            _any_private_export = true;
          }
        });
        _gen_ctors.forEach((ctor) => {
          if (ctor.access === "private") {
            _any_private_export = true;
          }
        });

        if (_any_private_export) {
          const b = record.generate_body_content;
          b.$line(`friend void ::skr::zz_register_record_${record.name.replaceAll('::', '_')}(struct ::skr::RTTRType* type);`);
        }
      }

      // gen iobject body
      if (this.project_db.is_derived(record, "skr::IObject")) {
        const b = record.generate_body_content;
        b.$line(``);
        b.$line(`::skr::GUID iobject_get_typeid() const override`);
        b.$scope((_b) => {
          b.$line(`using namespace ::skr;`);
          b.$line(
            `using ThisType = std::remove_cv_t<std::remove_pointer_t<decltype(this)>>;`,
          );
          b.$line(`return type_id_of<ThisType>();`);
        });
        b.$line(
          `void* iobject_get_head_ptr() const override { return const_cast<void*>((const void*)this); }`,
        );
        b.$line(``);
      }
    });
  }
  override gen(): void {
    // gen headers
    this.main_module_db.headers.forEach((header) => {
      _Gen.header(header);
    });

    // gen source
    if (this.project_db.config.batch_size === 0) {
      _Gen.source(this.main_module_db);
    } else {
      const main_db = this.main_module_db;
      const _gen_records = main_db.filter_record(_Gen.filter_record);
      const _gen_enums = main_db.filter_enum(_Gen.filter_enum);

      // do batch
      const batch_size = this.project_db.config.batch_size;
      let batch_idx = 1;
      const elem_count = _gen_records.length + _gen_enums.length;
      let elem_idx = 0;
      const _batch_records: db.Record[] = [];
      const _batch_enums: db.Enum[] = [];
      while (elem_idx < elem_count) {
        // clean up batch
        _batch_records.length = 0;
        _batch_enums.length = 0;

        // build batch
        for (let i = 0; i < batch_size; ++i) {
          if (elem_idx >= elem_count) break;

          // push element
          if (elem_idx < _gen_records.length) {
            _batch_records.push(_gen_records[elem_idx]);
          } else {
            _batch_enums.push(_gen_enums[elem_idx - _gen_records.length]);
          }

          // inc element idx
          elem_idx++;
        }

        // output batch
        const b = new CodeBuilder();
        _Gen.source_batched(
          b,
          _batch_records,
          _batch_enums,
          main_db.config.module_name
        );
        main_db.batch_files[`rttr_${batch_idx}`] = b;

        // add batch idx
        batch_idx++;
      }

    }
  }
}

export function load_generator(gm: gen.GenerateManager) {
  gm.add_generator("rttr", new RttrGenerator());
}
