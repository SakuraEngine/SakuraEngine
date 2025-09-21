#pragma once
#include <SkrV8/v8_fwd.hpp>
#include <SkrRTTR/script/scriptble_object.hpp>

// v8 includes
#include <v8-persistent-handle.h>
#include <v8-template.h>
#include "SkrV8/v8_bind_proxy.generated.h"

namespace skr
{
struct [[sattr(guid = "7c34fd55-b858-4a79-bd03-69164d4e6f82")]]
V8BindProxy : skr::IObject
{
    SKR_GENERATE_BODY(V8BindProxy);
    SKR_DELETE_COPY_MOVE(V8BindProxy);

    V8BindProxy() = default;
    virtual ~V8BindProxy();

    // validate
    inline bool is_valid() const { return _is_valid; }
    virtual void invalidate();

    // owner ship
    inline V8BindProxy* parent() const { return _parent; }
    inline void set_parent(V8BindProxy* parent)
    {
        if (_parent) { _parent->_children.remove(address); }
        _parent = parent;
        if (_parent) { _parent->_children.add(address, this); }
    }

public:
    void* address = nullptr;

protected:
    // validate
    bool _is_valid = true;

    // owner ship
    V8BindProxy* _parent = nullptr;
    Map<void*, V8BindProxy*> _children = {}; // fields
};

struct [[sattr(guid = "e904c60c-1207-49e2-9476-6f7f97db7675")]]
V8BPRecord : V8BindProxy
{
    SKR_GENERATE_BODY(V8BPRecord);
    // info
    const RTTRType* rttr_type = nullptr;

    // v8 info
    v8::Persistent<::v8::Object> v8_object = {};
    V8Isolate* isolate = nullptr;
};
struct [[sattr(guid = "7c46f696-8ee0-4922-a255-78d25b38e305")]]
V8BPValue : V8BPRecord
{
    SKR_GENERATE_BODY(V8BPValue);
    using Super = V8BPRecord;

    V8BPValue() = default;

    void invalidate() override;

    bool need_delete = false;
    const V8BTValue* bind_tp = nullptr;
};
struct [[sattr(guid = "185076da-2a1d-417f-b57e-0d27914a91d7")]]
V8BPObject : V8BPRecord
{
    SKR_GENERATE_BODY(V8BPObject);
    using Super = V8BPRecord;

    V8BPObject() = default;

    void invalidate() override;

    const V8BTObject* bind_tp = nullptr;
    ScriptbleObject* object = nullptr;
};

} // namespace skr