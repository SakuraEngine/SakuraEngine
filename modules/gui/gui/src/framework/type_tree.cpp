#include "SkrGui/framework/diagnostics.hpp"
#include "platform/guid.hpp"

namespace skr {
namespace gui {

// type tree
TypeTree::~TypeTree() SKR_NOEXCEPT
{

}

TypeTreeNode::~TypeTreeNode() SKR_NOEXCEPT
{

}

void TypeTreeNode::create_dynamic_type(skr_guid_t id, skr_dynamic_record_type_id parent_type, const char* name) SKR_NOEXCEPT
{
    if (parent_type)
    {
        skr_guid_t parent_id = {};
        skr_get_type_id((skr_type_t*)parent_type, &parent_id);
        type = skr_create_record_type(&id, size, align, &parent_id);
    }
    else
    {
        type = skr_create_record_type(&id, size, align, nullptr);
    }
    if (name) skr_record_type_set_name(type, name);
}

struct TypeTreeImpl : public TypeTree
{
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
        all_nodes.insert( {id, node} );
    }

    skr::flat_hash_map<skr_guid_t, TypeTreeNode*, skr::guid::hash> all_nodes;
};
TypeTree* Diagnosticable::type_tree = TypeTreeImpl::Get();

} }