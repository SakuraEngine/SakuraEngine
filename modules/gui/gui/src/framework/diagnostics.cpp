#include "SkrGui/framework/diagnostics.hpp"

namespace skr {
namespace gui {

IDiagnosticsProperty::IDiagnosticsProperty(const char* name, const char* description) SKR_NOEXCEPT
    : name(name), description(description)
{
    
}

IDiagnosticsProperty::~IDiagnosticsProperty() SKR_NOEXCEPT
{
    
}

const char* IDiagnosticsProperty::get_name() const SKR_NOEXCEPT
{
    return name.get().c_str();
}

const char* IDiagnosticsProperty::get_description() const SKR_NOEXCEPT
{
    return description.get().c_str();
}

const char* TextDiagnosticProperty::get_value() const SKR_NOEXCEPT
{
    return value.get().get().c_str();
}

const char* BoolDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get() ? "true" : "false";
}

const char* TextDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get().get().c_str();
}

IDiagnosticsProperty* DiagnosticsBuilder::add_property(IDiagnosticsProperty* property) SKR_NOEXCEPT
{
    return diagnostic_properties.get().emplace_back(property);
}

IDiagnosticsProperty* DiagnosticsBuilder::find_property(const char* name) const SKR_NOEXCEPT
{
    for (const auto& property : diagnostic_properties.get())
    {
        if (strcmp(property->get_name(), name) == 0)
        {
            return property;
        }
    }
    return nullptr;
}

DiagnosticsBuilder::~DiagnosticsBuilder() SKR_NOEXCEPT
{
    for (auto& property : diagnostic_properties.get())
    {
        SkrDelete(property);
    }
}

LiteSpan<IDiagnosticsProperty* const> DiagnosticsBuilder::get_diagnostics_properties() const SKR_NOEXCEPT
{
    const auto& diagnostic_properties = this->diagnostic_properties.get();
    return { diagnostic_properties.data(), diagnostic_properties.size() };
}

Diagnosticable::~Diagnosticable() SKR_NOEXCEPT
{
    
}

IDiagnosticsProperty* Diagnosticable::find_property(const char* name) const SKR_NOEXCEPT
{
    return diagnostic_builder.find_property(name);
}

LiteSpan<IDiagnosticsProperty* const> Diagnosticable::get_diagnostics_properties() const SKR_NOEXCEPT
{
    return diagnostic_builder.get_diagnostics_properties();
}

DiagnosticableTree::~DiagnosticableTree() SKR_NOEXCEPT
{

}

DiagnosticableTreeNode::~DiagnosticableTreeNode() SKR_NOEXCEPT
{

}

} }