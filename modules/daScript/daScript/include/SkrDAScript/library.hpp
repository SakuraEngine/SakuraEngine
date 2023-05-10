#pragma once
#include "SkrDAScript/module.hpp"

namespace skr {
namespace das {

template <typename FuncT, FuncT fn, bool CallRef = false>
inline auto add_extern_property(Module& mod, const Library& lib, const char8_t* name, const char8_t * cppName = nullptr);

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
};

} // namespace das
} // namespace skr

namespace skr {
namespace das {

template <typename FuncT, FuncT fn, bool CallRef>
inline auto add_extern_property(Module& mod, const Library& lib, const char8_t* name, const char8_t * cppName)
{
    /*
    using SimNodeType = SimNodeT<FuncT, fn>;
    auto fnX = make_smart<ExternalFn<FuncT, fn, SimNodeType, FuncT>>(name, lib, cppName);
    defaultTempFn tempFn;
    tempFn(fnX.get());
    fnX->arguments[0]->type->explicitConst = false;
    fnX->setSideEffects(SideEffects::none);
    fnX->propertyFunction = true;
    mod.addFunction(fnX,true);  // yes, this one can fail. same C++ bound property can be in multiple classes before or after refactor
    return fnX;
    */
    return nullptr;
}

template<int isRef, typename callT> struct register_property;

template<typename callT> 
struct register_property<true, callT> 
{
    FORCEINLINE static void reg(Library* lib, const char8_t* dotName, const char8_t* cppPropName)
    // , bool explicitConst=false, SideEffects sideEffects = SideEffects::none ) 
    {
        add_extern_property<decltype(&callT::static_call), callT::static_call, true>(
            *(lib->get_this_module()), *lib, dotName, cppPropName
        );//->args({"this"});
    }
};

template<typename callT> 
struct register_property<false, callT> {
    FORCEINLINE static void reg(Library* lib, const char8_t* dotName, const char8_t* cppPropName)
    // , bool explicitConst=false, SideEffects sideEffects = SideEffects::none ) 
    {
        add_extern_property<decltype(&callT::static_call), callT::static_call>(
            *(lib->get_this_module()), *lib, dotName, cppPropName
        );//->args({"this"});
    }
};

} // namespace das
} // namespace skr