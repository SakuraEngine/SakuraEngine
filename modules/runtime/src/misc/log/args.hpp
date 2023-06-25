#pragma once
#include "platform/configure.h"
#include "misc/types.h"
#include "misc/smart_pool.hpp"

namespace skr
{

template <typename = void>
struct Node : public skr::SInterface 
{
    SKR_RC_OBJECT_BODY
    virtual ~Node() SKR_NOEXCEPT = default;
    skr::SObjectPtr<Node<>> next_;
};

template <typename NodeType = Node<void>>
struct ArgsList 
{
private:
    template <typename T>
    struct TypedNode : public NodeType {
        template <typename Arg>
        constexpr TypedNode(const Arg& arg)
            : value(arg)
        {
        }

        T value;
    };
    skr::SObjectPtr<Node<>> head_;

public:
    template <typename T, typename Arg>
    const T& push(const Arg& arg)
    {
        auto new_node = SObjectPtr<TypedNode<T>>::Create(arg);
        auto& value = new_node->value;
        new_node->next = std::move(head_);
        head_ = std::move(new_node);
        return value;
    }
};

} // namespace skr