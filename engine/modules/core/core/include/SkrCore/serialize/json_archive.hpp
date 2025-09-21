#pragma once
#include <SkrCore/serialize/serialize_traits.hpp>
#include <SkrBase/misc/defer.hpp>
#include <SkrCore/json.hpp>

// fwd & guid
namespace skr
{
struct ArWriteJson;
struct ArReadJson;
struct ArWriteJsonInplace;
} // namespace skr
SKR_TYPE_INFO(skr::ArWriteJson, "af628a16-a3a4-41c3-8863-0ed45b3917c3");
SKR_TYPE_INFO(skr::ArReadJson, "b360c21d-7ad9-4c20-9b8f-71aea9a60413");
SKR_TYPE_INFO(skr::ArWriteJsonInplace, "45c01f7b-e40b-4f4c-af1f-835277a07822");

// json writer
namespace skr
{
// build progress:
// 1. create a object/array scope as root
// 2. one key + one value for object scope
// 3. key cannot be used in array scope
// 4. begin_object/begin_array will traits as a value call and push a new scope
struct SKR_CORE_API ArWriteJson final : public ArchiveWrite
{
    // ctor & dtor
    ArWriteJson();
    ~ArWriteJson();

    //==> ArchiveWrite APIs
    // structured api
    void impl_key(StringView key) override;
    void impl_key_copy(StringView key) override;
    void impl_begin_array() override;
    void impl_end_array() override;
    void impl_begin_object() override;
    void impl_end_object() override;

    // typed data
    void impl_primitive(EArchivePrimitiveType type, const void* data) override;
    void impl_str(StringView str) override;

    // non-structured api
    void impl_bytes(const void* data, uint64_t size) override;
    void impl_bits(const void* data, uint64_t size, uint64_t offset) override;
    //==> ArchiveWrite APIs end

    // factory
    static ArWriteJson Create();
    static ArWriteJson UseDoc(SP<JsonDocumentMut> doc, bool auto_set_root = true);

    // step 1: document setup
    // if doc is nullptr, a new doc will be created
    // if auto_set_root is true, root will be set to doc->root()
    void use_doc(SP<JsonDocumentMut> doc = nullptr, bool auto_set_root = true);
    bool has_doc() const;

    // step 2: get result
    const SP<JsonDocumentMut>& doc() const;
    JsonValueMut* root() const;
    JsonWriteResult write_to(JsonWriteCallback callback, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_string(String& str, EJsonWriteFlags flags = EJsonWriteFlags::None);
    JsonWriteResult write_to_file(const Path& path, EJsonWriteFlags flags = EJsonWriteFlags::None);

    // step 3: reset state for next use
    // reset means clear write state
    // reset document means clear document, and you need to setup document again
    void reset();
    void reset_doc();
    void reset_all();

private:
    // checks
    bool _check_key_state();

    // key operations
    void _set_key(StringView key, bool need_copy);
    JsonValueMut* _get_key_value() const;
    void _reset_key_state();

    // scope operations
    bool _add_to_scope(JsonValueMut* value);

private:
    // document used to create json object
    SP<JsonDocumentMut> _doc = nullptr;

    // solved root object
    JsonValueMut* _root = nullptr;

    // scope used to build json tree
    Vector<JsonValueMut*> _scope_stack = {};

    // string build mode buffer
    Vector<skr_char8> _base64_buffer = {};

    // root behaviour
    bool _auto_set_doc_root = false;

    // key cache
    bool _key_is_literal = false;
    bool _has_key = false;
    StringView _literal_key;
    String _owned_key;
};
} // namespace skr

// json reader
namespace skr
{
struct SKR_CORE_API ArReadJson final : public ArchiveRead
{
    // ctor & dtor
    ArReadJson();
    ~ArReadJson();

    //==> ArchiveRead APIs
    // structured api
    void impl_key(StringView key) override;
    bool impl_has_value() override;
    void impl_begin_array() override;
    uint64_t impl_array_size() override;
    void impl_end_array() override;
    void impl_begin_object() override;
    void impl_end_object() override;

    // typed data
    void impl_primitive(EArchivePrimitiveType type, void* data) override;
    //! view must be valid before any other archive operation, except checkpoint()
    StringView impl_str_view() override;

    // non-structured api
    void impl_bytes(void* data, uint64_t size) override;
    void impl_bits(void* data, uint64_t size, uint64_t offset) override;
    //==> ArchiveRead APIs end

    // factory
    static ArReadJson UseRoot(SP<JsonDocument> doc, JsonValue* root = nullptr);
    static ArReadJson UseRoot(SP<JsonDocumentMut> doc, JsonValueMut* root = nullptr);
    static ArReadJson UseRootUPtr(UPtr<JsonDocument> doc, JsonValue* root = nullptr);
    static ArReadJson UseRootUPtr(UPtr<JsonDocumentMut> doc, JsonValueMut* root = nullptr);
    static ArReadJson ReadFile(const Path& path, EJsonReadFlags flags = EJsonReadFlags::None);
    static ArReadJson ReadBuffer(const void* buffer, uint64_t len, EJsonReadFlags flags = EJsonReadFlags::None);

    // step 1: setup read source
    // doc used to take ownership of document
    // value used to set root value, if null, root will be doc->root()
    void use_root(SP<JsonDocument> doc, JsonValue* root = nullptr);
    void use_root(SP<JsonDocumentMut> doc, JsonValueMut* root = nullptr);
    void use_root_uptr(UPtr<JsonDocument> doc, JsonValue* root = nullptr);
    void use_root_uptr(UPtr<JsonDocumentMut> doc, JsonValueMut* root = nullptr);
    JsonReadResult read_file(const Path& path, EJsonReadFlags flags = EJsonReadFlags::None);
    JsonReadResult read_buffer(const void* buffer, uint64_t len, EJsonReadFlags flags = EJsonReadFlags::None);
    JsonValueCommon root() const;
    bool has_root() const;

    // step 2: reset state for next use
    void reset();
    void reset_root();
    void reset_all();

private:
    struct StackNode
    {
        JsonValueCommon value = nullptr;
        JsonValueCommon pending_value = nullptr;

        union
        {
            struct
            {
                union
                {
                    JsonIterArray iter;
                    JsonIterArrayMut iter_mut;
                };
            } data_arr;
            struct
            {
                JsonIterObject iter;
                JsonIterObjectMut iter_mut;
                uint64_t retry_count = 0;
            } data_obj;
        };
    };

    // checks
    bool _check_has_scope();

    // value operations
    JsonValueCommon _consume_current_value();

    // scope control
    void _push_scop(JsonValueCommon value);
    void _update_key_in_scope(StringView key, StackNode& node);

private:
    // owner ships, used to take ownership of document
    SP<JsonDocument> _doc_sp = nullptr;
    SP<JsonDocumentMut> _doc_mut_sp = nullptr;

    // root value
    JsonValueCommon _root = nullptr;

    // retry for non structured object read
    uint64_t _max_retry_count = 3;

    // scopes
    Vector<StackNode> _scope_stack;
};
} // namespace skr

// json write (inplace)
namespace skr
{
struct SKR_CORE_API ArWriteJsonInplace final : public ArchiveWrite
{
    // ctor & dtor
    ArWriteJsonInplace();
    ~ArWriteJsonInplace();

    //==> ArchiveWrite APIs
    // structured api
    void impl_key(StringView key) override;
    void impl_key_copy(StringView key) override;
    void impl_begin_array() override;
    void impl_end_array() override;
    void impl_begin_object() override;
    void impl_end_object() override;

    // typed data
    void impl_primitive(EArchivePrimitiveType type, const void* data) override;
    void impl_str(StringView str) override;

    // non-structured api
    void impl_bytes(const void* data, uint64_t size) override;
    void impl_bits(const void* data, uint64_t size, uint64_t offset) override;
    //==> ArchiveWrite APIs end

    // write help
    bool write_to_file(const Path& path);

    // reset state for next use
    void reset();
    void reset_string();
    void reset_all();

public:
    // result
    String result;

private:
    enum class EScopeKind
    {
        Array,
        Object,
    };
    struct ScopeData
    {
        EScopeKind kind;
        uint64_t index = 0;

        inline bool is_array() const { return kind == EScopeKind::Array; }
        inline bool is_object() const { return kind == EScopeKind::Object; }
    };

    // checks
    bool _check_key_state();

    // key operations
    void _set_key(StringView key, bool need_copy);
    void _reset_key_state();
    void _prepare_for_write_value();

    // scope operations
    void _push_scope(EScopeKind kind);
    void _pop_scope();

private:
    Vector<ScopeData> _scope_stack;

    // key cache
    bool _key_is_literal = false;
    bool _has_key = false;
    StringView _literal_key = {};
    String _owned_key = {};
};
} // namespace skr