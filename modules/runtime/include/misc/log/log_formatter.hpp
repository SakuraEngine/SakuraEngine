#pragma once
#include "log_base.hpp"
#include "misc/smart_pool.hpp"

namespace skr {
namespace log {

template <typename = void>
struct ArgNode : public skr::SInterface 
{
    SKR_RC_OBJECT_BODY
    virtual ~ArgNode() SKR_NOEXCEPT = default;
    skr::SObjectPtr<ArgNode<>> next_;

    // argument_value_formatter_type...
    virtual skr::string produce(const skr::string& specification) SKR_NOEXCEPT = 0;
};

template <typename NodeType = ArgNode<void>>
struct ArgsList 
{
private:
    template <typename T>
    struct TypedNode : public NodeType {
        template <typename Arg>
        constexpr TypedNode(const Arg& arg) SKR_NOEXCEPT
            : value(arg)
        {
        }

        virtual skr::string produce(const skr::string& specification) SKR_NOEXCEPT
        {
            return skr::text::argument_formatter<T>::produce(value, specification);
        }

        T value;
    };
    skr::SObjectPtr<ArgNode<>> head_;

public:
    template <typename T, typename Arg>
    const T& push(const Arg& arg) SKR_NOEXCEPT
    {
        auto new_node = SObjectPtr<TypedNode<T>>::Create(arg);
        auto& value = new_node->value;
        new_node->next = std::move(head_);
        head_ = std::move(new_node);
        return value;
    }
    template <typename...Args>
    void push(Args&&...args) SKR_NOEXCEPT
    {
        auto _ = { push<Args>(std::forward<Args>(args))... }; (void)_;
        
    }
};

struct LogFormatter
{
    ~LogFormatter() SKR_NOEXCEPT;

    [[nodiscard]] skr::string const& format(
        const skr::string& format,
        const ArgsList<>& args_list
    );
    skr::string formatted_string = u8"";
};

} // namespace log
} // namespace skr