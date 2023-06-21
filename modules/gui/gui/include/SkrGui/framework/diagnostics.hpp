#pragma once
#include "SkrGui/fwd_config.hpp"
#include "platform/atomic.h"
#include "SkrGui/dev/gdi/gdi_types.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

// properties
namespace skr::gui
{
struct SKR_GUI_API IDiagnosticsProperty {
    IDiagnosticsProperty(const char8_t* name, const char8_t* description) SKR_NOEXCEPT;
    virtual ~IDiagnosticsProperty() SKR_NOEXCEPT;
    virtual const char8_t* get_name() const SKR_NOEXCEPT;
    virtual const char8_t* get_description() const SKR_NOEXCEPT;
    virtual const char8_t* get_value_as_string() const SKR_NOEXCEPT = 0;

    template <typename T>
    const T& as() const { return static_cast<const T&>(*this); }
    template <typename T>
    T& as() { return static_cast<T&>(*this); }

    String name;
    String description;
};

template <typename T>
struct SKR_GUI_API DiagnosticsProperty : public IDiagnosticsProperty {
    DiagnosticsProperty(const char8_t* name, const T& value, const char8_t* description = nullptr)
        : IDiagnosticsProperty(name, description)
        , value(value)
    {
    }

    virtual const char8_t* get_value_as_string() const SKR_NOEXCEPT
    {
        return u8"undefined";
    }

    Optional<T> value;
};

struct SKR_GUI_API BoolDiagnosticProperty : public DiagnosticsProperty<bool> {
    BoolDiagnosticProperty(const char8_t* name, bool value, const char8_t* description = nullptr)
        : DiagnosticsProperty(name, value, description)
    {
    }

    const char8_t* get_value_as_string() const SKR_NOEXCEPT override;
};

struct SKR_GUI_API TextDiagnosticProperty : public DiagnosticsProperty<String> {
    TextDiagnosticProperty(const char8_t* name, const char8_t* value, const char8_t* description = nullptr)
        : DiagnosticsProperty(name, value, description)
    {
    }

    const char8_t* get_value() const SKR_NOEXCEPT;
    const char8_t* get_value_as_string() const SKR_NOEXCEPT override;
};
} // namespace skr::gui

// Diagnostics tree
namespace skr::gui
{
struct SKR_GUI_API DiagnosticsBuilder {
    ~DiagnosticsBuilder() SKR_NOEXCEPT;
    IDiagnosticsProperty* add_property(IDiagnosticsProperty* property) SKR_NOEXCEPT;
    template <typename... T>
    void add_properties(T&&... properties) SKR_NOEXCEPT
    {
        (add_property(properties), ...);
    }
    IDiagnosticsProperty*             find_property(const char8_t* name) const SKR_NOEXCEPT;
    Span<IDiagnosticsProperty* const> get_diagnostics_properties() const SKR_NOEXCEPT;

protected:
    Array<IDiagnosticsProperty*> diagnostic_properties;
};

struct SKR_GUI_API Diagnosticable SKR_GUI_OBJECT_BASE {
    SKR_GUI_TYPE_ROOT(Diagnosticable, "4e81165e-b13e-41ae-a84f-672429ea969e");
    SKR_GUI_RAII_MIX_IN()

    virtual ~Diagnosticable() SKR_NOEXCEPT;
    IDiagnosticsProperty*             find_property(const char8_t* name) const SKR_NOEXCEPT;
    Span<IDiagnosticsProperty* const> get_diagnostics_properties() const SKR_NOEXCEPT;

protected:
    DiagnosticsBuilder diagnostic_builder;
};

struct SKR_GUI_API DiagnosticableTree : public Diagnosticable {
    SKR_GUI_TYPE(DiagnosticableTree, "64b856c5-2127-46ee-9fb7-80e4d3a65163", Diagnosticable);
    virtual ~DiagnosticableTree() SKR_NOEXCEPT;
};

struct SKR_GUI_API DiagnosticableTreeNode : public DiagnosticableTree {
    SKR_GUI_TYPE(DiagnosticableTreeNode, "26e5515a-7654-4943-a9fe-766db8cedf72", DiagnosticableTree);
    virtual ~DiagnosticableTreeNode() SKR_NOEXCEPT;

    virtual void visit_diagnostics_children(FunctionRef<void(DiagnosticableTreeNode*)> visitor) SKR_NOEXCEPT = 0;
};

} // namespace skr::gui