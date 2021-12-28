#include "gtest/gtest.h"
#include "wasm3/wasm3.h"
#include "wasm3/m3_api_libc.h"

#define d_m3HasUVWASI
#define LINK_WASI
#include "wasm3/m3_api_wasi.h"

#define d_m3HasTracer
#include "wasm3/m3_api_tracer.h"

#include "wasm3/m3_env.h"

#define GAS_LIMIT 500000000
#define GAS_FACTOR 10000LL
#define MAX_MODULES 16
static int64_t initial_gas = GAS_FACTOR * GAS_LIMIT;
static int64_t current_gas = GAS_FACTOR * GAS_LIMIT;
static bool is_gas_metered = false;

m3ApiRawFunction(metering_usegas)
{
    m3ApiGetArg(int32_t, gas)

        current_gas -= gas;

    if (M3_UNLIKELY(current_gas < 0))
    {
        m3ApiTrap("[trap] Out of gas");
    }
    m3ApiSuccess();
}

const char* modname_from_fn(const char* fn)
{
    const char* sep = "/\\:*?";
    char c;
    while ((c = *sep++))
    {
        const char* off = strrchr(fn, c) + 1;
        fn = (fn < off) ? off : fn;
    }
    return fn;
}

M3Result link_all(IM3Module module)
{
    M3Result res;
    res = m3_LinkSpecTest(module);
    if (res) return res;

    res = m3_LinkLibC(module);
    if (res) return res;

#if defined(LINK_WASI)
    res = m3_LinkWASI(module);
    if (res) return res;
#endif

#if defined(d_m3HasTracer)
    res = m3_LinkTracer(module);
    if (res) return res;
#endif

#if defined(GAS_LIMIT)
    res = m3_LinkRawFunction(module, "metering", "usegas", "v(i)", &metering_usegas);
    if (!res)
    {
        fprintf(stderr, "Warning: Gas is limited to %0.4f\n", (double)(current_gas) / GAS_FACTOR);
        is_gas_metered = true;
    }
    if (res == m3Err_functionLookupFailed) { res = NULL; }
#endif

    return res;
}

class WASM3Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        env = m3_NewEnvironment();
        unsigned stackSize = 64 * 1024;
        runtime = m3_NewRuntime(env, stackSize, NULL);
    }

    M3Result repl_load(const char* fn)
    {
        M3Result result = m3Err_none;
        IM3Module module = NULL;

        u8* wasm = NULL;
        u32 fsize = 0;

        FILE* f = fopen(fn, "rb");
        if (!f)
        {
            return "cannot open file";
        }
        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (fsize < 8)
        {
            result = "file is too small";
            goto on_error;
        }
        else if (fsize > 64 * 1024 * 1024)
        {
            result = "file is too big";
            goto on_error;
        }

        wasm = (u8*)malloc(fsize);
        if (!wasm)
        {
            result = "cannot allocate memory for wasm binary";
            goto on_error;
        }

        if (fread(wasm, 1, fsize, f) != fsize)
        {
            result = "cannot read file";
            goto on_error;
        }
        fclose(f);
        f = NULL;

        result = m3_ParseModule(env, &module, wasm, fsize);
        if (result) goto on_error;

        result = m3_LoadModule(runtime, module);
        if (result) goto on_error;

        m3_SetModuleName(module, modname_from_fn(fn));

        result = link_all(module);
        if (result) goto on_error;

        if (wasm_bins_qty < MAX_MODULES)
        {
            wasm_bins[wasm_bins_qty++] = wasm;
        }

        return result;
    on_error:
        m3_FreeModule(module);
        if (wasm) free(wasm);
        if (f) fclose(f);

        return result;
    }

    void print_gas_used()
    {
#if defined(GAS_LIMIT)
        if (is_gas_metered)
        {
            fprintf(stderr, "Gas used: %0.4f\n", (double)(initial_gas - current_gas) / GAS_FACTOR);
        }
#endif
    }

    M3Result repl_call(const char* name, int argc, const char* argv[])
    {
        IM3Function func;
        M3Result result = m3_FindFunction(&func, runtime, name);
        if (result) return result;

        if (argc && (!strcmp(name, "main") || !strcmp(name, "_main")))
        {
            return "passing arguments to libc main() not implemented";
        }

        if (!strcmp(name, "_start"))
        {
#if defined(LINK_WASI)
            // Strip wasm file path
            if (argc > 0)
            {
                argv[0] = modname_from_fn(argv[0]);
            }

            m3_wasi_context_t* wasi_ctx = m3_GetWasiContext();
            wasi_ctx->argc = argc;
            wasi_ctx->argv = argv;

            result = m3_CallArgv(func, 0, NULL);

            print_gas_used();

            if (result == m3Err_trapExit)
            {
                exit(wasi_ctx->exit_code);
            }

            return result;
#else
            return "WASI not linked";
#endif
        }

        int arg_count = m3_GetArgCount(func);
        int ret_count = m3_GetRetCount(func);
        if (argc < arg_count)
        {
            return "not enough arguments";
        }
        else if (argc > arg_count)
        {
            return "too many arguments";
        }

        result = m3_CallArgv(func, argc, argv);

        print_gas_used();

        if (result) return result;

        static uint64_t valbuff[128];
        static const void* valptrs[128];
        memset(valbuff, 0, sizeof(valbuff));
        for (int i = 0; i < ret_count; i++)
        {
            valptrs[i] = &valbuff[i];
        }
        result = m3_GetResults(func, ret_count, valptrs);
        if (result) return result;

        if (ret_count <= 0)
        {
            fprintf(stderr, "Result: <Empty Stack>\n");
        }
        for (int i = 0; i < ret_count; i++)
        {
            switch (m3_GetRetType(func, i))
            {
                case c_m3Type_i32:
                    fprintf(stderr, "Result: %" PRIi32 "\n", *(i32*)valptrs[i]);
                    break;
                case c_m3Type_i64:
                    fprintf(stderr, "Result: %" PRIi64 "\n", *(i64*)valptrs[i]);
                    break;
#if d_m3HasFloat
                case c_m3Type_f32:
                    fprintf(stderr, "Result: %" PRIf32 "\n", *(f32*)valptrs[i]);
                    break;
                case c_m3Type_f64:
                    fprintf(stderr, "Result: %" PRIf64 "\n", *(f64*)valptrs[i]);
                    break;
#endif
                default:
                    return "unknown return type";
            }
        }

        return result;
    }

    void TearDown() override
    {
        if (runtime) m3_FreeRuntime(runtime);
        if (env) m3_FreeEnvironment(env);
    }

    IM3Environment env;
    IM3Runtime runtime;

    u8* wasm_bins[MAX_MODULES];
    int wasm_bins_qty = 0;
};

TEST_F(WASM3Test, LoadAndIncrement)
{
    M3Result result = m3Err_none;
    const char* argFile = "increment.wasm";
    const char* argFunc = "loadAndIncrement";
    const char* args[] = { "2" };
    result = repl_load(argFile);
    result = repl_call(argFunc, 1, args);
}