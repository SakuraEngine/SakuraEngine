import * as gen from "@framework/generator"
import * as db from "@framework/database"
import { CodeBuilder } from "@framework/utils"

class _Gen {
  static header_pre(header: db.Header) {
    const b = header.gen_code

    // header
    b.$line(`//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!`)
    b.$line(`//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!`)
    b.$line(`//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!`)
    b.$line(``)

    // basic includes
    b.$line(`#pragma once`)
    b.$line(`#include "SkrBase/config.h"`)
    b.$line(`#include <inttypes.h>`)
    b.$line(``)

    // include protect
    b.$line(`#ifdef __meta__`)
    b.$line(`#error "this file should not be inspected by meta"`)
    b.$line(`#endif`)
    b.$line(``)

    // file id
    b.$line(`#ifdef SKR_FILE_ID`)
    b.$indent(_b => {
      b.$line(`#undef SKR_FILE_ID`)
    })
    b.$line(`#endif`)
    b.$line(`#define SKR_FILE_ID ${header.file_id}`)
    b.$line(``)

    // GENERATE_BODY()
    b.$line(`// BEGIN Generated Body`)
    header.records.forEach((record) => {
      if (!record.has_generate_body_flag) return
      b.$line(`#define _zz_SKR_GENERATE_BODY_${header.file_id}_${record.generate_body_line} ${record.dump_generate_body()}`)
    })
    b.$line(`// END Generated Body`)
    b.$line(``)

    // forward declarations
    b.$line(`// BEGIN forward declarations`)
    header.records.forEach((record) => {
      b.$namespace_line(record.namespace.join("::"), () => {
        return `struct ${record.short_name};`
      })
    })
    header.enums.forEach((enum_) => {
      let prefix = enum_.is_scoped ? "class" : "";
      let underlying_type = enum_.underlying_type != 'unfixed' ? `: ${enum_.underlying_type}` : "";
      b.$namespace_line(enum_.namespace.join('::'), () => {
        return `enum ${prefix} ${enum_.short_name}${underlying_type};`
      })
    })
    b.$line(`// END forward declarations`)

  }
  static source_pre(b: CodeBuilder, main_db: db.Module) {
    // header
    b.$line(`//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!`)
    b.$line(`//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!`)
    b.$line(`//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!`)
    b.$line(``)

    // includes
    b.$line(`// BEGIN header includes`)
    b.$line(`// #define private public`)
    b.$line(`// #define protected public`)
    main_db.headers.forEach((header) => {
      // if (header.header_path.length === 0) return // should not happen
      b.$line(`#include "${header.header_path}"`)
    })
    b.$line(`// #undef private`)
    b.$line(`// #undef protected`)
    b.$line(`// END header includes`)
    b.$line(``)

    // diagnostic
    b.$line(`// BEGIN push diagnostic`)
    b.$line(`#if defined(__clang__)`)
    b.$line(`#pragma clang diagnostic push`)
    b.$line(`#pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"`)
    b.$line(`#endif`)
    b.$line(`// END push diagnostic`)

  }
  static source_post(b: CodeBuilder) {
    b.$line(`// BEGIN pop diagnostic`)
    b.$line(`#if defined(__clang__)`)
    b.$line(`#pragma clang diagnostic pop`)
    b.$line(`#endif`)
    b.$line(`// END pop diagnostic`)
  }
}

class BasicGenerator extends gen.Generator {
  override pre_gen(): void {
    this.main_module_db.each_record((record, _header) => {
      // check GENERATE_BODY()
      if (!record.generate_body_content.is_empty() && !record.has_generate_body_flag) {
        console.error(`cpp file: ${record.file_name}:${record.line}`)
        console.error(`content: ${record.generate_body_content}`)
        throw new Error(
          `The record ${record.name} has a body content but the generate_body_flag is not set. Please check the record.`
        )
      }
    })

    // gen header
    this.main_module_db.headers.forEach(header => {
      _Gen.header_pre(header);
    })

    // gen source
    _Gen.source_pre(this.main_module_db.pre_all_file, this.main_module_db);
  }
  override post_gen(): void {
    // gen source
    _Gen.source_post(this.main_module_db.post_all_file);
  }
}

export function load_generator(gm: gen.GenerateManager) {
  gm.add_generator("basic", new BasicGenerator())
}