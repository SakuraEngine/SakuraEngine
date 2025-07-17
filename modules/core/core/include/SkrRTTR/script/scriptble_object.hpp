#pragma once
#include "SkrRTTR/iobject.hpp"
#include "./stack_proxy.hpp"
#ifndef __meta__
    #include "SkrRTTR/script/scriptble_object.generated.h"
#endif

namespace skr
{
struct ScriptbleObject;

// clang-format off
sreflect_enum_class(guid = "a0393643-9f1b-423a-8754-f504bab41420")
EScriptbleObjectOwnership
// clang-format on
{
    None   = 0,
    Native = 1 >> 0,
    Script = 1 >> 1,
};

// clang-format off
sreflect_struct(guid = "66181cbc-69a0-41a5-899c-51c1c6d4ea3e")
SKR_CORE_API IScriptMixinCore : virtual public skr::IObject
// clang-format on
{
    SKR_GENERATE_BODY()
    virtual ~IScriptMixinCore() = default;

    virtual void on_object_destroyed(ScriptbleObject* obj)                                                                     = 0;
    // TODO. 可以不传递 signature 信息，用缓存的 mixin 签名来做，以加速调用
    virtual bool try_invoke_mixin(ScriptbleObject* obj, StringView name, const span<const StackProxy> args, StackProxy result) = 0;
};

// clang-format off
sreflect_struct(guid = "ecb7851e-f6c5-4814-8fba-a35668a2f277")
SKR_CORE_API ScriptbleObject : virtual public skr::IObject
// clang-format on
{
    SKR_GENERATE_BODY()
    virtual ~ScriptbleObject();

    inline EScriptbleObjectOwnership ownership() const { return _ownership_flag; }

    // flag api
    inline void ownership_take_native() { _ownership_flag |= EScriptbleObjectOwnership::Native; }
    inline void ownership_take_script() { _ownership_flag |= EScriptbleObjectOwnership::Script; }
    inline void ownership_release_native() { _ownership_flag &= ~EScriptbleObjectOwnership::Native; }
    inline void ownership_release_script() { _ownership_flag &= ~EScriptbleObjectOwnership::Script; }
    inline bool ownership_has_native() const { return flag_any(_ownership_flag, EScriptbleObjectOwnership::Native); }
    inline bool ownership_has_script() const { return flag_any(_ownership_flag, EScriptbleObjectOwnership::Script); }

    // mixin api
    inline IScriptMixinCore* mixin_core() const { return _mixin_core; }
    inline void              set_mixin_core(IScriptMixinCore* core) { _mixin_core = core; }
    inline void              notify_mixin_core_destroyed()
    {
        if (_mixin_core)
        {
            _mixin_core->on_object_destroyed(this);
            _mixin_core = nullptr;
        }
    }
    template <typename Ret, typename... Args>
    inline auto try_invoke_mixin_method(StringView name, Args... args)
    {
        if constexpr (std::is_same_v<Ret, void>)
        {
            if (!_mixin_core)
            {
                return false;
            }
            return _mixin_core->try_invoke_mixin(
                this,
                name,
                { StackProxyMaker<Args>::Make(args, /*mixin data must exist*/false)... },
                {}
            );
        }
        else
        {
            if (!_mixin_core)
            {
                return Optional<Ret>{};
            }

            Placeholder<Ret> ret;
            bool             invoke_success = _mixin_core->try_invoke_mixin(
                this,
                name,
                { StackProxyMaker<Args>::Make(args, /*mixin data must exist*/false)... },
                { .data = ret.data(), .signature = type_signature_of<Ret>() }
            );

            if (invoke_success)
            {
                return Optional<Ret>{ std::move(*ret.data_typed()) };
            }
            else
            {
                return Optional<Ret>{};
            }
        }
    }

private:
    // user must take ownership after create
    // if None, the object should be deleted by native
    // if Script, the object should be deleted by script
    // otherwise, the object should be deleted by native
    EScriptbleObjectOwnership _ownership_flag = EScriptbleObjectOwnership::None;
    IScriptMixinCore*         _mixin_core     = nullptr;
};
} // namespace skr