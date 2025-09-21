import {
  CodeBuilder, type_convert_options,
  type_options, all_component_kinds,
  dims_all, dims_no_scalar,
  matrix_dims, get_alignas_matrix,
  filter_matrix_comp_kind
} from "./util"
import type { TypeOption, ComponentKind, GlobalBuilders } from "./util";
import path from "node:path";

const _axis_lut = ["x", "y", "z", "w"]
const _comp_lut = ["x", "y", "z", "w"]
const _dir_lut = ["right", "up", "forward", "translation"]

interface GenMatrixOption extends TypeOption, GlobalBuilders {
  builder: CodeBuilder;
  base_name: string    // [basename][2/3/4]
};

function _gen_class_body(opt: GenMatrixOption) {
  const fwd_b = opt.fwd_builder;
  const c_decl_cpp_b = opt.c_decl_cpp_builder;
  const c_decl_c_b = opt.c_decl_c_builder;
  const traits_b = opt.traits_builder;
  const memory_traits_b = opt.memory_traits_builder;
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const comp_kind = opt.component_kind;
  const convert_opt = type_convert_options[base_name]!;

  // generate forward declaration
  fwd_b.$line(`// ${base_name} matrix, component: ${comp_name}`)
  for (const dim of matrix_dims) {
    fwd_b.$line(`struct ${base_name}${dim}x${dim};`)
  }
  fwd_b.$line(``);

  // generate c decls in cpp
  c_decl_cpp_b.$line(`// ${base_name} matrix, component: ${comp_name}`)
  for (const dim of matrix_dims) {
    c_decl_cpp_b.$line(`using skr_${base_name}${dim}x${dim}_t = ::skr::math::${base_name}${dim}x${dim};`)
  }
  c_decl_cpp_b.$line(``);

  // generate c decls in c
  c_decl_c_b.$line(`// ${base_name} matrix, component: ${comp_name}`)
  for (const dim of matrix_dims) {
    const vec_name = `skr_${base_name}${dim}_t`;

    c_decl_c_b.$line(`typedef struct ${get_alignas_matrix(opt, dim)}${base_name}${dim}x${dim}_t {`)
    c_decl_c_b.$indent(_b => {
      const axis_decls = _axis_lut
        .slice(0, dim)
        .map(axis => `axis_${axis}`)
        .join(`, `);
      c_decl_c_b.$line(`${vec_name} ${axis_decls};`)
    })
    c_decl_c_b.$line(`} ${base_name}${dim}x${dim}_t;`)
  }
  c_decl_c_b.$line(``);

  // generate matrix traits
  traits_b.$line(`// ${base_name} matrix, component: ${comp_name}`)
  for (const dim of matrix_dims) {
    // generate maker
    traits_b.$line(`template<> struct MathMatrixMaker<${comp_name}, ${dim}> {`)
    traits_b.$indent(_b => {
      traits_b.$line(`using Type = ${base_name}${dim}x${dim};`)
    })
    traits_b.$line(`};`)

    // generate traits
    traits_b.$line(`template<> struct MathMatrixTraits<${base_name}${dim}x${dim}> {`)
    traits_b.$indent(_b => {
      traits_b.$line(`inline static constexpr int32_t kDimensions = ${dim};`)
      traits_b.$line(`using ComponentType = ${comp_name};`)
      traits_b.$line(`using ImplementationFlag = void;`);
    })
    traits_b.$line(`};`)
  }
  traits_b.$line(``)

  // generate memory traits
  memory_traits_b.$line(`// ${base_name} matrix, component: ${comp_name}`);
  for (const dim of matrix_dims) {
    memory_traits_b.$line(`template<>`);
    memory_traits_b.$line(`struct MemoryTraits<::skr::math::${base_name}${dim}x${dim}> : MemoryTraitsPOD {`);
    memory_traits_b.$line(`};`);
  }
  memory_traits_b.$line(``);

  // generate class body
  for (const dim of matrix_dims) {
    const mat_name = `${base_name}${dim}x${dim}`;
    const vec_name = `${base_name}${dim}`;
    const vec3_name = `${base_name}3`;
    const vec4_name = `${base_name}4`;

    b.$line(`//! row major matrix`)
    b.$line(`struct ${get_alignas_matrix(opt, dim)}${mat_name} {`)
    b.$indent(_b => {
      // fill component types
      b.$line(`union {`)
      b.$indent(_b => {
        // axis based
        b.$line(`// axis based`)
        b.$line(`struct {`)
        b.$indent(_b => {
          for (let axis_idx = 0; axis_idx < dim; ++axis_idx) {
            b.$line(`${vec_name} axis_${_axis_lut[axis_idx]};`)
          }
        })
        b.$line(`};`)
        b.$line(``)

        // direction based
        b.$line(`// direction based`)
        b.$line(`struct {`)
        b.$indent(_b => {
          for (let axis_idx = 0; axis_idx < dim; ++axis_idx) {
            b.$line(`${vec_name} ${_dir_lut[axis_idx]};`)
          }
        })
        b.$line(`};`)
        b.$line(``)

        // row based
        b.$line(`// row based`)
        b.$line(`${vec_name} rows[${dim}];`)
        b.$line(``)

        // variable based
        b.$line(`// vector based`)
        b.$line(`struct {`)
        b.$indent(_b => {
          for (let row_idx = 0; row_idx < dim; ++row_idx) {
            const row_members = dims_all
              .slice(0, dim)
              .map(d => `m${row_idx}${d - 1}`)
              .join(`, `);
            b.$line(`${comp_name} ${row_members};`)
          }
        })
        b.$line(`};`)
      })
      b.$line(`};`)
      b.$line(``)

      // ctor & dtor
      {
        b.$line(`// ctor & dtor`)
        const default_ctor_exprs = `: ` + _axis_lut
          .slice(0, dim)
          .map(axis => `axis_${axis}(0)`)
          .join(`, `);
        // const default_ctor_exprs = `/*do noting for performance purpose, use factory init will be better*/`
        b.$line(`inline ${mat_name}() ${default_ctor_exprs} {}`)
        b.$line(`inline ${mat_name}(MathNoInitType) {}`)
        b.$line(`inline ${mat_name}(`)
        b.$indent(_b => {
          for (let row_idx = 0; row_idx < dim; ++row_idx) {
            const row_members = dims_all
              .slice(0, dim)
              .map(d => `${comp_name} m${row_idx}${d - 1}`)
              .join(`, `);
            if (row_idx === dim - 1) {
              b.$line(`${row_members}`)
            } else {
              b.$line(`${row_members},`)
            }
          }
        })
        b.$line(`):`)
        b.$indent(_b => {
          for (let row_idx = 0; row_idx < dim; ++row_idx) {
            const row_members = dims_all
              .slice(0, dim)
              .map(d => `m${row_idx}${d - 1}(m${row_idx}${d - 1})`)
              .join(`, `);
            if (row_idx === dim - 1) {
              b.$line(`${row_members}`)
            }
            else {
              b.$line(`${row_members},`)
            }
          }
        })
        b.$line(`{}`)
        {
          const axis_params = _axis_lut
            .slice(0, dim)
            .map(axis => `${vec_name} axis_${axis}`)
            .join(`, `);
          const axis_init_list = _axis_lut
            .slice(0, dim)
            .map(axis => `axis_${axis}(axis_${axis})`)
            .join(`, `);

          b.$line(`inline ${mat_name}(${axis_params}) noexcept : ${axis_init_list} {}`)
        }
        b.$line(`inline ~${mat_name}() = default;`)
        b.$line(``)
      }

      // convert with other dimensions
      b.$line(`// convert with other dimensions`)
      for (const other_dim of matrix_dims) {
        if (other_dim === dim) continue;
        const other_mat_name = `${base_name}${other_dim}x${other_dim}`;

        const copy_exprs = _axis_lut
          .slice(0, dim)
          .map(axis => `axis_${axis}(rhs.axis_${axis}.${_comp_lut.slice(0, dim).join(``)}())`)
          .join(`, `);

        b.$line(`explicit ${mat_name}(const ${other_mat_name}& rhs);`)
      }
      b.$line(``);

      // convert with other component kinds
      b.$line(`// convert with other component kinds`)
      for (const other_base_name in type_options) {
        const other_type_opt = type_options[other_base_name]!;
        if (!filter_matrix_comp_kind(other_type_opt.component_kind)) continue;
        const allow_cast = convert_opt.accept_list.includes(other_base_name)
        if (!allow_cast) continue;
        const allow_implicit = convert_opt.implicit_list.includes(other_base_name)
        const explicit_decorator = allow_implicit ? `` : `explicit `;
        const other_mat_name = `${other_base_name}${dim}x${dim}`;

        b.$line(`${explicit_decorator}${mat_name}(const ${other_mat_name}& rhs);`)
      }
      b.$line(``)

      // factory
      {
        b.$line(`// factory`)
        b.$line(`inline static ${mat_name} eye(${comp_name} v) {`)
        b.$indent(_b => {
          b.$line(`return ${mat_name}{`)
          b.$indent(_b => {
            for (let row_idx = 0; row_idx < dim; ++row_idx) {
              const row_members = dims_all
                .slice(0, dim)
                .map(d => d === row_idx + 1 ? `v` : `0`)
                .join(`, `);
              if (row_idx === dim - 1) {
                b.$line(`${row_members}`)
              } else {
                b.$line(`${row_members},`)
              }
            }
          })
          b.$line(`};`)
        })
        b.$line(`}`)
        b.$line(`inline static ${mat_name} fill(${comp_name} v) {`)
        b.$indent(_b => {
          b.$line(`return ${mat_name}{`)
          b.$indent(_b => {
            for (let row_idx = 0; row_idx < dim; ++row_idx) {
              const row_members = dims_all
                .slice(0, dim)
                .map(d => `v`)
                .join(`, `);
              if (row_idx === dim - 1) {
                b.$line(`${row_members}`)
              } else {
                b.$line(`${row_members},`)
              }
            }
          })
          b.$line(`};`)
        })
        b.$line(`}`)
        b.$line(`inline static ${mat_name} identity() { return eye(1); }`)
        b.$line(`inline static ${mat_name} zero() { return fill(0); }`)
        b.$line(`inline static ${mat_name} one() { return fill(1); }`)
        b.$line(`inline static ${mat_name} transposed(`);
        b.$indent(_b => {
          for (let col_idx = 0; col_idx < dim; ++col_idx) {
            const members = dims_all
              .slice(0, dim)
              .map(d => `${comp_name} m${d - 1}${col_idx}`)
              .join(`, `);
            if (col_idx === dim - 1) {
              b.$line(`${members}`)
            } else {
              b.$line(`${members},`)
            }
          }
        })
        b.$line(`) {`)
        b.$indent(_b => {
          b.$line(`return {`)
          b.$indent(_b => {
            for (let row_idx = 0; row_idx < dim; ++row_idx) {
              const row_members = dims_all
                .slice(0, dim)
                .map(d => `m${row_idx}${d - 1}`)
                .join(`, `);
              if (row_idx === dim - 1) {
                b.$line(`${row_members}`)
              } else {
                b.$line(`${row_members},`)
              }
            }
          })
          b.$line(`};`)
        })
        b.$line(`}`)
        b.$line(``)
      }

      // factory for utility usage
      b.$line(`// factory for utility usage`)
      if (dim >= 3) {
        b.$line(`static ${mat_name} from_scale(const ${base_name}3& scale);`)
      }
      if (dim === 4) {
        b.$line(`static ${mat_name} from_translation(const ${base_name}3& translation);`)
        b.$line(`static ${mat_name} look_to(const ${vec3_name}& from, const ${vec3_name}& dir, const ${vec3_name}& up);`)
        b.$line(`static ${mat_name} look_at(const ${vec3_name}& from, const ${vec3_name}& to, const ${vec3_name}& up);`)
        b.$line(`static ${mat_name} view_to(const ${vec3_name}& from, const ${vec3_name}& dir, const ${vec3_name}& up);`)
        b.$line(`static ${mat_name} view_at(const ${vec3_name}& from, const ${vec3_name}& to, const ${vec3_name}& up);`)
        b.$line(`static ${mat_name} perspective(${comp_name} view_width, ${comp_name} view_height, ${comp_name} near_distance, ${comp_name} far_distance);`)
        b.$line(`static ${mat_name} perspective_fov(${comp_name} fov_y, ${comp_name} aspect_ratio, ${comp_name} near_distance, ${comp_name} far_distance);`)
        b.$line(`static ${mat_name} orthographic(${comp_name} width, ${comp_name} height, ${comp_name} near_distance, ${comp_name} far_distance);`)
      }
      b.$line(``)

      // copy & move & assign & move assign
      {
        b.$line(`// copy & move & assign & move assign`)
        const copy_exprs = _axis_lut
          .slice(0, dim)
          .map(axis => `axis_${axis}(rhs.axis_${axis})`)
          .join(`, `);
        b.$line(`inline ${mat_name}(const ${mat_name}& rhs) noexcept : ${copy_exprs} {}`)
        const move_exprs = _axis_lut
          .slice(0, dim)
          .map(axis => `axis_${axis}(std::move(rhs.axis_${axis}))`)
          .join(`, `);
        b.$line(`inline ${mat_name}(${mat_name}&& rhs) noexcept : ${move_exprs} {}`)
        const assign_exprs = _axis_lut
          .slice(0, dim)
          .map(axis => `this->axis_${axis} = rhs.axis_${axis};`)
          .join(` `);
        b.$line(`inline ${mat_name}& operator=(const ${mat_name}& rhs) noexcept { ${assign_exprs} return *this; }`)
        const move_assign_exprs = _axis_lut
          .slice(0, dim)
          .map(axis => `this->axis_${axis} = std::move(rhs.axis_${axis});`)
          .join(` `);
        b.$line(`inline ${mat_name}& operator=(${mat_name}&& rhs) noexcept { ${move_assign_exprs} return *this; }`)
        b.$line(``)
      }

      // visitor operator
      b.$line(`// visitor operator`)
      b.$line(`inline ${vec_name} operator[](size_t idx) const { SKR_ASSERT(idx < ${dim}); return rows[idx]; }`)
      b.$line(`inline ${vec_name}& operator[](size_t idx) { SKR_ASSERT(idx < ${dim}); return rows[idx]; }`)
      b.$line(``)

      // mul operator
      b.$line(`// mul operator`)
      b.$line(`friend ${mat_name} operator*(const ${mat_name}& lhs, const ${mat_name}& rhs);`)
      b.$line(`${mat_name}& operator*=(const ${mat_name}& rhs);`)
      b.$line(`friend ${vec_name} operator*(const ${vec_name}& lhs, const ${mat_name}& rhs);`)

    })
    b.$line(`};`)
  }
}
function _gen_convert_func_body(opt: GenMatrixOption) {
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const comp_kind = opt.component_kind;
  const convert_opt = type_convert_options[base_name]!;

  b.$line(`// [${base_name}] convert with other dimensions`)
  for (const dim of matrix_dims) {
    const mat_name = `${base_name}${dim}x${dim}`;
    const vec_name = `${base_name}${dim}`;

    for (const other_dim of matrix_dims) {
      if (other_dim === dim) continue;
      const other_mat_name = `${base_name}${other_dim}x${other_dim}`;

      b.$line(`inline ${mat_name}::${mat_name}(const ${other_mat_name}& rhs) :`)
      b.$indent(_b => {
        for (let cur_axis_idx = 0; cur_axis_idx < dim; ++cur_axis_idx) {
          const end_comma = cur_axis_idx === dim - 1 ? `` : `,`;
          const cur_axis = _axis_lut[cur_axis_idx];

          let swizzle_init_exprs: string
          if (cur_axis_idx < other_dim) {
            if (other_dim > dim) {
              swizzle_init_exprs = `rhs.axis_${cur_axis}.` + _comp_lut
                .slice(0, dim)
                .join(``) + `()`;
            } else {
              swizzle_init_exprs = `rhs.axis_${cur_axis}` + `, ${comp_name}(0)`.repeat(dim - other_dim);
            }
          } else {
            const axis_init_exprs = _axis_lut
              .slice(0, dim)
              .map((axis, i) => i === cur_axis_idx ? `1` : `0`)
              .map(v => `${comp_name}(${v})`)
              .join(`, `);

            swizzle_init_exprs = axis_init_exprs
          }

          b.$line(`axis_${cur_axis}(${swizzle_init_exprs})${end_comma}`)
        }
      })
      b.$line(`{}`)
    }
  }
}

function _gen_convert_func_body_cross_type(opt: GenMatrixOption) {
  const b = opt.builder;
  const base_name = opt.base_name;
  const comp_name = opt.component_name;
  const comp_kind = opt.component_kind;
  const convert_opt = type_convert_options[base_name]!;

  b.$line(`// [${base_name}] convert with other component kinds`)
  for (const dim of matrix_dims) {
    const mat_name = `${base_name}${dim}x${dim}`;
    const vec_name = `${base_name}${dim}`;

    for (const other_base_name in type_options) {
      const other_type_opt = type_options[other_base_name]!;
      if (!filter_matrix_comp_kind(other_type_opt.component_kind)) continue;
      const allow_cast = convert_opt.accept_list.includes(other_base_name)
      if (!allow_cast) continue;
      const allow_implicit = convert_opt.implicit_list.includes(other_base_name)
      const other_mat_name = `${other_base_name}${dim}x${dim}`;

      b.$line(`inline ${mat_name}::${mat_name}(const ${other_mat_name}& rhs) :`)
      b.$indent(_b => {
        for (let cur_axis_idx = 0; cur_axis_idx < dim; ++cur_axis_idx) {
          const end_comma = cur_axis_idx === dim - 1 ? `` : `,`;
          const cur_axis = _axis_lut[cur_axis_idx];

          const axis_init_exprs = `axis_${cur_axis}(${vec_name}(rhs.axis_${cur_axis}))`
          b.$line(`${axis_init_exprs}${end_comma}`)
        }
      })
      b.$line(`{}`)
    }
  }
}

export function gen(global_builders: GlobalBuilders, gen_dir: string) {
  const inc_builder = new CodeBuilder();
  inc_builder.$util_header();

  // generate class body
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    if (!filter_matrix_comp_kind(type_opt.component_kind)) continue;

    // header
    const builder = new CodeBuilder();
    builder.$util_header();

    // include
    builder.$line(`#include <cstdint>`);
    builder.$line(`#include <cmath>`);
    builder.$line(`#include "../gen_math_fwd.hpp"`);
    builder.$line(`#include "../vec/${base_name}_vec.hpp"`);
    builder.$line(`#include <SkrBase/misc/debug.h>`);
    builder.$line(`#include <SkrBase/misc/hash.hpp>`);
    builder.$line(``);

    // gen code
    builder.$line(`namespace skr {`);
    builder.$line(`inline namespace math {`);

    _gen_class_body({
      builder,
      base_name,
      ...type_opt,
      ...global_builders
    })
    builder.$line(``);

    builder.$line(`}`)
    builder.$line(`}`)

    // write to file
    const file_name = path.join(gen_dir, `${base_name}_matrix.hpp`);
    builder.write_file(file_name)
    inc_builder.$line(`#include "./${path.basename(file_name)}"`);
  }

  // generate convert functions
  {
    const convert_builder = new CodeBuilder();
    convert_builder.$util_header();

    for (const base_name in type_options) {
      const type_opt = type_options[base_name]!;
      if (!filter_matrix_comp_kind(type_opt.component_kind)) continue;

      convert_builder.$line(`#include "./${base_name}_matrix.hpp"`);
    }
    convert_builder.$line(``);

    convert_builder.$line(`namespace skr {`);
    convert_builder.$line(`inline namespace math {`);

    for (const base_name in type_options) {
      const type_opt = type_options[base_name]!;
      if (!filter_matrix_comp_kind(type_opt.component_kind)) continue;

      _gen_convert_func_body({
        builder: convert_builder,
        base_name,
        ...type_opt,
        ...global_builders
      })
      convert_builder.$line(``);

      _gen_convert_func_body_cross_type({
        builder: convert_builder,
        base_name,
        ...type_opt,
        ...global_builders
      })
      convert_builder.$line(``);
    }

    convert_builder.$line(`}`)
    convert_builder.$line(`}`)

    const convert_file_name = path.join(gen_dir, `matrix_convert.hpp`);
    convert_builder.write_file(convert_file_name);
    inc_builder.$line(`#include "./matrix_convert.hpp"`);
  }

  // write include file
  const include_file_name = path.join(gen_dir, `gen_matrix.hpp`);
  inc_builder.write_file(include_file_name);
}