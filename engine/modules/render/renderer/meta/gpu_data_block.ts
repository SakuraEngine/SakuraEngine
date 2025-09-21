import * as gen from "@framework/generator";
import * as db from "@framework/database";
import * as ml from "@framework/meta_lang";
import { CodeBuilder } from "@framework/utils";

class SOAConfig extends ml.WithEnable {
  @ml.value("number")
  page_size: number = 16384;
}
class AOSConfig extends ml.WithEnable {
}

class RecordConfig extends ml.WithEnable {
  soa: SOAConfig = new SOAConfig();
  aos: AOSConfig = new AOSConfig();
}

class _Gen {
  static header_data_blocks(header: db.Header) {
    if (!header.gen_code_custom.datablocks) {
      header.gen_code_custom.datablocks = new CodeBuilder();
      const b = header.gen_code_custom.datablocks;
      b.$generate_note()
    }
    const b = header.gen_code_custom.datablocks;
    const _gen_records = header.records.filter(_Gen.filter_record);

    // check records
    for (const record of _gen_records) {
      const gpu_cfg = record.ml_configs.gpu as RecordConfig;
      if (gpu_cfg.soa.enable && gpu_cfg.aos.enable) {
        throw new Error(`record ${record.name} cannot enable both soa and aos`);
      }
    }

    // filter soa & aos records
    const _gen_soa_records = _gen_records.filter(r => (r.ml_configs.gpu as RecordConfig).soa.enable);
    const _gen_aos_records = _gen_records.filter(r => (r.ml_configs.gpu as RecordConfig).aos.enable);
    const _gpu_records = _gen_aos_records.concat(_gen_soa_records)

    // header basic
    b.$line("#pragma once")

    // include files
    b.$line(`#include "SkrRenderer/shared/gpu_table.hpp"`)
    b.$line(``)

    // gen datablocks
    b.$line(`//! BEGIN Data Blocks`)
    b.$line(`namespace skr::gpu {`)

    if (_gen_soa_records.length > 0) {
      _gen_soa_records.forEach((record) => {
        const gpu_cfg = record.ml_configs.gpu as RecordConfig;
        b.$line(`template <>`)
        b.$line(`struct AOSOAInfo<${record.name}> {`)
        b.$line(`inline static constexpr bool IsSOA = true;`)
        b.$line(`inline static constexpr uint32_t SOAPageSize = ${gpu_cfg.soa.page_size};`)
        b.$line(`};`)
      })
    }

    if (_gpu_records.length > 0) {
      _gpu_records.forEach((record) => {
        const is_soa = (record.ml_configs.gpu as RecordConfig).soa.enable;

        b.$line(`template <>`)
        b.$line(`struct GPUDatablock<${record.name}> {`)
        b.$indent(_b => {
          // build size traits
          const size_expr = record.fields
            .map(f => `GPUDatablock<${f.type}>::Size`)
            .join("");
          b.$line(`inline static constexpr uint32_t Size =`)
          b.$indent(_b => {
            record.fields.forEach((f, idx) => {
              const suffix = idx < record.fields.length - 1 ? " +" : ";";
              b.$line(`GPUDatablock<${f.type}>::Size${suffix}`);
            })
          })
          b.$line(``)

          // build ctor
          b.$line(`GPUDatablock() = default;`)
          b.$line(`inline GPUDatablock<${record.name}>(const ${record.name}& v)`)
          b.$indent(_b => {
            record.fields.forEach((f, idx) => {
              const prefix = idx === 0 ? ":" : ",";
              b.$line(`${prefix} _${f.short_name}(v.${f.short_name})`);
            });
          })
          b.$line(`{}`)
          b.$line(``)

          // build operator
          b.$line(`inline operator ${record.name}() const {`)
          b.$indent(_b => {
            b.$line(`${record.name} v;`)
            record.fields.forEach((f) => {
              b.$line(`v.${f.short_name} = _${f.short_name};`)
            });
            b.$line(`return v;`)
          })
          b.$line(`}`)
          b.$line(``)

          b.$line(`#ifndef __CPPSL__`)
          if (is_soa) {
            b.$line(`static void SetupTableConfig(gpu::TableConfig& config) {`)
            var index = 0;
            b.$indent(_b => {
              b.$line(`config.with_page_size(AOSOAInfo<${record.name}>::SOAPageSize);`)
              record.fields.forEach((f) => {
                b.$line(`config.add_component(${index++}, sizeof(gpu::GPUDatablock<${f.type}>));`)
              });
            })
            b.$line(`}`)
            b.$line(``)
            b.$line(`static void StoreInstance(gpu::TableInstance& Table, uint32_t IndexInTable, const ${record.name}& v) {`)
            var index = 0;
            b.$indent(_b => {
              record.fields.forEach((f) => {
                b.$line(`Table.Store(${index++}, IndexInTable, v.${f.short_name});`)
              });
            })
            b.$line(`}`)
          }
          else {
            b.$line(`static void SetupTableConfig(gpu::TableConfig& config) {`)
            b.$indent(_b => {
              b.$line(`config.with_page_size(AOSOAInfo<${record.name}>::SOAPageSize);`)
              b.$line(`config.add_component(0, sizeof(gpu::GPUDatablock<${record.name}>));`)
            })
            b.$line(`}`)
            b.$line(``)
            b.$line(`static void StoreInstance(gpu::TableInstance& Table, uint32_t IndexInTable, const ${record.name}& v) {`)
            b.$indent(_b => {
              b.$line(`Table.Store(0, IndexInTable, v);`)
            })
            b.$line(`}`)
          }
          b.$line(`#endif`)

          if (is_soa) {
            b.$line(`#ifdef __CPPSL__`)
            b.$line(`template <typename ByteBufferType>`)
            b.$line(`static ${record.name} LoadAllSOAElements(ByteBufferType buffer, uint32_t instance, uint32_t buffer_offset) {`)
            b.$indent(_b => {
              b.$line(`${record.name} v;`)
              b.$line(`const auto row = Row<${record.name}>(instance, buffer_offset);`)
              record.fields.forEach((f) => {
                b.$line(`v.${f.short_name} = row.Load<SubBlock(${record.name}, ${f.short_name})>(buffer);`)
              });
              b.$line(`return v;`)
            })
            b.$line(`}`)
            b.$line(`#endif`)
          }

          record.fields.forEach((f) => {
            b.$line(`const GPUDatablock<${f.type}> _${f.short_name};`)
          })
        });
        b.$line(`};`)
      });
    }

    b.$line(`} // namespace skr::gpu`)
    b.$line(`//! END Data Blocks`)
  }
  static filter_record(record: db.Record) {
    return record.ml_configs.gpu.enable;
  }
}

class GPUDataBlock extends gen.Generator {
  override inject_configs(): void {
    // record
    this.main_module_db.each_record((record) => {
      record.ml_configs.gpu = new RecordConfig();
    })
  }
  override gen(): void {
    // gen headers
    this.main_module_db.headers.forEach((header) => {
      _Gen.header_data_blocks(header);
    });
  }
}

export function load_generator(gm: gen.GenerateManager) {
  gm.add_generator("gpu_data_block", new GPUDataBlock());
}