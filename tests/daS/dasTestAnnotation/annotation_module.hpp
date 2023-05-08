#pragma once
#include "SkrDAScript/daScript.hpp"

// example type, which we are going to expose to das
struct Color {
    uint8_t r, g, b, a;
    float luminance() const { return 0.2126f*r + 0.7152f*g + 0.0722f*b; }
};

struct AnnotationRegister_Color
{
    AnnotationRegister_Color(skr::das::Library* lib)
    {
        annotation = skr::das::StructureAnnotation::Create(u8"Color", u8"Color", lib);
        colorType = skr::das::TypeDecl::Create<Color>(lib, u8"Color");
        annotation->add_field(u8"r", u8"r", offsetof(Color, r), skr::das::EBuiltinType::UINT8);
        annotation->add_field(u8"g", u8"g", offsetof(Color, g), skr::das::EBuiltinType::UINT8);
        annotation->add_field(u8"b", u8"b", offsetof(Color, b), skr::das::EBuiltinType::UINT8);
        annotation->add_field(u8"a", u8"a", offsetof(Color, a), skr::das::EBuiltinType::UINT8);
    }

    ~AnnotationRegister_Color()
    {
        skr::das::TypeDecl::Free(colorType);
        skr::das::TypeDecl::Free(u8Type);
        skr::das::StructureAnnotation::Free(annotation);
    }

    skr::das::StructureAnnotation* annotation = nullptr;
    skr::das::TypeDecl* u8Type = nullptr;
    skr::das::TypeDecl* colorType = nullptr;
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
        mod->add_annotation(cReg.annotation);
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