import * as path from "node:path";
import * as fs from "node:fs";
import * as ml from "./meta_lang.ts";
import { CodeBuilder } from "./utils.ts";

//============================================================
// cpp types
//============================================================
// help functions
type NameWithNamespace = {
  name: string;
  short_name: string;
  namespace: string[];
};
function fill_name(obj: NameWithNamespace, name: string) {
  obj.name = name;

  const split_name = name.split("::");
  obj.short_name = split_name[split_name.length - 1];
  obj.namespace = split_name.slice(0, split_name.length - 1);
}

export type CppTypes =
  | Enum
  | EnumValue
  | Record
  | Ctor
  | Field
  | Method
  | Function
  | Parameter;

// enum
export class Enum {
  // name & namespace
  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  // info
  is_scoped: boolean;
  underlying_type: string;
  comment: string;
  file_name: string;
  line: number;
  values: EnumValue[] = [];

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    fill_name(this, json_obj.name);

    // load info
    this.is_scoped = json_obj.is_scoped;
    this.underlying_type = json_obj.underlying_type;
    this.comment = json_obj.comment;
    this.file_name = json_obj.file_name;
    this.line = json_obj.line;

    // load values
    for (const value of json_obj.values) {
      this.values.push(new EnumValue(this, value));
    }

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }
}
export class EnumValue {
  // owner
  parent: Enum;

  // name & namespace
  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  // info
  value: number;
  file_name: string;
  comment: string;
  line: number;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(parent: Enum, json_obj: any) {
    this.parent = parent;

    this.file_name = parent.file_name;

    // parse name
    fill_name(this, json_obj.name);

    // parse json
    this.value = json_obj.value;
    this.comment = json_obj.comment;
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }
}

// record
export class Record {
  // name & namespace
  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  // info
  bases: string[];
  fields: Field[];
  methods: Method[];
  ctors: Ctor[];
  file_name: string;
  line: number;
  comment: string;
  has_generate_body_flag: boolean = false;
  generate_body_flag_mark: string = "";
  generate_body_line: number = 0;
  generate_body_content: CodeBuilder = new CodeBuilder();

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    // parse name
    fill_name(this, json_obj.name);

    // info
    this.bases = json_obj.bases;
    this.comment = json_obj.comment;
    this.file_name = json_obj.file_name;
    this.line = json_obj.line;

    // load fields
    this.fields = [];
    for (const field of json_obj.fields) {
      this.fields.push(new Field(this, field));
    }

    // load methods
    this.methods = [];
    for (const method of json_obj.methods) {
      const short_name = method.name.split("::").pop() as string;
      if (short_name.startsWith("_zz_skr_generate_body_flag_")) {
        this.has_generate_body_flag = true;
        this.generate_body_flag_mark = short_name.substring(
          "_zz_skr_generate_body_flag_".length
        );
        this.generate_body_line = method.line;
      } else {
        this.methods.push(new Method(this, method));
      }
    }

    // load ctors
    this.ctors = [];
    for (const ctor of json_obj.ctors) {
      this.ctors.push(new Ctor(this, ctor));
    }

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }

  dump_generate_body(): string {
    return this.generate_body_content.content_marco;
  }
}
export class Ctor {
  parent: Record;

  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  access: string;

  parameters: Parameter[] = [];

  file_name: string;
  comment: string;
  line: string;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(parent: Record, json_obj: any) {
    this.parent = parent;
    this.file_name = parent.file_name;

    fill_name(this, json_obj.name);

    this.access = json_obj.access;
    this.comment = json_obj.comment;

    // load parameters
    for (const parameter of json_obj.parameters) {
      this.parameters.push(new Parameter(this, parameter));
    }

    // load return type
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }

  dump_params_type_only() {
    return this.parameters.map((param) => param.type).join(", ");
  }
}
export class Field {
  parent: Record;

  short_name: string;

  type: string;
  raw_type: string;
  access: string;
  default_value: string;
  array_size: number;
  is_functor: boolean;
  is_static: boolean;
  is_anonymous: boolean;
  file_name: string;
  comment: string;
  line: number;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(parent: Record, json_obj: any) {
    this.parent = parent;
    this.file_name = parent.file_name;

    this.short_name = json_obj.name;

    this.type = json_obj.type;
    this.raw_type = json_obj.raw_type;
    this.access = json_obj.access;
    this.default_value = json_obj.default_value;
    this.array_size = json_obj.array_size;
    this.is_functor = json_obj.is_functor;
    this.is_static = json_obj.is_static;
    this.is_anonymous = json_obj.is_anonymous;
    this.comment = json_obj.comment;
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }

  signature() {
    return this.array_size > 0 ? `${this.type}[${this.array_size}]` : this.type;
  }
}
export class Method {
  parent: Record;

  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  access: string;
  is_static: boolean;
  is_const: boolean;
  is_nothrow: boolean;
  file_name: string;
  comment: string;
  parameters: Parameter[] = [];
  ret_type: string;
  raw_ret_type: string;
  line: string;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(parent: Record, json_obj: any) {
    this.parent = parent;

    this.file_name = parent.file_name;

    fill_name(this, json_obj.name);

    this.access = json_obj.access;
    this.is_static = json_obj.is_static;
    this.is_const = json_obj.is_const;
    this.is_nothrow = json_obj.is_nothrow;
    this.comment = json_obj.comment;

    // load parameters
    for (const parameter of json_obj.parameters) {
      this.parameters.push(new Parameter(this, parameter));
    }

    // load return type
    this.ret_type = json_obj.ret_type;
    this.raw_ret_type = json_obj.raw_ret_type;
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }

  dump_params() {
    return this.parameters.map((param) => `${param.type} ${param.name}`).join(
      ", ",
    );
  }
  dump_params_with_comma() {
    const params = this.dump_params();
    return params.length > 0 ? `, ${params}` : "";
  }
  dump_params_name_only() {
    return this.parameters.map((param) => param.name).join(", ");
  }
  dump_params_name_only_with_comma() {
    const params = this.dump_params_name_only();
    return params.length > 0 ? `, ${params}` : "";
  }
  dump_params_type_only() {
    return this.parameters.map((param) => param.type).join(", ");
  }
  dump_params_type_only_with_comma() {
    const params = this.dump_params_type_only();
    return params.length > 0 ? `, ${params}` : "";
  }
  dump_const() {
    return this.is_const ? " const" : "";
  }
  dump_noexcept() {
    return this.is_nothrow ? " noexcept" : "";
  }
  dump_modifiers() {
    let modifier = "";
    if (this.is_const) modifier += " const";
    if (this.is_nothrow) modifier += " noexcept";
    return modifier;
  }

  has_return() {
    return this.ret_type != "void";
  }

  signature() {
    return this.is_static
      ? `${this.ret_type}(*)(${this.dump_params_type_only()})`
      : `${this.ret_type}(${this.parent.name}::*)(${this.dump_params_type_only()}) ${this.dump_const()}`;
  }
}
export class Parameter {
  parent: Method | Function | Ctor;

  name: string;
  type: string;
  array_size: number;
  raw_type: string;
  is_functor: boolean;
  is_callback: boolean;
  is_anonymous: boolean;
  default_value: string;
  file_name: string;
  comment: string;
  line: number;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(parent: Method | Function | Ctor, json_obj: any) {
    this.parent = parent;
    this.file_name = parent.file_name;

    this.name = json_obj.name;
    this.type = json_obj.type;
    this.array_size = json_obj.array_size;
    this.raw_type = json_obj.raw_type;
    this.is_functor = json_obj.is_functor;
    this.is_callback = json_obj.is_callback;
    this.is_anonymous = json_obj.is_anonymous;
    this.default_value = json_obj.default_value;
    this.comment = json_obj.comment;
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }
}
export class Function {
  name: string = "";
  short_name: string = "";
  namespace: string[] = [];

  is_static: boolean;
  is_const: boolean;
  comment: string;
  parameters: Parameter[] = [];
  ret_type: string;
  raw_ret_type: string;
  file_name: string;
  line: number;

  // attrs
  raw_attrs: string[] = [];
  ml_programs: ml.Program[] = [];
  ml_configs: Dict<any> = {};

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    fill_name(this, json_obj.name);

    this.is_static = json_obj.is_static;
    this.is_const = json_obj.is_const;
    this.comment = json_obj.comment;

    // load parameters
    for (const parameter of json_obj.parameters) {
      this.parameters.push(new Parameter(this, parameter));
    }

    // load return type
    this.ret_type = json_obj.ret_type;
    this.raw_ret_type = json_obj.raw_ret_type;
    this.file_name = json_obj.file_name;
    this.line = json_obj.line;

    // load attrs
    this.raw_attrs = json_obj.attrs;
  }
}

//============================================================
// data base config
//============================================================
export class ModuleConfig {
  module_name: string;
  meta_dir: string;
  api: string;

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    this.module_name = json_obj.module_name;
    this.meta_dir = json_obj.meta_dir;
    this.api = json_obj.api;
  }
}
export class GeneratorConfig {
  entry_file: string;
  import_dir: string[];

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    this.entry_file = json_obj.entry_file;
    this.import_dir = json_obj.import === undefined ? [] : json_obj.import;
  }
}
export class CodegenConfig {
  output_dir: string;
  main_module: ModuleConfig;
  include_modules: ModuleConfig[] = [];
  generators: GeneratorConfig[] = [];
  batch_size: number = 0; // 0 means no batch

  // deno-lint-ignore no-explicit-any
  constructor(json_obj: any) {
    this.output_dir = json_obj.output_dir;

    // load main module
    this.main_module = new ModuleConfig(json_obj.main_module);

    // load include modules
    if (json_obj.include_modules !== undefined) {
      for (const module of json_obj.include_modules) {
        this.include_modules.push(new ModuleConfig(module));
      }
    }

    // load generators
    if (json_obj.generators !== undefined) {
      for (const generator of json_obj.generators) {
        this.generators.push(new GeneratorConfig(generator));
      }
    }

    // load custom config
    if (typeof json_obj.batch_size === "number") {
      this.batch_size = json_obj.batch_size;
    } else {
      throw new Error(`batch size must be a number`)
    }
  }
}

//============================================================
// project unit
//============================================================
export class Header {
  parent: Module;

  // meta source info
  meta_path: string = "";
  meta_path_relative: string = "";

  // codegen info
  file_id: string = ""; // 用于 codegen 时候定义 FILE_ID 宏
  output_header_path: string = ""; // 用于 codegen 时输出的 header
  header_path: string = ""; // 用于 codegen 时 include 对应的 header

  // data
  records: Record[] = [];
  enums: Enum[] = [];
  functions: Function[] = [];

  gen_code: CodeBuilder = new CodeBuilder();
  gen_code_custom: Dict<CodeBuilder> = {};

  constructor(parent: Module, meta_file_path: string) {
    this.parent = parent;

    // solve meta path
    this.meta_path = path.normalize(meta_file_path).replaceAll(
      path.sep,
      "/",
    );
    this.meta_path_relative = path.relative(
      parent.config.meta_dir,
      this.meta_path,
    );

    // solve output header path and file id
    const reg_non_alpha_ch = /\W+/g;
    this.file_id = `FID_${parent.config.module_name}_${this.meta_path_relative.replaceAll(reg_non_alpha_ch, "_")
      }`;
    const reg_meta_path = /(.*?)\.(.*?)\.meta/g;
    this.output_header_path = this.meta_path_relative.replace(
      reg_meta_path,
      "$1.generated.$2",
    );

    // load json
    const json_obj = JSON.parse(
      fs.readFileSync(this.meta_path, { encoding: "utf-8" }),
    );
    for (const record_json of json_obj.records) {
      this.records.push(new Record(record_json));
    }
    for (const enum_json of json_obj.enums) {
      this.enums.push(new Enum(enum_json));
    }
    for (const function_json of json_obj.functions) {
      this.functions.push(new Function(function_json));
    }

    // solve include path
    const solve_include_path = (file_name: string) => {
      file_name = file_name.replaceAll(path.sep, "/");
      if (this.header_path && this.header_path !== file_name) {
        throw new Error(
          `header path is not same, ${this.header_path} != ${file_name}`,
        );
      }
      this.header_path = file_name;
    };
    for (const record of this.records) {
      solve_include_path(record.file_name);
    }
    for (const enum_obj of this.enums) {
      solve_include_path(enum_obj.file_name);
    }
    for (const function_obj of this.functions) {
      solve_include_path(function_obj.file_name);
    }
  }

  // helper
  get_header_path_of_suffix(suffix: string) {
    const reg_meta_path = /(.*?)\.(.*?)\.meta/g;
    return this.meta_path_relative.replace(
      reg_meta_path,
      `$1.${suffix}.generated.$2`,
    );
  }

  // find
  find_record(name: string) {
    for (const record of this.records) {
      if (record.name === name) {
        return record;
      }
    }
    return null;
  }
  find_enum(name: string) {
    for (const enum_obj of this.enums) {
      if (enum_obj.name === name) {
        return enum_obj;
      }
    }
    return null;
  }
  find_function(name: string) {
    for (const function_obj of this.functions) {
      if (function_obj.name === name) {
        return function_obj;
      }
    }
    return null;
  }

  // each functions
  each_record(
    func: (record: Record, header: Header) => void,
  ) {
    for (const record of this.records) {
      func(record, this);
    }
  }
  each_enum(
    func: (enum_obj: Enum, header: Header) => void,
  ) {
    for (const enum_obj of this.enums) {
      func(enum_obj, this);
    }
  }
  each_function(
    func: (function_obj: Function, header: Header) => void,
  ) {
    for (const function_obj of this.functions) {
      func(function_obj, this);
    }
  }
  each_cpp_types(
    func: (cpp_type: CppTypes, header: Header) => void,
  ) {
    for (const record of this.records) {
      func(record, this);

      for (const ctor of record.ctors) {
        func(ctor, this);
        for (const param of ctor.parameters) {
          func(param, this);
        }
      }

      for (const method of record.methods) {
        func(method, this);
        for (const param of method.parameters) {
          func(param, this);
        }
      }

      for (const field of record.fields) {
        func(field, this);
      }
    }
    for (const enum_obj of this.enums) {
      func(enum_obj, this);

      for (const value of enum_obj.values) {
        func(value, this);
      }
    }
    for (const function_obj of this.functions) {
      func(function_obj, this);
      for (const param of function_obj.parameters) {
        func(param, this);
      }
    }
  }
}

export class Module {
  parent: Project;
  config: ModuleConfig;
  headers: Header[] = [];

  // source info
  pre_all_file: CodeBuilder = new CodeBuilder();
  post_all_file: CodeBuilder = new CodeBuilder();
  main_file: CodeBuilder = new CodeBuilder();
  batch_files: Dict<CodeBuilder> = {};

  constructor(parent: Project, config: ModuleConfig) {
    this.parent = parent;
    this.config = config;

    // check meta dir
    if (!fs.existsSync(config.meta_dir)) {
      throw new Error(`meta dir not exist "${config.meta_dir}"`);
      // return
    }

    // glob meta files
    const meta_files = fs.globSync(
      path.join(config.meta_dir, "**", "*.h.meta"),
    );

    // load headers
    for (const meta_file of meta_files) {
      this.headers.push(new Header(this, meta_file));
    }
  }

  // find
  find_record(name: string) {
    for (const header of this.headers) {
      const record = header.find_record(name);
      if (record) {
        return record;
      }
    }
    return null;
  }
  find_enum(name: string) {
    for (const header of this.headers) {
      const enum_obj = header.find_enum(name);
      if (enum_obj) {
        return enum_obj;
      }
    }
    return null;
  }
  find_function(name: string) {
    for (const header of this.headers) {
      const function_obj = header.find_function(name);
      if (function_obj) {
        return function_obj;
      }
    }
    return null;
  }

  // each functions
  each_record(
    func: (record: Record, header: Header) => void,
  ) {
    for (const header of this.headers) {
      header.each_record(func);
    }
  }
  each_enum(
    func: (enum_obj: Enum, header: Header) => void,
  ) {
    for (const header of this.headers) {
      header.each_enum(func);
    }
  }
  each_function(
    func: (function_obj: Function, header: Header) => void,
  ) {
    for (const header of this.headers) {
      header.each_function(func);
    }
  }
  each_cpp_types(
    func: (cpp_type: CppTypes, header: Header) => void,
  ) {
    for (const header of this.headers) {
      header.each_cpp_types(func);
    }
  }

  // filter
  filter_record(
    func: (record: Record, header: Header) => boolean,
  ) {
    const result: Record[] = [];
    this.each_record((record, header) => {
      if (func(record, header)) {
        result.push(record);
      }
    });
    return result;
  }
  filter_enum(
    func: (enum_obj: Enum, header: Header) => boolean,
  ) {
    const result: Enum[] = [];
    this.each_enum((enum_obj, header) => {
      if (func(enum_obj, header)) {
        result.push(enum_obj);
      }
    });
    return result;
  }
  filter_function(
    func: (function_obj: Function, header: Header) => boolean,
  ) {
    const result: Function[] = [];
    this.each_function((function_obj, header) => {
      if (func(function_obj, header)) {
        result.push(function_obj);
      }
    });
    return result;
  }
}

export class Project {
  main_module: Module;
  include_modules: Module[] = [];
  config: CodegenConfig;

  constructor(
    config: CodegenConfig,
    load_include_module: boolean = false,
  ) {
    this.config = config;

    // load main module
    this.main_module = new Module(this, config.main_module);

    // load include modules
    if (load_include_module) {
      for (const include_module of config.include_modules) {
        this.include_modules.push(new Module(this, include_module));
      }
    }
  }

  // find
  find_record(name: string) {
    let record = this.main_module.find_record(name);
    if (record) {
      return record;
    }
    for (const include_module of this.include_modules) {
      record = include_module.find_record(name);
      if (record) {
        return record;
      }
    }
    return null;
  }
  find_enum(name: string) {
    let enum_obj = this.main_module.find_enum(name);
    if (enum_obj) {
      return enum_obj;
    }
    for (const include_module of this.include_modules) {
      enum_obj = include_module.find_enum(name);
      if (enum_obj) {
        return enum_obj;
      }
    }
    return null;
  }
  find_function(name: string) {
    let function_obj = this.main_module.find_function(name);
    if (function_obj) {
      return function_obj;
    }
    for (const include_module of this.include_modules) {
      function_obj = include_module.find_function(name);
      if (function_obj) {
        return function_obj;
      }
    }
    return null;
  }

  // each functions
  each_record(
    func: (record: Record, header: Header) => void,
  ) {
    this.main_module.each_record(func);
    for (const include_module of this.include_modules) {
      include_module.each_record(func);
    }
  }
  each_enum(
    func: (enum_obj: Enum, header: Header) => void,
  ) {
    this.main_module.each_enum(func);
    for (const include_module of this.include_modules) {
      include_module.each_enum(func);
    }
  }
  each_function(
    func: (function_obj: Function, header: Header) => void,
  ) {
    this.main_module.each_function(func);
    for (const include_module of this.include_modules) {
      include_module.each_function(func);
    }
  }
  each_cpp_types(
    func: (cpp_type: CppTypes, header: Header) => void,
  ) {
    this.main_module.each_cpp_types(func);
    for (const include_module of this.include_modules) {
      include_module.each_cpp_types(func);
    }
  }

  // tool functions
  is_derived(record: Record, base: string) {
    for (const base_name of record.bases) {
      if (base_name === base) {
        return true;
      }
      const base_record = this.find_record(base_name);
      if (base_record != null && this.is_derived(base_record, base)) {
        return true;
      }
    }
  }
  is_derived_or_same(record: Record, base: string) {
    return record.name == base || this.is_derived(record, base);
  }
  get_all_bases(record: Record) {
    const bases: string[] = [];
    for (const base_name of record.bases) {
      bases.push(base_name);
      const base_record = this.find_record(base_name);
      if (base_record != null) {
        bases.push(...this.get_all_bases(base_record));
      }
    }
    return bases;
  }
}
