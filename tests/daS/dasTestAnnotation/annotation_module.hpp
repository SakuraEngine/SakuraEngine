#include "daScript/daScript.h"

using namespace das;

// example type, which we are going to expose to das
struct Color {
    uint8_t r, g, b, a;
    float luminance() const { return 0.2126f*r + 0.7152f*g + 0.0722f*b; }
};

// create type annotation, which exposes it to C++
struct ColorAnnotation : public ManagedStructureAnnotation<Color,true,true> {
    ColorAnnotation(ModuleLibrary & ml) : ManagedStructureAnnotation ("Color", ml) {
        // expose individual fields
        addField<DAS_BIND_MANAGED_FIELD(r)>("r");
        addField<DAS_BIND_MANAGED_FIELD(g)>("g");
        addField<DAS_BIND_MANAGED_FIELD(b)>("b");
        addField<DAS_BIND_MANAGED_FIELD(a)>("a");
        // expose properties
        addProperty<DAS_BIND_MANAGED_PROP(luminance)>("luminance");
    }
    virtual bool isLocal() const override { return true; }  // this ref-value can appear as local variable in das
    virtual bool canCopy() const override { return true; }  // this ref-value can be copied
    virtual bool canMove() const override { return true; }  // this ref-value can be moved
};

// custom function, which takes type as an input, as well as returns it
Color makeGray ( const Color & c ) {
    uint8_t lum = uint8_t ( min ( c.luminance(), 255.0f ) );
    return  { lum, lum, lum, c.a };
}

// making custom builtin module
class Module_Tutorial03 : public Module {
public:
    Module_Tutorial03() : Module("test_annotation") {   // module name, when used from das file
        ModuleLibrary lib;
        lib.addModule(this);
        lib.addBuiltInModule();
        // register custom type annotation
        addAnnotation(make_smart<ColorAnnotation>(lib));
        // note SimNode_ExtFuncCallAndCopyOrMove - this means function returns ref type value,
        //  which needs to be copied or moved
        addExtern<DAS_BIND_FUN(makeGray),SimNode_ExtFuncCallAndCopyOrMove>(*this, lib, "make_gray",
            SideEffects::none, "makeGray");
    }

    virtual ModuleAotType aotRequire ( TextWriter & ) const { return ModuleAotType::cpp; }
};