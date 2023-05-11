#pragma once
#include "SkrDAScript/daScript.hpp"
#include "utils/log.h"

// example type, which we are going to expose to das
struct Color 
{
    uint8_t r, g, b, a;
    float f;
    float f1;
    float luminance() const 
    { 
        const auto l = 0.2126f*r + 0.7152f*g + 0.0722f*b; 
        return l;
    }
};
SKR_DASCRIPT_INLINE_TYPE_FACTORY(Color, Color);

struct AnnotationRegister_Color
{
    AnnotationRegister_Color(skr::das::Library* lib)
    {
        using namespace skr::das;
        colorAnnotation = StructureAnnotation::Create<Color>(lib, u8"Color");
        auto cA = colorAnnotation;
        cA->add_field(offsetof(Color, r), EBuiltinType::UINT8, u8"r");
        cA->add_field(offsetof(Color, g), EBuiltinType::UINT8, u8"g");
        cA->add_field(offsetof(Color, b), EBuiltinType::UINT8, u8"b");
        cA->add_field(offsetof(Color, a), EBuiltinType::UINT8, u8"a");
        cA->add_field(offsetof(Color, f), EBuiltinType::FLOAT2, u8"f2");
        cA->add_property<decltype(&Color::luminance), &Color::luminance>(u8"luminance");
        colorType = skr::das::TypeDecl::MakeType<Color>(lib);
    }

    ~AnnotationRegister_Color()
    {
        skr::das::StructureAnnotation::Free(colorAnnotation);
    }

    skr::das::StructureAnnotation* colorAnnotation = nullptr;
    skr::das::TypeDecl colorType = nullptr;
};

// custom function, which takes type as an input, as well as returns it
inline Color makeGray ( const Color & c ) {
    uint8_t lum = uint8_t ( std::min ( c.luminance(), 255.0f ) );
    return  { lum, lum, lum, c.a };
}

struct ModuleRegister_TestAnnotation
{
    ModuleRegister_TestAnnotation()
        : mod(skr::das::Module::Create(u8"test_annotation")),
          mod_lib(skr::das::Library::Create({1, &mod, true})),
          cReg(mod_lib)
    {
        mod->add_annotation(cReg.colorAnnotation);
        // addExtern<DAS_BIND_FUN(makeGray),SimNode_ExtFuncCallAndCopyOrMove>(*this, lib, "make_gray",
        //    SideEffects::none, "makeGray");
    }

    ~ModuleRegister_TestAnnotation()
    {
        skr::das::Library::Free(mod_lib);
        skr::das::Module::Free(mod);
    }

    skr::das::Module* mod = nullptr;
    skr::das::Library* mod_lib = nullptr;
    AnnotationRegister_Color cReg;
};