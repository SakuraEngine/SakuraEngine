import * as process from "node:process";
import * as gen_vector from "./framework/gen_vector";
import * as gen_math_func from "./framework/gen_math_func";
import * as gen_matrix from "./framework/gen_matrix";
import * as gen_misc_types from "./framework/gen_misc_types";
import path from "node:path";
import { CodeBuilder, type_options } from "./framework/util";

const argv = process.argv.slice(2);

// parse args
if (argv.length != 1) {
  throw Error("only support 1 argument that generate dir");
}
const gen_dir = argv[0]!;

// fwd builder
const fwd_builder = new CodeBuilder();
fwd_builder.$util_header();
fwd_builder.$line("");
fwd_builder.$line("namespace skr {");
fwd_builder.$line("inline namespace math {");

// c decl builder
const c_decl_cpp_builder = new CodeBuilder();
c_decl_cpp_builder.$util_header();
c_decl_cpp_builder.$line(`#include "./gen_math.hpp"`);
const c_decl_c_builder = new CodeBuilder();
c_decl_c_builder.$util_header();
c_decl_c_builder.$line(`#include <SkrBase/config.h>`)

// traits builder
const traits_builder = new CodeBuilder();
traits_builder.$util_header();
traits_builder.$line(`#include "./gen_math.hpp"`)
traits_builder.$line(`namespace skr {`)
traits_builder.$line(`inline namespace math {`)

// memory traits builder
const memory_traits_builder = new CodeBuilder();
memory_traits_builder.$util_header();
memory_traits_builder.$line(`#include <SkrBase/memory/memory_traits.hpp>`)
memory_traits_builder.$line(`#include "./gen_math.hpp"`)
memory_traits_builder.$line(`namespace skr::memory {`)

// generate vector types
gen_vector.gen(
  {
    fwd_builder,
    c_decl_cpp_builder,
    c_decl_c_builder,
    traits_builder,
    memory_traits_builder
  },
  path.join(gen_dir, "vec")
)

// generate matrix types
gen_matrix.gen(
  {
    fwd_builder,
    c_decl_cpp_builder,
    c_decl_c_builder,
    traits_builder,
    memory_traits_builder
  },
  path.join(gen_dir, "mat")
)

// generate math functions
gen_math_func.gen(
  {
    fwd_builder,
    c_decl_cpp_builder,
    c_decl_c_builder,
    traits_builder,
    memory_traits_builder
  },
  path.join(gen_dir, "math")
)

// generate misc types
gen_misc_types.gen(
  {
    fwd_builder,
    c_decl_cpp_builder,
    c_decl_c_builder,
    traits_builder,
    memory_traits_builder
  },
  path.join(gen_dir, "misc")
)

// write forward declaration
fwd_builder.$line("}");
fwd_builder.$line("}");
const fwd_out_path = path.join(gen_dir, "gen_math_fwd.hpp");
fwd_builder.write_file(fwd_out_path);

// write c decls
const c_decl_cpp_out_path = path.join(gen_dir, "gen_math_c_decl.hpp");
const c_decl_c_out_path = path.join(gen_dir, "gen_math_c_decl.h");
// c_decl_cpp_builder.write_file(c_decl_cpp_out_path);
// c_decl_c_builder.write_file(c_decl_c_out_path);

// write traits
traits_builder.$line("}");
traits_builder.$line("}");
const traits_out_path = path.join(gen_dir, "gen_math_traits.hpp");
traits_builder.write_file(traits_out_path);

// write memory traits
memory_traits_builder.$line("}");
const memory_traits_out_path = path.join(gen_dir, "gen_math_memory_traits.hpp");
memory_traits_builder.write_file(memory_traits_out_path);

// full include builder
const full_inc_builder = new CodeBuilder();
full_inc_builder.$util_header();
full_inc_builder.$line(`#include "gen_math_fwd.hpp"`);
full_inc_builder.$line(`#include "./vec/gen_vector.hpp"`);
full_inc_builder.$line(`#include "./mat/gen_matrix.hpp"`);
full_inc_builder.$line(`#include "./math/gen_math_func.hpp"`);
full_inc_builder.$line(`#include "./misc/gen_misc.hpp"`);
const full_inc_path = path.join(gen_dir, "gen_math.hpp");
full_inc_builder.write_file(full_inc_path);