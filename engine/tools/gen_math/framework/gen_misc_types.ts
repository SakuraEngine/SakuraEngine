import {
  CodeBuilder, type_convert_options,
  type_options, all_component_kinds,
  dims_all, dims_no_scalar
} from "./util"
import type { TypeOption, ComponentKind, GlobalBuilders } from "./util";
import path from "node:path";

const _suffix_lut: Dict<string> = {
  "float": "F",
  "double": "D",
}
const _quat_comp_lut = ["x", "y", "z", "w"];
const _rotator_comp_lut = ["pitch", "yaw", "roll"];

interface GenMiscOption extends GlobalBuilders {
  builder: CodeBuilder;
};

function _gen_quat(opt: GenMiscOption) {
  const fwd_b = opt.fwd_builder;
  const c_decl_cpp_b = opt.c_decl_cpp_builder;
  const c_decl_c_b = opt.c_decl_c_builder;
  const memory_traits_b = opt.memory_traits_builder;
  const b = opt.builder;

  // generate memory traits
  memory_traits_b.$line(`// quaternion`);
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const _suffix = _suffix_lut[type_opt.component_name];

    memory_traits_b.$line(`template<>`);
    memory_traits_b.$line(`struct MemoryTraits<::skr::math::Quat${_suffix}> : MemoryTraitsPOD {`);
    memory_traits_b.$line(`};`);
  }
  memory_traits_b.$line(``);

  fwd_b.$line(`// quaternion`)
  c_decl_cpp_b.$line(`// quaternion`)
  c_decl_c_b.$line(`// quaternion`)

  // generate body
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const suffix = _suffix_lut[type_opt.component_name];
    if (suffix === undefined) {
      throw new Error(`unknown component name ${type_opt.component_name} for quat`);
    }

    // gen forward & c decls
    fwd_b.$line(`struct Quat${suffix};`);
    c_decl_cpp_b.$line(`using skr_quat_${suffix.toLowerCase()}_t = ::skr::math::Quat${suffix};`);
    c_decl_c_b.$line(`typedef struct alignas(16) skr_quat_${suffix.toLowerCase()}_t {`);
    c_decl_c_b.$indent(_b => {
      const comp_name = type_opt.component_name;
      const quat_comp_lut = _quat_comp_lut
        .join(`, `);

      c_decl_c_b.$line(`${comp_name} ${quat_comp_lut};`)
    })
    c_decl_c_b.$line(`} skr_quat_${suffix.toLowerCase()}_t;`);

    // get data
    const comp_name = type_opt.component_name;
    const quat_name = `Quat${suffix}`;
    const rotator_name = `Rotator${suffix}`;
    const vec3_name = `${base_name}3`;
    const vec4_name = `${base_name}4`;
    const matrix3_name = `${base_name}3x3`;
    const matrix4_name = `${base_name}4x4`;

    b.$line(`struct alignas(16) ${quat_name} {`)
    b.$indent(_b => {
      // member
      b.$line(`${comp_name} ${_quat_comp_lut.join(`, `)};`)
      b.$line(``)

      // ctor & dtor
      b.$line(`// ctor & dtor`)
      b.$line(`inline ${quat_name}() : x(0), y(0), z(0), w(1) {}`)
      b.$line(`inline ${quat_name}(MathNoInitType) {}`)
      {
        const ctor_params = _quat_comp_lut
          .map(c => `${comp_name} ${c}`)
          .join(`, `);
        const init_list = _quat_comp_lut
          .map(c => `${c}(${c})`)
          .join(`, `);
        b.$line(`inline ${quat_name}(${ctor_params}) : ${init_list} {}`)
      }
      b.$line(`inline ~${quat_name}() = default;`)
      b.$line(``)

      // factory
      b.$line(`// factory`)
      b.$line(`inline static ${quat_name} Identity() { return { 0, 0, 0, 1 }; }`)
      b.$line(`inline static ${quat_name} Fill(${comp_name} v) { return { v, v, v, v }; }`)
      b.$line(`static ${quat_name} Euler(${comp_name} pitch, ${comp_name} yaw, ${comp_name} roll);`)
      b.$line(`static ${quat_name} AxisAngle(${vec3_name} axis, ${comp_name} angle);`)
      b.$line(``)

      // copy & move & assign & move assign
      b.$line(`// copy & move & assign & move assign`)
      b.$line(`inline ${quat_name}(${quat_name} const&) = default;`)
      b.$line(`inline ${quat_name}(${quat_name}&&) = default;`)
      b.$line(`inline ${quat_name}& operator=(${quat_name} const&) = default;`)
      b.$line(`inline ${quat_name}& operator=(${quat_name}&&) = default;`)
      b.$line(``)

      // convert with vector
      b.$line(`// convert with vector`)
      b.$line(`inline explicit ${quat_name}(const ${vec4_name}& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}`)
      b.$line(`inline explicit operator ${vec4_name}() const { return { x, y, z, w }; }`)
      b.$line(``)

      // as vector
      b.$line(`// as vector`)
      b.$line(`inline ${vec4_name}& as_vector() { return *reinterpret_cast<${vec4_name}*>(this); }`)
      b.$line(`inline const ${vec4_name}& as_vector() const { return *reinterpret_cast<const ${vec4_name}*>(this); }`)
      b.$line(``)

      // convert with rotator
      b.$line(`// convert with rotator`)
      b.$line(`explicit ${quat_name}(const ${rotator_name}& rotator);`)
      b.$line(`static ${quat_name} FromRotator(const ${rotator_name}& rotator);`)
      b.$line(``)

      // neg operator
      b.$line(`// negative operator`)
      b.$line(`${quat_name} operator-() const;`)
      b.$line(``)

      // mul assign operator
      b.$line(`// mul assign operator`)
      b.$line(`${quat_name}& operator*=(const ${quat_name}& rhs);`)
      b.$line(``)

      // [use as_vector()] compare operator

      // get axis & angle
      b.$line(`// get axis & angle`)
      b.$line(`${vec3_name} axis() const;`)
      b.$line(`${comp_name} angle() const;`)
      b.$line(`void axis_angle(${vec3_name}& axis, ${comp_name}& angle) const;`);
      b.$line(``)

      // identity
      b.$line(`// identity`)
      b.$line(`bool is_identity() const;`)
      b.$line(`bool is_nearly_identity(${comp_name} threshold_angle = ${comp_name}(0.00284714461)) const;`)
      b.$line(``)

      // to matrix
      b.$line(`// to matrix`)
      b.$line(`explicit operator ${matrix3_name}() const;`)
      b.$line(`explicit operator ${matrix4_name}() const;`)
      b.$line(`${matrix3_name} to_matrix3() const;`)
      b.$line(`${matrix4_name} to_matrix4() const;`)
      b.$line(``)

      // from matrix
      b.$line(`// from matrix`)
      b.$line(`explicit ${quat_name}(const ${matrix3_name}& mat);`)
      b.$line(`explicit ${quat_name}(const ${matrix4_name}& mat);`)
      b.$line(`static ${quat_name} FromMatrix(const ${matrix3_name}& mat);`)
      b.$line(`static ${quat_name} FromMatrix(const ${matrix4_name}& mat);`)
    })
    b.$line(`};`)
  }

  fwd_b.$line(``);
  c_decl_cpp_b.$line(``);
  c_decl_c_b.$line(``);
}

function _gen_rotator(opt: GenMiscOption) {
  const fwd_b = opt.fwd_builder;
  const c_decl_cpp_b = opt.c_decl_cpp_builder;
  const memory_traits_b = opt.memory_traits_builder;
  const c_decl_c_b = opt.c_decl_c_builder;
  const b = opt.builder;

  // generate memory traits
  memory_traits_b.$line(`// rotator`);
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const _suffix = _suffix_lut[type_opt.component_name];

    memory_traits_b.$line(`template<>`);
    memory_traits_b.$line(`struct MemoryTraits<::skr::math::Rotator${_suffix}> : MemoryTraitsPOD {`);
    memory_traits_b.$line(`};`);
  }
  memory_traits_b.$line(``);

  fwd_b.$line(`// rotator`)
  c_decl_cpp_b.$line(`// rotator`)
  c_decl_c_b.$line(`// rotator`)

  // generate body
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const suffix = _suffix_lut[type_opt.component_name];
    if (suffix === undefined) {
      throw new Error(`unknown component name ${type_opt.component_name} for quat`);
    }

    // gen forward & c decls
    fwd_b.$line(`struct Rotator${suffix};`);
    c_decl_cpp_b.$line(`using skr_rotator_${suffix.toLowerCase()}_t = ::skr::math::Rotator${suffix};`);
    c_decl_c_b.$line(`typedef struct skr_rotator_${suffix.toLowerCase()}_t {`);
    c_decl_c_b.$indent(_b => {
      const comp_name = type_opt.component_name;
      const rotator_comp_lut = _rotator_comp_lut
        .join(`, `);

      c_decl_c_b.$line(`${comp_name} ${rotator_comp_lut};`)
    })
    c_decl_c_b.$line(`} skr_rotator_${suffix.toLowerCase()}_t;`);

    // get data
    const comp_name = type_opt.component_name;
    const rotator_name = `Rotator${suffix}`;
    const vec3_name = `${base_name}3`;
    const vec4_name = `${base_name}4`;
    const mat3_name = `${base_name}3x3`;
    const mat4_name = `${base_name}4x4`;
    const quat_name = `Quat${suffix}`;

    b.$line(`struct ${rotator_name} {`)
    b.$indent(_b => {
      // member
      b.$line(`${comp_name} ${_rotator_comp_lut.join(`, `)};`)
      b.$line(``)

      // ctor & dtor
      b.$line(`// ctor & dtor`)
      b.$line(`inline ${rotator_name}() : pitch(0), yaw(0), roll(0) {}`)
      b.$line(`inline ${rotator_name}(MathNoInitType) {}`)
      {
        const ctor_params = _rotator_comp_lut
          .map(c => `${comp_name} ${c}`)
          .join(`, `);
        const init_list = _rotator_comp_lut
          .map(c => `${c}(${c})`)
          .join(`, `);
        b.$line(`inline ${rotator_name}(${ctor_params}) : ${init_list} {}`)
      }
      b.$line(`inline ~${rotator_name}() = default;`)
      b.$line(``)

      // factory
      b.$line(`// factory`)
      b.$line(``)

      // convert with vector
      b.$line(`// convert with vector`)
      b.$line(`inline explicit ${rotator_name}(const ${vec3_name}& vec) : pitch(vec.x), yaw(vec.y), roll(vec.z) {}`)
      b.$line(`inline explicit operator ${vec3_name}() const { return { pitch, yaw, roll }; }`)
      b.$line(``)

      // as vector
      b.$line(`// as vector`)
      b.$line(`inline ${vec3_name}& as_vector() { return *reinterpret_cast<${vec3_name}*>(this); }`)
      b.$line(`inline const ${vec3_name}& as_vector() const { return *reinterpret_cast<const ${vec3_name}*>(this); }`)
      b.$line(``)

      // convert with quat
      b.$line(`// convert with quat`)
      b.$line(`explicit ${rotator_name}(const ${quat_name}& quat);`)
      b.$line(`static ${rotator_name} FromQuat(const ${quat_name}& quat);`)
      b.$line(``)

      // [use as_vector()] arithmetic operator
      // [use as_vector()] compare operator

      // negative operator
      b.$line(`// negative operator`)
      b.$line(`${rotator_name} operator-() const;`)
      b.$line(``)

      // mul assign operator
      b.$line(`// mul assign operator`)
      b.$line(`${rotator_name}& operator*=(const ${rotator_name}& rhs);`)
      b.$line(``)

      // add & sub assign operator
      b.$line(`// add & sub assign operator`)
      b.$line(`${rotator_name}& operator+=(const ${rotator_name}& rhs);`)
      b.$line(`${rotator_name}& operator-=(const ${rotator_name}& rhs);`)
      b.$line(``)

      // to matrix
      b.$line(`// to matrix`)
      b.$line(`explicit operator ${mat3_name}() const;`)
      b.$line(`explicit operator ${mat4_name}() const;`)
      b.$line(`${mat3_name} to_matrix3() const;`)
      b.$line(`${mat4_name} to_matrix4() const;`)
      b.$line(``)

      // from matrix
      b.$line(`// from matrix`)
      b.$line(`explicit ${rotator_name}(const ${mat3_name}& mat);`)
      b.$line(`explicit ${rotator_name}(const ${mat4_name}& mat);`)
      b.$line(`static ${rotator_name} FromMatrix(const ${mat3_name}& mat);`)
      b.$line(`static ${rotator_name} FromMatrix(const ${mat4_name}& mat);`)
    })
    b.$line(`}; `)
  }

  fwd_b.$line(``);
  c_decl_cpp_b.$line(``);
  c_decl_c_b.$line(``);
}

function _gen_transform(opt: GenMiscOption) {
  const fwd_b = opt.fwd_builder;
  const c_decl_cpp_b = opt.c_decl_cpp_builder;
  const memory_traits_b = opt.memory_traits_builder;
  const c_decl_c_b = opt.c_decl_c_builder;
  const b = opt.builder;

  // generate memory traits
  memory_traits_b.$line(`// transform`);
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const _suffix = _suffix_lut[type_opt.component_name];

    memory_traits_b.$line(`template<>`);
    memory_traits_b.$line(`struct MemoryTraits<::skr::math::Transform${_suffix}> : MemoryTraitsPOD {`);
    memory_traits_b.$line(`};`);
  }
  memory_traits_b.$line(``);

  fwd_b.$line(`// transform`)
  c_decl_cpp_b.$line(`// transform`)
  c_decl_c_b.$line(`// transform`)

  // generate body
  for (const base_name in type_options) {
    const type_opt = type_options[base_name]!;

    // filter component kinds
    if (type_opt.component_kind !== "floating") continue;

    // get suffix
    const suffix = _suffix_lut[type_opt.component_name];
    if (suffix === undefined) {
      throw new Error(`unknown component name ${type_opt.component_name} for quat`);
    }

    // gen forward & c decls
    fwd_b.$line(`struct Transform${suffix};`);
    c_decl_cpp_b.$line(`using skr_transform_${suffix.toLowerCase()}_t = ::skr::math::Transform${suffix};`);
    c_decl_c_b.$line(`typedef struct skr_transform_${suffix.toLowerCase()}_t {`);
    c_decl_c_b.$indent(_b => {
      const comp_name = type_opt.component_name;
      const vec3_name = `skr_${base_name}3_t`;
      const vec4_name = `skr_${base_name}4_t`;
      const quat_name = `skr_quat_${suffix.toLowerCase()}_t`;

      c_decl_c_b.$line(`${quat_name} rotation;`)
      c_decl_c_b.$line(`${vec3_name} position;`)
      c_decl_c_b.$line(`${vec3_name} scale;`)
    })
    c_decl_c_b.$line(`} skr_transform_${suffix.toLowerCase()}_t;`);

    // get data
    const comp_name = type_opt.component_name;
    const vec3_name = `${base_name}3`;
    const vec4_name = `${base_name}4`;
    const mat3_name = `${base_name}3x3`;
    const mat4_name = `${base_name}4x4`;
    const quat_name = `Quat${suffix}`;
    const transform_name = `Transform${suffix}`;

    b.$line(`struct ${transform_name} {`)
    b.$indent(_b => {
      // gen member
      b.$line(`alignas(16) ${quat_name} rotation;`)
      b.$line(`alignas(16) ${vec3_name} position;`)
      b.$line(`alignas(16) ${vec3_name} scale;`)
      b.$line(``)

      // ctor & dtor
      b.$line(`// ctor & dtor`)
      b.$line(`inline ${transform_name}() = default;`)
      b.$line(`inline ${transform_name}(MathNoInitType) : rotation(kMathNoInit), position(kMathNoInit), scale(kMathNoInit) {}`)
      {
        const ctor_params = [
          `const ${quat_name}& rotation`,
          `const ${vec3_name}& position`,
          `const ${vec3_name}& scale`,
        ].join(`, `);

        const init_list = [
          `rotation(rotation)`,
          `position(position)`,
          `scale(scale)`,
        ].join(`, `);

        b.$line(`inline ${transform_name}(${ctor_params}) : ${init_list} {}`)
      }
      b.$line(`inline ~${transform_name}() = default;`)
      b.$line(``)

      // factory
      b.$line(`// factory`)
      b.$line(`inline static ${transform_name} Identity() { return { ${quat_name}::Identity(), ${vec3_name}(0), ${vec3_name}(1) }; }`)
      b.$line(``)

      // copy & move & assign & move assign
      b.$line(`// copy & move & assign & move assign`)
      b.$line(`inline ${transform_name}(${transform_name} const&) = default;`)
      b.$line(`inline ${transform_name}(${transform_name}&&) = default;`)
      b.$line(`inline ${transform_name}& operator=(${transform_name} const&) = default;`)
      b.$line(`inline ${transform_name}& operator=(${transform_name}&&) = default;`)
      b.$line(``)

      // mul assign operator
      b.$line(`// mul assign operator`)
      b.$line(`${transform_name}& operator*=(const ${transform_name}& rhs);`)
      b.$line(``)

      // identity
      b.$line(`// identity`)
      b.$line(`bool is_identity() const;`)
      b.$line(`bool is_nearly_identity(${comp_name} threshold = ${comp_name}(0.00001)) const;`)
      b.$line(``)

      // to matrix
      b.$line(`// to matrix`)
      b.$line(`explicit operator ${mat4_name}() const;`)
      b.$line(`${mat4_name} to_matrix() const;`)
      b.$line(`${mat4_name} to_matrix_no_scale() const;`)
      b.$line(``)

      // from matrix
      b.$line(`// from matrix`)
      b.$line(`explicit ${transform_name}(const ${mat3_name}& mat);`)
      b.$line(`explicit ${transform_name}(const ${mat4_name}& mat);`)
      b.$line(`static ${transform_name} FromMatrix(const ${mat3_name}& mat);`)
      b.$line(`static ${transform_name} FromMatrix(const ${mat4_name}& mat);`)
      b.$line(``)

      // directions
      b.$line(`// directions`)
      b.$line(`${vec3_name} right() const;`)
      b.$line(`${vec3_name} up() const;`)
      b.$line(`${vec3_name} forward() const;`)
      b.$line(`${vec3_name} left() const;`)
      b.$line(`${vec3_name} down() const;`)
      b.$line(`${vec3_name} back() const;`)
      b.$line(``)
    })
    b.$line(`};`)
  }

  fwd_b.$line(``);
  c_decl_cpp_b.$line(``);
  c_decl_c_b.$line(``);
}

export function gen(global_builders: GlobalBuilders, gen_dir: string) {
  const inc_builder = new CodeBuilder();
  inc_builder.$util_header();

  // generate quat
  {
    // header
    const builder = new CodeBuilder();
    builder.$util_header();

    // include
    builder.$line(`#include "../vec/gen_vector.hpp"`);
    builder.$line(`#include "../mat/gen_matrix.hpp"`);
    builder.$line(`#include "../math/gen_math_func.hpp"`);
    builder.$line(``);

    // gen code
    builder.$line(`namespace skr {`);
    builder.$line(`inline namespace math {`);
    _gen_quat({
      builder,
      ...global_builders,
    })
    builder.$line(`}`)
    builder.$line(`}`)

    // write to file
    const file_name = path.join(gen_dir, `quat.hpp`);
    builder.write_file(file_name);
    inc_builder.$line(`#include "./quat.hpp"`);
  }

  // generate rotator
  {
    const builder = new CodeBuilder();
    builder.$util_header();

    // include
    builder.$line(`#include "../vec/gen_vector.hpp"`);
    builder.$line(`#include "../mat/gen_matrix.hpp"`);
    builder.$line(`#include "../math/gen_math_func.hpp"`);
    builder.$line(``);

    // gen code
    builder.$line(`namespace skr {`);
    builder.$line(`inline namespace math {`);
    _gen_rotator({
      builder,
      ...global_builders,
    })
    builder.$line(`}`)
    builder.$line(`}`)

    // write to file
    const file_name = path.join(gen_dir, `rotator.hpp`);
    builder.write_file(file_name);
    inc_builder.$line(`#include "./rotator.hpp"`);
  }

  // generate transform
  {
    const builder = new CodeBuilder();
    builder.$util_header();

    // include
    builder.$line(`#include "../vec/gen_vector.hpp"`);
    builder.$line(`#include "../mat/gen_matrix.hpp"`);
    builder.$line(`#include "../math/gen_math_func.hpp"`);
    builder.$line(`#include "./quat.hpp"`);
    builder.$line(``);

    // gen code
    builder.$line(`namespace skr {`);
    builder.$line(`inline namespace math {`);
    _gen_transform({
      builder,
      ...global_builders,
    })
    builder.$line(`}`)
    builder.$line(`}`)

    // write to file
    const file_name = path.join(gen_dir, `transform.hpp`);
    builder.write_file(file_name);
    inc_builder.$line(`#include "./transform.hpp"`);
  }

  // write inc file
  const full_inc_path = path.join(gen_dir, "gen_misc.hpp");
  inc_builder.write_file(full_inc_path);
}