#include <iostream>
#include <cassert>
#include <array> // msvc will complain std::array is uninstantiated if <array> is removed
#include "CppSL/Constant.hpp"
#include "CppSL/Decl.hpp"
#include "CppSL/CppSLAST.hpp"
#include "CppSL/langs/HLSLGenerator.hpp"
#include "CppSL/langs/MSLGenerator.hpp"

void mandelbrot(skr::CppSL::AST& AST)
{
    using namespace skr::CppSL;
    std::vector<ParamVarDecl*> cos_params = { AST.DeclareParam(EVariableQualifier::None, AST.Float3Type, L"v") };
    auto cos_func = AST.DeclareFunction(L"cos", AST.Float3Type, cos_params, nullptr);
    std::vector<ParamVarDecl*> dot_params = {
        AST.DeclareParam(EVariableQualifier::None, AST.Float2Type, L"a"),
        AST.DeclareParam(EVariableQualifier::None, AST.Float2Type, L"b")
    };
    auto dot_func = AST.DeclareFunction(L"dot", AST.FloatType, dot_params, nullptr);

    using namespace skr::CppSL;
    FunctionDecl* mandelbrot = nullptr;
    {
        auto tid = AST.DeclareParam(EVariableQualifier::None, AST.UInt2Type, L"tid");
        auto tsize = AST.DeclareParam(EVariableQualifier::None, AST.UInt2Type, L"tsize");
        std::vector<ParamVarDecl*> mandelbrot_params = { tid, tsize };
        auto mandelbrot_body = AST.Block({});
        mandelbrot = AST.DeclareFunction(L"mandelbrot", AST.Float4Type, mandelbrot_params, mandelbrot_body);

        // const float x = float(tid.x) / (float)tsize.x;
        auto x = AST.Variable(EVariableQualifier::Const, AST.FloatType, L"x", AST.Div(AST.StaticCast(AST.FloatType, AST.Access(tid->ref(), AST.Constant(IntValue(0)))), AST.StaticCast(AST.FloatType, AST.Access(tsize->ref(), AST.Constant(IntValue(0))))));
        mandelbrot_body->add_statement(x);

        // const float y = float(tid.y) / (float)tsize.y;
        auto y = AST.Variable(EVariableQualifier::Const, AST.FloatType, L"y", AST.Div(AST.StaticCast(AST.FloatType, AST.Access(tid->ref(), AST.Constant(IntValue(1)))), AST.StaticCast(AST.FloatType, AST.Access(tsize->ref(), AST.Constant(IntValue(1))))));
        mandelbrot_body->add_statement(y);

        // const float2 uv = float2(x, y);
        std::vector<Expr*> uv_inits = { x->ref(), y->ref() };
        auto uv = AST.Variable(EVariableQualifier::Const, AST.Float2Type, L"uv", AST.Construct(AST.Float2Type, uv_inits));
        mandelbrot_body->add_statement(uv);

        // float n = 0.0f;
        auto n = AST.Variable(EVariableQualifier::None, AST.FloatType, L"n", AST.Constant(FloatValue("0.0f")));
        mandelbrot_body->add_statement(n);

        // float2 c = float2(-0.444999992847442626953125f, 0.0f);
        std::vector<Expr*> c_inits = {
            AST.Constant(FloatValue("-0.444999992847442626953125f")),
            AST.Constant(FloatValue("0.0f"))
        };
        auto c = AST.Variable(EVariableQualifier::None, AST.Float2Type, L"c", AST.Construct(AST.Float2Type, c_inits));
        mandelbrot_body->add_statement(c);

        // c = c + (uv - float2(0.5f, 0.5f)) * 2.3399999141693115234375f;
        auto vec_05_05_init = std::vector<Expr*>{
            AST.Constant(FloatValue("0.5f")),
            AST.Constant(FloatValue("0.5f"))
        };
        auto modify_c = AST.Assign(c->ref(),
            AST.Add(c->ref(), AST.Mul(AST.Sub(uv->ref(), AST.Construct(AST.Float2Type, vec_05_05_init)), AST.Constant(FloatValue("2.3399999141693115234375f")))));
        mandelbrot_body->add_statement(modify_c);

        // float2 z = float2(0.f, 0.f);
        std::vector<Expr*> z_inits = {
            AST.Constant(FloatValue("0.f")),
            AST.Constant(FloatValue("0.f"))
        };
        auto z = AST.Variable(EVariableQualifier::None, AST.Float2Type, L"z", AST.InitList(z_inits));
        mandelbrot_body->add_statement(z);

        // const int M = 128;
        auto M = AST.Variable(EVariableQualifier::Const, AST.IntType, L"M", AST.Constant(IntValue(128)));
        mandelbrot_body->add_statement(M);

        // TODO: FOR LOOP
        /*
            for (int i = 0; i < M; i++) {
                z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
                if (dot(z, z) > 2.0f) {
                    break;
                }
                n += 1.0f;
            }
        */
        auto for_init = AST.Variable(EVariableQualifier::None, AST.IntType, L"i", AST.Constant(IntValue(0)));
        auto for_cond = AST.Less(for_init->ref(), AST.Unary(UnaryOp::PLUS, M->ref()));
        auto for_inc = AST.AddAssign(for_init->ref(), AST.Constant(IntValue(1)));
        auto for_body = AST.Block({});

        // z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
        {
            auto temp_a = AST.Mul(AST.Access(z->ref(), AST.Constant(IntValue(0))), AST.Access(z->ref(), AST.Constant(IntValue(0))));
            auto temp_b = AST.Mul(AST.Access(z->ref(), AST.Constant(IntValue(1))), AST.Access(z->ref(), AST.Constant(IntValue(1))));
            auto temp_c = AST.Sub(temp_a, temp_b);

            auto temp_d = AST.Mul(AST.Constant(FloatValue("2.0f")), AST.Access(z->ref(), AST.Constant(IntValue(0))));
            auto temp_e = AST.Mul(temp_d, AST.Access(z->ref(), AST.Constant(IntValue(1))));
            std::vector<Expr*> z_inits_for = { temp_c, temp_e };
            auto temp_f = AST.Construct(AST.Float2Type, z_inits_for);
            auto modify_z = AST.Assign(z->ref(), AST.Add(temp_f, c->ref()));
            for_body->add_statement(modify_z);
        }

        // if (dot(z, z) > 2.0f) break;
        {
            std::vector<Expr*> dot_args = { z->ref(), z->ref() };
            auto temp_a = AST.CallFunction(dot_func->ref(), dot_args);
            auto if_stmt = AST.If(AST.Greater(temp_a, AST.Constant(FloatValue("2.0f"))), AST.Block({ AST.Break() }));
            for_body->add_statement(if_stmt);
        }

        // n += 1.0f;
        auto increment_n = AST.AddAssign(n->ref(), AST.Constant(FloatValue("1.0f")));
        for_body->add_statement(increment_n);

        mandelbrot_body->add_statement(AST.For(for_init, for_cond, for_inc, for_body));

        // const float t = float(n) / float(M);
        auto t = AST.Variable(EVariableQualifier::Const, AST.FloatType, L"t", AST.Div(n->ref(), AST.StaticCast(AST.FloatType, M->ref())));
        mandelbrot_body->add_statement(t);

        // const float3 d = float3(0.3f, 0.3f, 0.5f);
        std::vector<Expr*> d_inits = {
            AST.Constant(FloatValue("0.3f")),
            AST.Constant(FloatValue("0.3f")),
            AST.Constant(FloatValue("0.5f"))
        };
        auto d = AST.Variable(EVariableQualifier::Const, AST.Float3Type, L"d", AST.Construct(AST.Float3Type, d_inits));
        mandelbrot_body->add_statement(d);

        // const float3 e = float3(-0.2f, -0.3f, -0.5f);
        std::vector<Expr*> e_inits = {
            AST.Constant(FloatValue("-0.2f")),
            AST.Constant(FloatValue("-0.3f")),
            AST.Constant(FloatValue("-0.5f"))
        };
        auto e = AST.Variable(EVariableQualifier::Const, AST.Float3Type, L"e", AST.Construct(AST.Float3Type, e_inits));
        mandelbrot_body->add_statement(e);

        // const float3 f = float3(2.1f, 2.0f, 3.0f);
        std::vector<Expr*> f_inits = {
            AST.Constant(FloatValue("2.1f")),
            AST.Constant(FloatValue("2.0f")),
            AST.Constant(FloatValue("3.0f"))
        };
        auto f = AST.Variable(EVariableQualifier::Const, AST.Float3Type, L"f", AST.Construct(AST.Float3Type, f_inits));
        mandelbrot_body->add_statement(f);

        // const float3 g = float3(0.0f, 0.1f, 0.0f);
        std::vector<Expr*> g_inits = {
            AST.Constant(FloatValue("0.0f")),
            AST.Constant(FloatValue("0.1f")),
            AST.Constant(FloatValue("0.0f"))
        };
        auto g = AST.Variable(EVariableQualifier::Const, AST.Float3Type, L"g", AST.Construct(AST.Float3Type, g_inits));
        mandelbrot_body->add_statement(g);

        // return float4(d + (e * cos(((f * t) + g) * 2.f * pi)), 1.0f);
        auto temp0 = AST.Add(AST.Mul(f->ref(), t->ref()), g->ref());
        Expr* temp1 = AST.Mul(AST.Mul(temp0, AST.Constant(FloatValue("2.f"))), AST.Constant(FloatValue("3.14159265358979323846f"))); // pi
        auto temp2 = AST.CallFunction(cos_func->ref(), std::span<Expr*>(&temp1, 1));
        auto temp3 = AST.Mul(e->ref(), temp2);
        auto temp4 = AST.Add(d->ref(), temp3);
        std::vector<Expr*> return_inits = { temp4, AST.Constant(FloatValue("1.0f")) };
        auto return_value = AST.Construct(AST.Float4Type, return_inits);
        mandelbrot_body->add_statement(AST.Return(return_value));
    }
    {
        auto kernel_body = AST.Block({});
        auto sv_tid = AST.DeclareParam(EVariableQualifier::Const, AST.UInt2Type, L"tid");
        sv_tid->add_attr(AST.DeclareAttr<BuiltinAttr>(L"ThreadID"));
        auto output_buf = AST.DeclareGlobalResource(AST.StructuredBuffer(AST.Float4Type, BufferFlags::ReadWrite), L"output");
        output_buf->add_attr(AST.DeclareAttr<ResourceBindAttr>());
        std::vector<ParamVarDecl*> kernel_params = { sv_tid };
        auto kernel = AST.DeclareFunction(L"kernel", AST.VoidType, kernel_params, kernel_body);
        kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Compute));
        kernel->add_attr(AST.DeclareAttr<KernelSizeAttr>(32, 32, 1));

        // const uint2 tsize = uint2(1024, 1024);
        std::vector<Expr*> tsize_inits = { AST.Constant(IntValue(1024)), AST.Constant(IntValue(1024)) };
        auto tsize = AST.Variable(EVariableQualifier::Const, AST.UInt2Type, L"tsize", AST.Construct(AST.UInt2Type, tsize_inits));
        kernel_body->add_statement(tsize);
        auto row_pitch = AST.Variable(EVariableQualifier::Const, AST.UIntType, L"row_pitch", AST.Access(tsize->ref(), AST.Constant(IntValue(0))));
        kernel_body->add_statement(row_pitch);

        // output.store(tid.x + tid.y * row_pitch, mandelbrot(tid, tsize));
        auto tid_x = AST.Access(sv_tid->ref(), AST.Constant(IntValue(0)));
        auto tid_y = AST.Access(sv_tid->ref(), AST.Constant(IntValue(1)));
        auto output_index = AST.Add(tid_x, AST.Mul(tid_y, row_pitch->ref()));

        std::vector<Expr*> mandelbrot_args = { sv_tid->ref(), tsize->ref() };
        auto mandelbrot_call = AST.CallFunction(mandelbrot->ref(), mandelbrot_args);

        auto StoreIntrin = AST.FindIntrinsic("BUFFER_WRITE");
        std::vector<const TypeDecl*> arg_types = { &output_buf->type(), AST.UIntType, AST.Float4Type };
        std::vector<EVariableQualifier> arg_qualifiers = { EVariableQualifier::Inout, EVariableQualifier::None, EVariableQualifier::None };
        auto StoreFunction = AST.SpecializeTemplateFunction(StoreIntrin, arg_types, arg_qualifiers);
        std::vector<Expr*> store_args = { output_buf->ref(), output_index, mandelbrot_call };
        auto output_store = AST.CallFunction(StoreFunction->ref(), std::span<Expr*>(store_args));

        kernel_body->add_statement(output_store);
    }
}

int main()
{
    using namespace skr::CppSL;
    AST AST;

    // some_test(AST);
    mandelbrot(AST);

    HLSL::HLSLGenerator hlsl_generator = {};
    SourceBuilderNew hlsl_sb = {};
    std::wcout << hlsl_generator.generate_code(hlsl_sb, AST) << std::endl;

    MSL::MSLGenerator msl_generator = {};
    SourceBuilderNew msl_sb = {};
    std::wcout << msl_generator.generate_code(msl_sb, AST) << std::endl;

    std::wcout << AST.dump() << std::endl;

    return 0;
}