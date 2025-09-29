#pragma once

// final using alias
#ifdef __meta__
    #define sfinal_alias clang::annotate("__final_name__")
#else
    #define sfinal_alias
#endif

// basic meta
#ifdef __meta__
    #define sattr(...) clang::annotate("__reflect__"), clang::annotate(SKR_MAKE_STRING(__VA_ARGS__))
#else
    #define sattr(...)
#endif

// generate body
#ifdef __meta__
    #define SKR_GENERATE_BODY(__MARK) void _zz_skr_generate_body_flag_##__MARK();
#else
    #define _SKR_GENERATE_BODY_NAME(__FILE, __MARK) _SKR_GENERATE_BODY_NAME_IMPL(__FILE, _, __MARK)
    #define _SKR_GENERATE_BODY_NAME_IMPL(__FILE, __SEP1, __MARK) _zz_SKR_GENERATE_BODY_##__FILE##__SEP1##__MARK
    #define SKR_GENERATE_BODY(__MARK) _SKR_GENERATE_BODY_NAME(SKR_FILE_ID, __MARK)
#endif

// filter marco
#ifdef __meta__
    #define SKR_IFNOT_META(...)
#else
    #define SKR_IFNOT_META(...) __VA_ARGS__
#endif
#ifdef __meta__
    #define SKR_IF_META(...) __VA_ARGS__
#else
    #define SKR_IF_META(...)
#endif

// param flag
#define sparam_in sattr(rttr.flags += "In")
#define sparam_out sattr(rttr.flags += "Out")
#define sparam_inout sattr(rttr.flags += ["In", "Out"])

// script api
#define sscript_visible sattr(rttr.flags += "ScriptVisible")
#define sscript_newable sattr(rttr.flags += "ScriptNewable")
#define sscript_mapping sattr(rttr.flags += "ScriptMapping")
#define sscript_mixin sattr(rttr.flags += "ScriptMixin" rttr.script_mixin = true)
#define sscript_getter(__NAME) sattr(rttr.attrs += `ScriptGetter(SKR_UTF8(#__NAME))`)
#define sscript_setter(__NAME) sattr(rttr.attrs += `ScriptSetter(SKR_UTF8(#__NAME))`)

// rttr helper
#define srttr_flag(__FLAG) sattr(rttr.flags += #__FLAG)
#define srttr_attr(...) sattr(rttr.attrs += `__VA_ARGS__`)