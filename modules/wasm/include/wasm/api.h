#pragma once
#include "wasm_configure.h"
#ifdef USE_M3
    #include "wasm3/wasm3.h"
#endif

#define swa_handle_error(error)        \
    {                                  \
        swa_error("error: %s", error); \
        swa_assert(0 && (error));      \
    }

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SWAInstance SWAInstance;
typedef struct SWAInstanceDescriptor SWAInstanceDescriptor;
typedef const struct SWAInstance* SWAInstanceId;

typedef struct SWARuntime SWARuntime;
typedef struct SWARuntimeDescriptor SWARuntimeDescriptor;
typedef const struct SWARuntime* SWARuntimeId;

typedef struct SWAHostFunctionDescriptor SWAHostFunctionDescriptor;
typedef struct SWAModule SWAModule;
typedef struct SWAModuleDescriptor SWAModuleDescriptor;
typedef const struct SWAModule* SWAModuleId;

typedef struct SWAFunction SWAFunction;
typedef const struct SWAFunction* SWAFunctionId;
typedef const char* SWAExecResult;

typedef enum ESWAValueType
{
    SWA_VAL_I32 = 0x7FU,
    SWA_VAL_I64 = 0x7EU,
    SWA_VAL_F32 = 0x7DU,
    SWA_VAL_F64 = 0x7CU,
    SWA_VAL_V128 = 0x7BU,
    SWA_VAL_FuncRef = 0x70U,
    SWA_VAL_ExternRef = 0x6FU
} ESWAValueType;

typedef struct SWAValue {
    union
    {
        swa_f32 f;
        swa_f64 F;
        swa_i32 i;
        swa_i64 I;
        swa_ptr ptr;
        swa_cptr cptr;
    };
    ESWAValueType type;
#ifdef __cplusplus
    /* clang-format off */
    SKR_FORCEINLINE operator swa_f32() const { return f; }
    SKR_FORCEINLINE operator swa_f64() const { return F; }
    SKR_FORCEINLINE operator swa_i32() const { return i; }
    SKR_FORCEINLINE operator swa_i64() const { return I; }
    SKR_FORCEINLINE operator swa_ptr() const { return ptr; }
    SKR_FORCEINLINE operator swa_cptr() const { return cptr; }
    SWAValue() : I(0) {}
    SWAValue(swa_f32 f) : f(f), type(SWA_VAL_F32) {}
    SWAValue(swa_f64 F) : F(F), type(SWA_VAL_F64) {}
    SWAValue(swa_i32 i) : i(i), type(SWA_VAL_I32) {}
    SWAValue(swa_i64 I) : I(I), type(SWA_VAL_I64) {}
    SWAValue(swa_ptr ptr) : ptr(ptr), type(SWA_VAL_I64) {}
    SWAValue(swa_cptr cptr) : cptr(cptr), type(SWA_VAL_I64) {}
    /* clang-format on */
#endif
} SWAValue;

typedef struct SWAExecDescriptor {
    const uint32_t param_count;
    const SWAValue* params;
    const uint32_t ret_count;
    SWAValue* rets;
} SWAExecDescriptor;

typedef struct SWANamedObjectTable SWANamedObjectTable;

typedef enum ESWABackend
{
    ESWA_BACKEND_UNDEFINED,
#ifdef USE_M3
    ESWA_BACKEND_WASM3,
#endif
    ESWA_BACKEND_EMSCRIPTON,
    ESWA_BACKEND_MAX_ENUM_BIT = 0x7FFFFFFF
} ESWABackend;

// Instance APIs
SKR_WASM_API SWAInstanceId swa_create_instance(const struct SWAInstanceDescriptor* desc);
typedef SWAInstanceId (*SWAProcCreateInstance)(const struct SWAInstanceDescriptor* desc);
SKR_WASM_API void swa_free_instance(SWAInstanceId instance);
typedef void (*SWAProcFreeInstance)(SWAInstanceId instance);

// Runtime APIs
SKR_WASM_API SWARuntimeId swa_create_runtime(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
typedef SWARuntimeId (*SWAProcCreateRuntime)(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
SKR_WASM_API void swa_free_runtime(SWARuntimeId runtime);
typedef void (*SWAProcFreeRuntime)(SWARuntimeId runtime);

// Module APIs
SKR_WASM_API SWAModuleId swa_create_module(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
typedef SWAModuleId (*SWAProcCreateModule)(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
SKR_WASM_API void swa_module_link_host_function(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
typedef void (*SWAProcModuleLinkHostFunction)(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
SKR_WASM_API void swa_free_module(SWAModuleId module);
typedef void (*SWAProcFreeModule)(SWAModuleId module);

// Function APIs
SKR_WASM_API SWAExecResult swa_exec(SWAModuleId runtime, const char8_t* const name, SWAExecDescriptor* desc);
typedef SWAExecResult (*SWAProcExec)(SWAModuleId runtime, const char8_t* const name, SWAExecDescriptor* desc);

// Util X
SKR_WASM_API SWARuntimeId swa_instance_try_find_runtime(SWAInstanceId instance, const char* name);
SKR_WASM_API SWAModuleId swa_runtime_try_find_module(SWARuntimeId runtime, const char* name);

typedef struct SWAProcTable {
    // Instance APIs
    const SWAProcCreateInstance create_instance;
    const SWAProcFreeInstance free_instance;

    // Runtime APIs
    const SWAProcCreateRuntime create_runtime;
    const SWAProcFreeRuntime free_runtime;

    // Module APIs
    const SWAProcCreateModule create_module;
    const SWAProcModuleLinkHostFunction link_host_function;
    const SWAProcFreeModule free_module;

    // Function APIs
    const SWAProcExec exec;
} SWAProcTable;

typedef struct SWAInstance {
    const SWAProcTable* proc_table;
    struct SWANamedObjectTable* runtimes;
} SWAInstance;

typedef struct SWAInstanceDescriptor {
    ESWABackend backend;
} SWAInstanceDescriptor;

typedef struct SWARuntime {
    SWAInstanceId instance;
    const char8_t* name;
    const SWAProcTable* proc_table;
    struct SWANamedObjectTable* modules;
} SWARuntime;

typedef struct SWARuntimeDescriptor {
    const char8_t* name;
    uint32_t stack_size;
} SWARuntimeDescriptor;

typedef struct SWAModuleDescriptor {
    const char8_t* name;
    const uint8_t* wasm;
    uint32_t wasm_size;
    uint8_t bytes_pinned_outside;
    uint8_t strong_stub;
} SWAModuleDescriptor;

typedef struct SWAModule {
    SWARuntimeId runtime;
    const char8_t* name;
    uint8_t* wasm;
    uint32_t wasm_size;
    uint8_t bytes_pinned_outside;

    uint8_t strong_stub;
    uint8_t instantiated;
} SWAModule;

typedef struct SWAHostFunctionDescriptor {
    const char* module_name;
    const char* function_name;
    void* proc;
    struct
    {
#ifdef USE_M3
        M3RawCall m3;
#endif
    } backend_wrappers;
    struct
    {
#ifdef USE_M3
        const char* m3;
#endif
    } signatures;
} SWAHostFunctionDescriptor;

#ifdef __cplusplus
} // end extern "C"

    #ifdef USE_M3
        #include "wasm/backend/wasm3/utilx.inl"
    #endif
    #ifdef USE_WASM_EDGE
        #include "wasm/backend/wasmedge/utilx.inl"
    #endif
namespace swa
{
class utilx
{
public:
    // linkage
    template <typename Func>
    SKR_FORCEINLINE static SWAHostFunctionDescriptor linkage(const char* module_name,
        const char* function_name, Func* function)
    {
        SWAHostFunctionDescriptor out = {};
    #ifdef USE_M3
        m3::utilx<Func>::fill_linkage(out, module_name, function_name, function);
    #endif
    #ifdef USE_WASM_EDGE
        wa_edge::utilx<Func>::fill_linkage(out, module_name, function_name, function);
    #endif
        return out;
    }
    template <typename Func>
    SKR_FORCEINLINE static void link(SWAModuleId module, const char* module_name,
        const char* function_name, Func* function)
    {
        SWAHostFunctionDescriptor host_func = linkage(module_name, function_name, function);
        swa_module_link_host_function(module, &host_func);
    }

    // execution
    struct executor {
        template <typename RetT, typename... Args>
        SKR_FORCEINLINE RetT exec(Args&&... args)
        {
            SWAValue ret;
            const SWAValue iargs[] = { std::forward<Args>(args)... };
            SWAExecDescriptor exec_desc = {
                sizeof...(Args), iargs,
                1, &ret
            };
            auto error = ::swa_exec(module, function_name, &exec_desc);
            if (error) { swa_handle_error(error); }
            return RetT(ret);
        }
        template <typename RetT>
        SKR_FORCEINLINE RetT exec()
        {
            SWAValue ret;
            SWAExecDescriptor exec_desc = {
                0, nullptr,
                1, &ret
            };
            auto error = ::swa_exec(module, function_name, &exec_desc);
            if (error) { swa_handle_error(error); }
            return RetT(ret);
        }
        executor(SWAModuleId module, const char* function_name)
            : module(module)
            , function_name(function_name)
        {
        }
        SWAModuleId module;
        const char* function_name;
    };
};
} // namespace swa

#endif