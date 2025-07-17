#pragma once
#include "SkrRTTR/script/stack_proxy.hpp"
#include <SkrRTTR/type.hpp>
#include <SkrCore/log.hpp>

// script export concept
//   - primitive: primitive type, always include [number, boolean, string, real]
//   - mapping: map type structure into a script object, methods is not supported
//   - value: impl full record feature [ctor, method, static_method, field, static_field, property, static_property]
//     - life contorl: by default, we copy data into script to avoid script holding native pointer,
//                     in some optimize case, like field/static_field/param(call script), we will hold native pointer
//                     to avoid copy, and we will invalidate bind data when handle was destroyed, if script used it,
//                     we will throw exception
//     - optimize:
//       - field/static_field: pass by ref
//       - param(call script): if inout, pass by ref and never return, if pure out, return new value
//   - object: impl full record feature [ctor, method, static_method, field, static_field, property, static_property]
//     - life contorl: we hold ScriptbleObject pointer in script, when ScriptbleObject destroyed, we will listen this
//                     event and invalidate bind data, if script used it, we will throw exception
//
// export behaviour:
//   parameter(call native):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |   (copy)T  | (copy)T |  (copy)T  | (copy)T |    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |     T&     |   (copy)T  | (copy)T |  (copy)T  |(by ref)T|    -     | Note. by default, will have inout flag
//   |  const T&  |   (copy)T  | (copy)T |  (copy)T  |(by ref)T|    -     |
//
//   parameter(call script):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |   (copy)T  | (copy)T |  (copy)T  |(by ref)T|    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |     T&     |   (copy)T  | (copy)T |  (copy)T  |(by ref)T|    -     |
//   |  const T&  |   (copy)T  | (copy)T |  (copy)T  |(by ref)T|    -     |
//
//   return(call native):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |   (copy)T  | (copy)T |  (copy)T  | (copy)T |    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |     T&     |   (copy)T  | (copy)T |  (copy)T  | (copy)T |    -     |
//   |  const T&  |   (copy)T  | (copy)T |  (copy)T  | (copy)T |    -     |
//
//   return(call script):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |   (copy)T  | (copy)T |  (copy)T  | (copy)T |    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |     T&     |      -     |    -    |     -     |    -    |    -     |
//   |  const T&  |      -     |    -    |     -     |    -    |    -     |
//
//   field(native -> script):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |    (copy)T | (copy)T |  (copy)T  |(by ref)T|    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T&  |      -     |    -    |     -     |    -    |    -     |
//   |     T&     |      -     |    -    |     -     |    -    |    -     |
//
//   field(script -> native):
//   |            |  primitive |  enum   |  mapping  |  value  |  object  |
//   |     T      |    (copy)T | (copy)T |  (copy)T  | (copy)T |    -     |
//   |     T*     |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T*  |      -     |    -    |     -     |    -    |(by ref)T?|
//   |  const T&  |      -     |    -    |     -     |    -    |    -     |
//   |     T&     |      -     |    -    |     -     |    -    |    -     |
//
// suggestions for type erase(data memory -> void*):
//   Primitive: void* means T*
//   Enum: void* means underlying type*
//   Mapping: void* means T*
//   Value: void* means T*
//!  Object: void* means T**, because we only pass object by pointer, and when we talk about object, we always trait it as a pointer
//
// about enum export
//   many script has no enum concept, so we need to export enum as an object
//   we recommend export enum as number type, witch is more friendly to native
//
// about inherit:
//   - cpp inherit won't mapping to script
//   - base methods/fields will flatten into final export type
//   - script inherit operator (like instanceof in js) won't work
// why not export inherit?
//   - overload cross multi bases will be easy to handle
//   - script neednot to care about inherit in most case
//   - support script inherit will broken cpp inherit model
//   - support script inherit will induct more complex concept (interface mixin), witch will make code unstable
//
// about parameter In/Out flag:
//   - In: ignore, and always be default
//   - Out: will removed in parameter list, and added in return list
//   - InOut: will appear in both parameter and return list
//
// script support primitive types(must support):
//   void: in return
//   integer: int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t
//   floating: float, double
//   boolean: bool
//   string: skr::String, skr::StringView
//
// about StringView:
//   - cannot pass by ref
//   - cannot used as field
//   - cannot return by script(ScriptMixin)
//   - will copy when return by native
//
// about generic container element visit:
//   - at()/find() means get copy in all cases
//   - visit([func]) means access by ref in value case, it will be invalid out of [func] body
//   - each by iterator means access by ref in value case, it will be invalid when iterator reach end
//
// script support generic types(can be toggled off):
//   skr::Array: accept script array
//   skr::Span: accept script array
//   skr::Vector: accept script array
//   skr::Map: accept script map
//   skr::Optional: make primitive/mapping type nullable without use pointer
//   skr::NotNull: make object type non-nullable without use reference
//   skr::FunctionRef: support callback from script for each() methods

namespace skr
{
// StringView export helper
struct StringViewStackProxy {
    StringView view;
    String     holder;

    static void* custom_mapping(void* obj)
    {
        return &reinterpret_cast<StringViewStackProxy*>(obj)->view;
    }
};

// root binder, means script visible types
//   - primitive
//   - enum
//   - value
//   - mapping
//   - object
struct ScriptBinderPrimitive;
struct ScriptBinderMapping;
struct ScriptBinderObject;
struct ScriptBinderEnum;
struct ScriptBinderValue;
struct ScriptBinderRoot {
    enum class EKind
    {
        None,
        Primitive,
        Value,
        Mapping,
        Object,
        Enum,
    };

    // ctor
    inline ScriptBinderRoot() = default;
    inline ScriptBinderRoot(ScriptBinderPrimitive* primitive)
        : _kind(EKind::Primitive)
        , _binder(primitive)
    {
    }
    inline ScriptBinderRoot(ScriptBinderMapping* mapping)
        : _kind(EKind::Mapping)
        , _binder(mapping)
    {
    }
    inline ScriptBinderRoot(ScriptBinderObject* object)
        : _kind(EKind::Object)
        , _binder(object)
    {
    }
    inline ScriptBinderRoot(ScriptBinderEnum* enum_)
        : _kind(EKind::Enum)
        , _binder(enum_)
    {
    }
    inline ScriptBinderRoot(ScriptBinderValue* value)
        : _kind(EKind::Value)
        , _binder(value)
    {
    }

    // copy & move
    inline ScriptBinderRoot(const ScriptBinderRoot& other) = default;
    inline ScriptBinderRoot(ScriptBinderRoot&& other)      = default;

    // assign & move assign
    inline ScriptBinderRoot& operator=(const ScriptBinderRoot& other) = default;
    inline ScriptBinderRoot& operator=(ScriptBinderRoot&& other)      = default;

    // compare
    inline bool operator==(const ScriptBinderRoot& other) const
    {
        return _kind == other._kind && _binder == other._binder;
    }
    inline bool operator!=(const ScriptBinderRoot& other) const
    {
        return !operator==(other);
    }

    // hash
    inline static size_t _skr_hash(const ScriptBinderRoot& self)
    {
        return hash_combine(
            Hash<EKind>()(self._kind),
            Hash<void*>()(self._binder)
        );
    }

    // kind getter
    inline EKind kind() const { return _kind; }
    inline bool  is_empty() const { return _kind == EKind::None; }
    inline bool  is_primitive() const { return _kind == EKind::Primitive; }
    inline bool  is_mapping() const { return _kind == EKind::Mapping; }
    inline bool  is_object() const { return _kind == EKind::Object; }
    inline bool  is_enum() const { return _kind == EKind::Enum; }
    inline bool  is_value() const { return _kind == EKind::Value; }

    // binder getter
    inline ScriptBinderPrimitive* primitive() const
    {
        SKR_ASSERT(_kind == EKind::Primitive);
        return static_cast<ScriptBinderPrimitive*>(_binder);
    }
    inline ScriptBinderMapping* mapping() const
    {
        SKR_ASSERT(_kind == EKind::Mapping);
        return static_cast<ScriptBinderMapping*>(_binder);
    }
    inline ScriptBinderObject* object() const
    {
        SKR_ASSERT(_kind == EKind::Object);
        return static_cast<ScriptBinderObject*>(_binder);
    }
    inline ScriptBinderEnum* enum_() const
    {
        SKR_ASSERT(_kind == EKind::Enum);
        return static_cast<ScriptBinderEnum*>(_binder);
    }
    inline ScriptBinderValue* value() const
    {
        SKR_ASSERT(_kind == EKind::Value);
        return static_cast<ScriptBinderValue*>(_binder);
    }

    // ops
    inline void reset()
    {
        _kind   = EKind::None;
        _binder = nullptr;
    }

private:
    EKind _kind   = EKind::None;
    void* _binder = nullptr;
};

// nested binder, means elements of export types
//   - ctor
//   - field
//   - static field
//   - method
//   - static method
//   - property
//   - static property
//   - param
//   - return
struct ScriptBinderCtor;
struct ScriptBinderField;
struct ScriptBinderStaticField;
struct ScriptBinderMethod;
struct ScriptBinderStaticMethod;
struct ScriptBinderProperty;
struct ScriptBinderStaticProperty;
struct ScriptBinderParam;
struct ScriptBinderReturn;
// TODO. ScriptBinderNested，用于在实际类型转换时传递全量的信息

//==================nested binders==================
// nested binder, field & static field
struct ScriptBinderField {
    const RTTRType*      owner  = nullptr;
    ScriptBinderRoot     binder = {};
    const RTTRFieldData* data   = nullptr;
    bool                 failed = false;
};
struct ScriptBinderStaticField {
    const RTTRType*            owner  = nullptr;
    ScriptBinderRoot           binder = {};
    const RTTRStaticFieldData* data   = nullptr;
    bool                       failed = false;
};

// nested binder, method & static method
struct ScriptBinderParam {
    ScriptBinderRoot     binder           = {};
    const RTTRParamData* data             = nullptr;
    uint32_t             index            = 0;
    bool                 pass_by_ref      = false;
    bool                 appare_in_return = false;
    ERTTRParamFlag       inout_flag       = ERTTRParamFlag::None;
};
struct ScriptBinderReturn {
    ScriptBinderRoot binder      = {};
    bool             pass_by_ref = false;
    bool             is_void     = false;
};
struct ScriptBinderMethod {
    const RTTRType*           owner           = nullptr;
    const RTTRMethodData*     data            = nullptr;
    const RTTRMethodData*     mixin_impl_data = nullptr;
    ScriptBinderReturn        return_binder   = {};
    Vector<ScriptBinderParam> params_binder   = {};
    uint32_t                  params_count    = 0;
    uint32_t                  return_count    = 0;
    bool                      is_mixin        = false;
    bool                      failed          = false;
};
struct ScriptBinderStaticMethod {
    const RTTRType*             owner         = nullptr;
    const RTTRStaticMethodData* data          = nullptr;
    ScriptBinderReturn          return_binder = {};
    Vector<ScriptBinderParam>   params_binder = {};
    uint32_t                    params_count  = 0;
    uint32_t                    return_count  = 0;
    bool                        failed        = false;
};

// nested binder, property
struct ScriptBinderProperty {
    ScriptBinderRoot   binder = {};
    ScriptBinderMethod setter = {};
    ScriptBinderMethod getter = {};
    bool               failed = false;
};
struct ScriptBinderStaticProperty {
    ScriptBinderRoot         binder = {};
    ScriptBinderStaticMethod setter = {};
    ScriptBinderStaticMethod getter = {};
    bool                     failed = false;
};

// nested binder, constructor
struct ScriptBinderCtor {
    const RTTRCtorData*       data          = nullptr;
    Vector<ScriptBinderParam> params_binder = {};
    bool                      failed        = false;
};
//==================nested binders==================

//==================root binders==================
struct ScriptBinderPrimitive {
    uint32_t    size      = 0;
    uint32_t    alignment = 0;
    GUID        type_id   = {};
    DtorInvoker dtor      = nullptr;

    bool failed = false;
};
struct ScriptBinderMapping {
    const RTTRType*                type   = nullptr;
    Map<String, ScriptBinderField> fields = {};

    bool failed = false;
};
struct ScriptBinderRecordBase {
    const RTTRType* type = nullptr;

    bool             is_script_newable = false;
    ScriptBinderCtor ctor              = {};

    Map<String, ScriptBinderField>          fields            = {};
    Map<String, ScriptBinderStaticField>    static_fields     = {};
    Map<String, ScriptBinderMethod>         methods           = {};
    Map<String, ScriptBinderStaticMethod>   static_methods    = {};
    Map<String, ScriptBinderProperty>       properties        = {};
    Map<String, ScriptBinderStaticProperty> static_properties = {};

    bool failed = false;
};
struct ScriptBinderObject : ScriptBinderRecordBase {
};
struct ScriptBinderValue : ScriptBinderRecordBase {
};
struct ScriptBinderEnum {
    const RTTRType* type = nullptr;

    ScriptBinderPrimitive* underlying_binder = nullptr;

    Map<String, const RTTREnumItemData*> items = {};

    bool is_signed = false;
};
//==================root binders==================

// function binder, used for call script function
struct ScriptBinderCallScript {
    ScriptBinderReturn        return_binder = {};
    Vector<ScriptBinderParam> params_binder = {};
    uint32_t                  params_count  = 0;
    uint32_t                  return_count  = 1;

    bool failed = false;
};

// export case
enum class EScriptExportCase : uint8_t
{
    Basic,       // global variable or create from script
    Field,       // field value
    StaticField, // static field value
    Param,       // param value
    Return,      // return value
};

// TODO. Generic type support
//   ScriptBinderGeneric，存储一个 GenericId，可以通过 GenericId cast 到具体子类
//   扩展 Generic 类型采用向 ScriptBinderManager 中注册 Plugin 的形式进行，或者所有的 Generic 类型都通过 Plugin 形式插入
//   Generic 类型采用特殊的方式注入函数，令其模拟 Value Export 的行为
//   与普通 Value 类型的区别是，除了 void* 的首位指针，还会传入 ScriptBinderGeneric* 便于 Generic 行为的模拟
//   Optional/FunctionRef/Delegate/Event 等特殊 Generic 类型的导出可以考虑不进行行为模拟，而是在 Script 端主动处理
struct SKR_CORE_API ScriptBinderManager {
    // ctor & dtor
    ScriptBinderManager();
    ~ScriptBinderManager();

    // get binder
    ScriptBinderRoot       get_or_build(GUID type_id); // TODO. 直接传入 TypeSignatureView，若带有 Modifier 则抛出 warning，并提醒在外部清除 Modifier 以进行抑制
    ScriptBinderCallScript build_call_script_binder(span<const StackProxy> params, StackProxy ret);

    // each
    void each_cached_root_binder(FunctionRef<void(const GUID&, const ScriptBinderRoot&)> func);

    // clear
    void clear();

private:
    // make root binder
    ScriptBinderPrimitive* _make_primitive(GUID type_id);
    ScriptBinderMapping*   _make_mapping(const RTTRType* type);
    ScriptBinderObject*    _make_object(const RTTRType* type);
    ScriptBinderValue*     _make_value(const RTTRType* type);
    ScriptBinderEnum*      _make_enum(const RTTRType* type);
    void                   _fill_record_info(ScriptBinderRecordBase& out, const RTTRType* type);

    // make nested binder
    void _make_ctor(ScriptBinderCtor& out, const RTTRCtorData* ctor, const RTTRType* owner);
    void _make_method(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner);
    void _make_static_method(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* method, const RTTRType* owner);
    void _make_mixin_method(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRMethodData* impl_method, const RTTRType* owner);
    void _make_prop_getter(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner);
    void _make_prop_setter(ScriptBinderMethod& out, const RTTRMethodData* method, const RTTRType* owner);
    void _make_static_prop_getter(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* method, const RTTRType* owner);
    void _make_static_prop_setter(ScriptBinderStaticMethod& out, const RTTRStaticMethodData* method, const RTTRType* owner);
    void _make_field(ScriptBinderField& out, const RTTRFieldData* field, const RTTRType* owner);
    void _make_static_field(ScriptBinderStaticField& out, const RTTRStaticFieldData* field, const RTTRType* owner);
    void _make_param(ScriptBinderParam& out, const RTTRParamData* param);
    void _make_return(ScriptBinderReturn& out, TypeSignatureView signature);
    void _make_param_call_script(ScriptBinderParam& out, const RTTRParamData* param);
    void _make_return_call_script(ScriptBinderReturn& out, TypeSignatureView signature);

    // checker
    void _try_export_field(TypeSignatureView signature, ScriptBinderRoot& out_binder);

private:
    // cache
    Map<GUID, ScriptBinderRoot> _cached_root_binders;

    // logger
    struct Logger {
        struct StackScope {
            StackScope(Logger* logger, String name)
                : _logger(logger)
            {
                _logger->_stack.push_back({ name, false });
            }
            ~StackScope()
            {
                bool err = _logger->any_error();
                _logger->_stack.pop_back();
                if (!_logger->_stack.is_empty())
                {
                    _logger->_stack.back().any_error |= err;
                }
            }

        private:
            Logger* _logger;
        };

        template <typename... Args>
        inline StackScope stack(StringView fmt, Args&&... args)
        {
            return StackScope{
                this,
                format(fmt, std::forward<Args>(args)...)
            };
        }

        template <typename... Args>
        inline void error(StringView fmt, Args&&... args)
        {
            // record error
            if (!_stack.is_empty())
            {
                _stack.back().any_error = true;
            }

            // combine error str
            String error;
            format_to(error, fmt, std::forward<Args>(args)...);
            error.append(u8"\n");
            for (const auto& stack : _stack.range_inv())
            {
                format_to(error, u8"    at: {}\n", stack.name);
            }
            error.first(error.length_buffer() - 1);

            // log error
            SKR_LOG_FMT_ERROR(u8"{}", error.c_str());
        }

        inline bool any_error() const
        {
            if (_stack.is_empty()) { return false; }
            return _stack.back().any_error;
        }

    private:
        struct ErrorStack {
            String name      = {};
            bool   any_error = false;
        };
        Vector<ErrorStack> _stack;
    };
    Logger _logger;
};
} // namespace skr
