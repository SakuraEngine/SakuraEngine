import {
  CodeBuilder, dims_no_scalar,
  type_convert_options, type_options,
  get_alignas_vector, vector_has_simd_optimize
} from "./util"
import type { TypeOption, GlobalBuilders } from "./util";
import path from "node:path";

const _comp_lut = ['x', 'y', 'z', 'w'];
const _arithmetic_ops = ['+', '-', '*', '/', '%']
const _compare_ops = ['==', '!=', '<', '<=', '>', '>=']
const _boolean_ops = ['&&', '||', '==', '!=']

export interface GenVectorOption extends TypeOption, GlobalBuilders {
  builder: CodeBuilder;
  base_name: string    // [basename][2/3/4]
};

// generate ctor
function _gen_ctor(dim: number, opt: GenVectorOption) {
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const vec_name = `${base_name}${dim}`;

  // default ctor
  const default_init = _comp_lut
    .slice(0, dim)
    .map(comp => `${comp}(0)`)
    .join(", ");
  b.$line(`inline ${vec_name}(): ${default_init} {}`);

  // no init ctor
  b.$line(`inline ${vec_name}(MathNoInitType) {}`)

  // scalar ctor
  const scalar_init = _comp_lut
    .slice(0, dim)
    .map(comp => `${comp}(v)`)
    .join(", ");
  b.$line(`inline ${vec_name}(${comp_name} v): ${scalar_init} {}`);

  // swizzle ctors
  _gen_ctor_swizzle_recursive(
    [],
    dim,
    dim,
    opt
  )
}
function _gen_ctor_swizzle_recursive(
  param_dims: number[],
  remain_dim: number,
  dim: number,
  opt: GenVectorOption
) {
  if (remain_dim !== 0) {
    // do recursive
    for (let i = 1; i <= remain_dim; ++i) {
      if (i == dim) continue; // skip copy ctor
      param_dims.push(i)
      _gen_ctor_swizzle_recursive(
        param_dims,
        remain_dim - i,
        dim,
        opt
      )
      param_dims.pop()
    }
  } else {
    const b = opt.builder;
    const base_name = opt.base_name;
    const comp_name = opt.component_name;
    const vec_name = `${base_name}${dim}`;

    // generate params
    let cur_idx = 0;
    const params = param_dims
      .map((len, i) => {
        let param_suffix = ""
        for (let j = 0; j < len; ++j) {
          param_suffix += _comp_lut[cur_idx + j]
        }

        cur_idx += len;

        return len == 1 ?
          `${comp_name} v_${param_suffix}` :
          `${base_name}${len} v_${param_suffix}`
      })
      .join(", ");

    // generate init list
    cur_idx = 0;
    let cur_comp = 0;
    const init_list = param_dims
      .flatMap((len, i) => {
        let param_suffix = ""
        for (let j = 0; j < len; ++j) {
          param_suffix += _comp_lut[cur_idx + j]
        }

        const result = []
        for (let j = 0; j < len; ++j) {
          const visitor = len != 1 ? `.${_comp_lut[j]}` : ``;
          result.push(`${_comp_lut[cur_comp]}(v_${param_suffix}${visitor})`)
          ++cur_comp
        }

        cur_idx += len;
        return result
      })
      .join(", ");

    // dump ctor
    b.$line(`inline ${vec_name}(${params}): ${init_list} {}`);
  }
}

// generate swizzle
function _gen_swizzle(dim: number, opt: GenVectorOption) {
  for (let out_dim = 2; out_dim <= dim; ++out_dim) {
    _gen_swizzle_recursive(
      [],
      out_dim,
      out_dim,
      dim,
      opt
    )
  }
}
function _gen_swizzle_recursive(
  comps: number[],
  remain_dim: number,
  out_dim: number,
  usable_dim: number,
  opt: GenVectorOption
) {
  if (remain_dim !== 0) {
    // do recursive
    for (let i = 0; i < usable_dim; ++i) {
      comps.push(i)
      _gen_swizzle_recursive(
        comps,
        remain_dim - 1,
        out_dim,
        usable_dim,
        opt
      )
      comps.pop()
    }
  } else {
    const b = opt.builder;
    const base_name = opt.base_name;
    const comp_name = opt.component_name;
    const out_vec_name = `${base_name}${out_dim}`;

    // gen getter
    const get_swizzle_name = comps
      .map(comp => _comp_lut[comp])
      .join("");
    const get_init_list = comps
      .map(comp => _comp_lut[comp])
      .join(", ");

    b.$line(`inline ${out_vec_name} ${get_swizzle_name}() const { return {${get_init_list}}; }`);

    {
      let is_unique = true;
      const seen_comp: number[] = [];
      for (const comp of comps) {
        if (seen_comp.includes(comp)) {
          is_unique = false;
          break;
        } else {
          seen_comp.push(comp);
        }
      }

      if (is_unique) {
        const set_swizzle_name = "set_" + comps
          .map(comp => _comp_lut[comp])
          .join("");
        const set_assign_list = comps
          .map((comp, i) => `${_comp_lut[comp]} = v.${_comp_lut[i]};`)
          .join(" ");

        b.$line(`inline void ${set_swizzle_name}(const ${out_vec_name}& v) { ${set_assign_list} }`);
      }
    }
  }
}

// generate operators
function _gen_arithmetic_operator(dim: number, opt: GenVectorOption) {
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const vec_name = `${base_name}${dim}`;
  const comp_kind = opt.component_kind;
  const is_signed = opt.is_signed;

  // unary operator
  if (comp_kind == "floating" || comp_kind == "integer") {
    b.$line(`// unary operator`);

    // neg
    if (is_signed) {
      const init_list = _comp_lut
        .slice(0, dim)
        .map(comp => `-${comp}`)
        .join(", ");
      b.$line(`inline ${vec_name} operator-() const { return { ${init_list} }; }`);
    }

    b.$line(``);
  } else {
    b.$line(`// unary operator`);

    // not
    const init_list = _comp_lut
      .slice(0, dim)
      .map(comp => `!${comp}`)
      .join(", ");
    b.$line(`inline ${vec_name} operator!() const { return { ${init_list} }; }`);

    b.$line(``);
  }

  // arithmetic operator
  if (comp_kind == "floating" || comp_kind == "integer") {
    b.$line(`// arithmetic operator`);
    for (const op of _arithmetic_ops) {
      let init_list
      if (op === '%' && comp_kind == "floating") {
        // const fmod_name = comp_name === "double" ? "fmod" : "fmodf";
        const fmod_name = "fmod"
        init_list = _comp_lut
          .slice(0, dim)
          .map(comp => `::std::${fmod_name}(lhs.${comp}, rhs.${comp})`)
          .join(", ");
      } else {
        init_list = _comp_lut
          .slice(0, dim)
          .map(comp => `lhs.${comp} ${op} rhs.${comp}`)
          .join(", ");
      }
      b.$line(`inline friend ${vec_name} operator${op}(const ${vec_name}& lhs, const ${vec_name}& rhs) { return { ${init_list} }; }`);
    }
    b.$line(``);
  }

  // arithmetic assign operator
  if (comp_kind == "floating" || comp_kind == "integer") {
    b.$line(`// arithmetic assign operator`);
    for (const op of _arithmetic_ops) {
      let assign_exprs
      if (op === "%" && comp_kind == "floating") {
        // const fmod_name = comp_name === "double" ? "fmod" : "fmodf";
        const fmod_name = "fmod"
        assign_exprs = _comp_lut
          .slice(0, dim)
          .map(comp => `${comp} = ::std::${fmod_name}(${comp}, rhs.${comp})`)
          .join(", ");
      } else {
        assign_exprs = _comp_lut
          .slice(0, dim)
          .map(comp => `${comp} ${op}= rhs.${comp}`)
          .join(", ");
      }

      b.$line(`inline ${vec_name}& operator${op}=(const ${vec_name}& rhs) { ${assign_exprs}; return *this; }`);
    }
    b.$line(``);
  }
}

// generate class body
function _gen_class_body(opt: GenVectorOption) {
  const fwd_b = opt.fwd_builder;
  const c_decl_cpp_b = opt.c_decl_cpp_builder;
  const c_decl_c_b = opt.c_decl_c_builder;
  const traits_b = opt.traits_builder;
  const memory_traits_b = opt.memory_traits_builder;
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const comp_kind = opt.component_kind;

  // generate forward declaration
  fwd_b.$line(`// ${base_name} vector, component: ${comp_name}`);
  for (const dim of dims_no_scalar) {
    fwd_b.$line(`struct ${base_name}${dim};`);
  }
  fwd_b.$line(``);

  // generate c decl in cpp
  c_decl_cpp_b.$line(`// ${base_name} vector, component: ${comp_name}`);
  for (const dim of dims_no_scalar) {
    c_decl_cpp_b.$line(`using skr_${base_name}${dim}_t = ::skr::math::${base_name}${dim};`);
  }
  c_decl_cpp_b.$line(``);

  // generate c decl in c
  c_decl_c_b.$line(`// ${base_name} vector, component: ${comp_name}`);
  for (const dim of dims_no_scalar) {
    c_decl_c_b.$line(`typedef struct ${get_alignas_vector(opt, dim)}skr_${base_name}${dim}_t{`);
    c_decl_c_b.$indent(_b => {
      const comp_decls = _comp_lut
        .slice(0, dim)
        .join(", ");
      c_decl_c_b.$line(`${comp_name} ${comp_decls};`);
    })
    c_decl_c_b.$line(`} skr_${base_name}${dim}_t;`);
  }
  c_decl_c_b.$line(``);

  // generate vector traits
  traits_b.$line(`// ${base_name} vector, component: ${comp_name}`);
  for (const dim of dims_no_scalar) {
    // generate maker
    traits_b.$line(`template<> struct MathVectorMaker<${comp_name}, ${dim}> {`);
    traits_b.$indent(b => {
      traits_b.$line(`using Type = ${base_name}${dim};`);
    })
    traits_b.$line(`};`)

    // generate traits
    traits_b.$line(`template<> struct MathVectorTraits<${base_name}${dim}> {`);
    traits_b.$indent(b => {
      traits_b.$line(`inline static constexpr size_t kDimensions = ${dim};`);
      traits_b.$line(`using ComponentType = ${comp_name};`);
      traits_b.$line(`using ImplementationFlag = void;`);
    })
    traits_b.$line(`};`)
  }
  traits_b.$line(``);

  // generate memory traits
  memory_traits_b.$line(`// ${base_name} vector, component: ${comp_name}`);
  for (const dim of dims_no_scalar) {
    memory_traits_b.$line(`template<>`);
    memory_traits_b.$line(`struct MemoryTraits<::skr::math::${base_name}${dim}> : MemoryTraitsPOD {`);
    memory_traits_b.$line(`};`);
  }
  memory_traits_b.$line(``);

  // generate class body
  for (const dim of dims_no_scalar) {
    const vec_name = `${base_name}${dim}`;

    b.$line(`struct ${get_alignas_vector(opt, dim)}${vec_name} {`);
    b.$indent(b => {
      // fill components
      b.$line(`${comp_name} ${_comp_lut.slice(0, dim).join(", ")};`)
      b.$line(``)

      // ctor
      b.$line(`// ctor & dtor`);
      _gen_ctor(dim, opt);
      b.$line(`inline ~${vec_name}() = default;`);
      b.$line(``)

      // cast ctor
      {
        b.$line(`// cast ctor`)
        const convert_option = type_convert_options[base_name]!;
        if (convert_option === undefined) { throw Error(`convert option of '${base_name}' not found`) }
        for (const rhs_base_name in type_options) {
          if (rhs_base_name === base_name) continue; // skip self cast
          if (!convert_option.accept_list.includes(rhs_base_name)) continue; // skip not accept type
          const is_implicit = convert_option.implicit_list.includes(rhs_base_name); // check if implicit cast
          const explicit_decl = is_implicit ? "" : "explicit ";

          b.$line(`${explicit_decl}${vec_name}(const ${rhs_base_name}${dim}& rhs);`)
        }
        b.$line(``)
      }

      // directions factory
      if (dim === 3 && comp_kind === "floating") {
        b.$line(`// directions factory`);
        b.$line(`inline static ${vec_name} forward() { return {0, 0, 1}; }`);
        b.$line(`inline static ${vec_name} back() { return {0, 0, -1}; }`);
        b.$line(`inline static ${vec_name} right() { return {1, 0, 0}; }`);
        b.$line(`inline static ${vec_name} left() { return {-1, 0, 0}; }`);
        b.$line(`inline static ${vec_name} up() { return {0, 1, 0}; }`);
        b.$line(`inline static ${vec_name} down() { return {0, -1, 0}; }`);
        b.$line(``)
      }

      // copy & move & assign & move assign
      b.$line(`// copy & move & assign & move assign`);
      b.$line(`inline ${vec_name}(const ${vec_name}&) = default;`);
      b.$line(`inline ${vec_name}(${vec_name}&&) = default;`);
      b.$line(`inline ${vec_name}& operator=(const ${vec_name}&) = default;`);
      b.$line(`inline ${vec_name}& operator=(${vec_name}&&) = default;`);
      b.$line(``)

      // array assessor
      b.$line(`// array assessor`);
      b.$line(`inline ${comp_name}& operator[](size_t i) {`);
      b.$indent(b => {
        b.$line(`SKR_ASSERT(i >= 0 && i < ${dim} && "index out of range");`);
        b.$line(`return reinterpret_cast<${comp_name}*>(this)[i];`);
      })
      b.$line(`}`);
      b.$line(`inline ${comp_name} operator[](size_t i) const {`);
      b.$indent(b => {
        b.$line(`return const_cast<${vec_name}*>(this)->operator[](i);`);
      })
      b.$line(`}`);
      b.$line(``);

      // arithmetic operators
      _gen_arithmetic_operator(dim, opt);

      // boolean & compare operators
      if (comp_kind === "boolean") {
        b.$line(`// boolean operator`);
        for (const op of _boolean_ops) {
          const compare_exprs = _comp_lut
            .slice(0, dim)
            .map(comp => `lhs.${comp} ${op} rhs.${comp}`)
            .join(", ");
          b.$line(`inline friend ${vec_name} operator${op}(const ${vec_name}& lhs, const ${vec_name}& rhs) { return {${compare_exprs}}; }`);
        }
        b.$line(``);
      } else {
        b.$line(`// compare operator`);
        for (const op of _compare_ops) {
          b.$line(`friend bool${dim} operator${op}(const ${vec_name}& lhs, const ${vec_name}& rhs);`);
        }
        b.$line(``);
      }

      // swizzle
      b.$line(`// swizzle`);
      _gen_swizzle(dim, opt);
      b.$line(``);

      // hash
      b.$line(`// hash`);
      b.$line(`inline static skr_hash _skr_hash(const ${vec_name}& v) {`);
      b.$indent(b => {
        b.$line(`auto hasher = ::skr::Hash<${comp_name}>{};`)
        b.$line(`auto result = hasher(v.${_comp_lut[0]});`);
        for (let i = 1; i < dim; ++i) {
          b.$line(`result = ::skr::hash_combine(result, hasher(v.${_comp_lut[i]}));`);
        }
        b.$line(`return result;`);
      })
      b.$line(`}`)
    })
    b.$line(`};`);
  }
}

// generate compare operators
function _gen_compare_operator(compare_builder: CodeBuilder) {
  const b = compare_builder;

  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;
    const comp_name = type_opt.component_name;
    if (type_opt.component_kind === "boolean") continue; // skip boolean type

    for (const dim of dims_no_scalar) {
      const vec_name = `${base_name}${dim}`;
      const boolean_vec_name = `bool${dim}`;

      b.$line(`// compare operator for [${vec_name}]`);
      for (const op of _compare_ops) {
        const compare_exprs = _comp_lut
          .slice(0, dim)
          .map(comp => `lhs.${comp} ${op} rhs.${comp}`)
          .join(", ")

        b.$line(`inline ${boolean_vec_name} operator${op}(const ${vec_name}& lhs, const ${vec_name}& rhs) { return {${compare_exprs}}; }`);
      }
      b.$line(``);
    }
  }
}

// generate convert constructors
function _gen_covert_operator(compare_builder: CodeBuilder) {
  const b = compare_builder;

  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;
    const comp_name = type_opt.component_name;

    for (const dim of dims_no_scalar) {
      const vec_name = `${base_name}${dim}`;
      const convert_opt = type_convert_options[base_name]!;

      b.$line(`// convert operator for [${vec_name}]`);
      for (const from_base_name in type_options) {
        if (from_base_name === base_name) continue; // skip self cast
        if (!convert_opt.accept_list.includes(from_base_name)) continue; // skip not accept type
        const from_type_opt = type_options[from_base_name]!;
        const from_vec_name = `${from_base_name}${dim}`;

        const init_list = _comp_lut
          .slice(0, dim)
          .map(comp => `${comp}(static_cast<${comp_name}>(rhs.${comp}))`)
          .join(", ")

        b.$line(`inline ${vec_name}::${vec_name}(const ${from_vec_name}& rhs) : ${init_list} {}`)
      }
      b.$line(``);
    }
  }
}

export function gen(global_builders: GlobalBuilders, gen_dir: string) {
  const inc_builder = new CodeBuilder()
  inc_builder.$util_header();

  // generate class body
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // header
    let vector_builder = new CodeBuilder()
    vector_builder.$util_header();

    // include
    vector_builder.$line(`#include <cstdint>`);
    vector_builder.$line(`#include <cmath>`);
    vector_builder.$line(`#include "../gen_math_fwd.hpp"`);
    vector_builder.$line(`#include "../../math_constants.hpp"`);
    vector_builder.$line(`#include <SkrBase/misc/debug.h>`);
    vector_builder.$line(`#include <SkrBase/misc/hash.hpp>`);
    vector_builder.$line(``);

    // gen code
    vector_builder.$line(`namespace skr {`)
    vector_builder.$line(`inline namespace math {`);
    _gen_class_body({
      builder: vector_builder,
      base_name: base_name,
      ...type_opt,
      ...global_builders
    })
    vector_builder.$line(`}`);
    vector_builder.$line(`}`);

    // write to file
    const out_path = path.join(gen_dir, `${base_name}_vec.hpp`);
    vector_builder.write_file(out_path);
    inc_builder.$line(`#include "./${base_name}_vec.hpp"`);
  }

  // get has boolean type
  const has_boolean_type = Object.values(type_options).find(
    t => t!.component_kind === "boolean") !== undefined;

  // generate compare & convert
  if (has_boolean_type) {
    // header
    let compare_op_builder = new CodeBuilder()
    compare_op_builder.$util_header();

    // include
    for (const base_name in type_options) {
      const type_opt = type_options[base_name]!;
      compare_op_builder.$line(`#include "${base_name}_vec.hpp"`);
    }
    compare_op_builder.$line(``);

    compare_op_builder.$line(`namespace skr {`)
    compare_op_builder.$line(`inline namespace math {`);

    // compare operator
    _gen_compare_operator(compare_op_builder)

    // convert operator
    _gen_covert_operator(compare_op_builder)

    compare_op_builder.$line(`}`)
    compare_op_builder.$line(`}`)

    // write to file
    const out_path = path.join(gen_dir, `vec_compare_and_convert.hpp`);
    compare_op_builder.write_file(out_path);
    inc_builder.$line(`#include "./vec_compare_and_convert.hpp"`);
  }

  // write include file
  const inc_path = path.join(gen_dir, `gen_vector.hpp`);
  inc_builder.write_file(inc_path);
}