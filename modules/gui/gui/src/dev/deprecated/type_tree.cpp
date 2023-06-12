#include "SkrGui/dev/deprecated/type_tree.hpp"
#include "platform/guid.hpp"

namespace skr
{
namespace gui
{

// type tree
TypeTree::~TypeTree() SKR_NOEXCEPT
{
}

TypeTreeNode::~TypeTreeNode() SKR_NOEXCEPT
{
}

void TypeTreeNode::create_dynamic_type(skr_guid_t id, skr_record_type_id parent_type, const char8_t* name) SKR_NOEXCEPT
{
    type = skr::type::GetTypeRegistry()->register_record(id);
    new (type) skr::type::RecordType{
        size, align, name, id, true, parent_type, {}, {}, {}
    };
}

struct TypeTreeImpl : public TypeTree {
    TypeTreeImpl()
    {
    }
    static TypeTree* Get()
    {
        static TypeTreeImpl type_tree;
        return &type_tree;
    }

    void register_type(skr_guid_t id, TypeTreeNode* node) SKR_NOEXCEPT final
    {
        all_nodes.insert({ id, node });
    }

    skr::flat_hash_map<skr_guid_t, TypeTreeNode*, skr::guid::hash> all_nodes;
};

} // namespace gui
} // namespace skr