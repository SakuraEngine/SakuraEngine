#pragma once
#include "SkrDAScript/module.hpp"
#include "SkrDAScript/function.hpp"

namespace skr {
namespace das {

template<bool IS_REF, bool IS_CMRES, typename callT> 
struct register_property;

template <typename FuncT, FuncT fn, bool IS_REF = false>
inline BuiltInFunction add_extern_property(Module* mod, const Library* lib, const char8_t* name, const char8_t * cppName = nullptr);

struct LibraryDescriptor
{
    uint32_t init_mod_counts = 0;
    skr::das::Module* const* init_mods = nullptr;
    bool init_builtin_mods = false;
};

struct SKR_DASCRIPT_API Library
{
    static Library* Create(const LibraryDescriptor& desc) SKR_NOEXCEPT;
    static void Free(Library* library) SKR_NOEXCEPT;

    virtual ~Library() SKR_NOEXCEPT;
    virtual Module* get_this_module() const SKR_NOEXCEPT = 0;

    virtual void add_module(Module* mod) SKR_NOEXCEPT = 0;
    virtual void add_builtin_module() SKR_NOEXCEPT = 0;

    template<bool IS_REF, bool IS_CMRES, typename callT> 
    void register_property(const char8_t* dotName, const char8_t* cppPropName);
};

} // namespace das
} // namespace skr

namespace skr {
namespace das {

template <typename FuncT, FuncT fn, bool IS_REF>
FORCEINLINE BuiltInFunction add_extern_property(Module* mod, const Library* lib, const char8_t* name, const char8_t* cppName)
{
    /*
    fnX->arguments[0]->type->explicitConst = false;
    fnX->setSideEffects(SideEffects::none);
    mod.addFunction(fnX,true);  // yes, this one can fail. same C++ bound property can be in multiple classes before or after refactor
    return fnX;
    */
    auto f = BuiltInFunction::MakeExternFunction<FuncT, fn, IS_REF>(lib, name, cppName);
    f.set_is_property(true);
    mod->add_function(f);
    return f;
}

template<bool IS_CMRES, typename callT> 
struct register_property<true, IS_CMRES, callT> 
{
    FORCEINLINE static void reg(Library* lib, const char8_t* dotName, const char8_t* cppPropName)
    // , bool explicitConst=false, SideEffects sideEffects = SideEffects::none ) 
    {
        add_extern_property<decltype(&callT::static_call), callT::static_call, true>(
            lib->get_this_module(), lib, dotName, cppPropName
        ).args({u8"this"});
    }
};

template<bool IS_CMRES, typename callT> 
struct register_property<false, IS_CMRES, callT> 
{
    FORCEINLINE static void reg(Library* lib, const char8_t* dotName, const char8_t* cppPropName)
    // , bool explicitConst=false, SideEffects sideEffects = SideEffects::none ) 
    {
        add_extern_property<decltype(&callT::static_call), callT::static_call>(
            lib->get_this_module(), lib, dotName, cppPropName
        ).args({u8"this"});
    }
};

template<bool IS_REF, bool IS_CMRES, typename callT> 
void Library::register_property(const char8_t* dotName, const char8_t* cppPropName)
{
    skr::das::register_property<callT::ref, IS_CMRES, callT>::reg(this, dotName, cppPropName);
}

} // namespace das
} // namespace skr