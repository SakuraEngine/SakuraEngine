#pragma once
#include "SkrGui/framework/fwd_containers.hpp"
#include "SkrGui/framework/type_tree.hpp"
#include "platform/atomic.h"

namespace skr {
namespace gui {

template<typename T> struct DiagnosticsProperty;
struct Diagnosticable;
struct DiagnosticableTree;
struct DiagnosticableTreeNode;

struct SKR_GUI_API IDiagnosticsProperty
{
    IDiagnosticsProperty(const char* name, const char* description) SKR_NOEXCEPT;
    virtual ~IDiagnosticsProperty() SKR_NOEXCEPT;
    virtual const char* get_name() const SKR_NOEXCEPT;
    virtual const char* get_description() const SKR_NOEXCEPT;
    virtual const char* get_value_as_string() const SKR_NOEXCEPT = 0;

    template<typename T> const T& as() const { return static_cast<const T&>(*this); }
    template<typename T> T& as() { return static_cast<T&>(*this); }

    TextStorage name;
    TextStorage description;
};

template<typename T>
struct SKR_GUI_API DiagnosticsProperty : public IDiagnosticsProperty
{
    DiagnosticsProperty(const char* name, const T& value, const char* description = nullptr)
        : IDiagnosticsProperty(name, description), value(value) {}

    virtual const char* get_value_as_string() const SKR_NOEXCEPT
    {
        return "undefined";
    }

    LiteOptional<T> value;
};

struct SKR_GUI_API BoolDiagnosticProperty : public DiagnosticsProperty<bool>
{
    BoolDiagnosticProperty(const char* name, bool value, const char* description = nullptr)
        : DiagnosticsProperty(name, value, description) {}

    const char* get_value_as_string() const SKR_NOEXCEPT override;
};

struct SKR_GUI_API TextDiagnosticProperty : public DiagnosticsProperty<TextStorage>
{
    TextDiagnosticProperty(const char* name, const char* value, const char* description = nullptr)
        : DiagnosticsProperty(name, value, description) {}
    
    const char* get_value() const SKR_NOEXCEPT;
    const char* get_value_as_string() const SKR_NOEXCEPT override;
};

struct SKR_GUI_API DiagnosticsBuilder
{
    ~DiagnosticsBuilder() SKR_NOEXCEPT;
    IDiagnosticsProperty* add_property(IDiagnosticsProperty* property) SKR_NOEXCEPT;
    template<typename... T>
    void add_properties(T&&... properties) SKR_NOEXCEPT
    {
        (add_property(properties), ...);
    }
    IDiagnosticsProperty* find_property(const char* name) const SKR_NOEXCEPT;
    LiteSpan<IDiagnosticsProperty* const> get_diagnostics_properties() const SKR_NOEXCEPT;

protected:
    VectorStorage<IDiagnosticsProperty*> diagnostic_properties;
};

struct SKR_GUI_API Diagnosticable : public SInterface
{
    SKR_GUI_BASE_TYPE(Diagnosticable, "4e81165e-b13e-41ae-a84f-672429ea969e");
    virtual ~Diagnosticable() SKR_NOEXCEPT;

    IDiagnosticsProperty* find_property(const char* name) const SKR_NOEXCEPT;
    LiteSpan<IDiagnosticsProperty* const> get_diagnostics_properties() const SKR_NOEXCEPT;

    template <typename T>
    bool IsA()
    {
        if (!std::is_base_of_v<Diagnosticable, T>) return false;
        auto T_type = T::getStaticType();
        return T_type->IsBaseOf(*this->getType());
    }
    template <typename T>
    T* Cast()
    {
        if (!IsA<T>()) return nullptr;
        return static_cast<T*>(this);
    }

    virtual uint32_t add_refcount() override;
    virtual uint32_t release() override;

    static struct TypeTree* type_tree;
protected:
    SAtomicU32 rc = 0;
    DiagnosticsBuilder diagnostic_builder;
};

struct SKR_GUI_API DiagnosticableTree : public Diagnosticable
{
    SKR_GUI_TYPE(DiagnosticableTree, Diagnosticable, "64b856c5-2127-46ee-9fb7-80e4d3a65163");
    virtual ~DiagnosticableTree() SKR_NOEXCEPT;

};

struct SKR_GUI_API DiagnosticableTreeNode : public DiagnosticableTree
{
    SKR_GUI_TYPE(DiagnosticableTreeNode, DiagnosticableTree, "26e5515a-7654-4943-a9fe-766db8cedf72");
    virtual ~DiagnosticableTreeNode() SKR_NOEXCEPT;

    virtual LiteSpan<DiagnosticableTreeNode* const> get_diagnostics_children() const = 0;
};

} }