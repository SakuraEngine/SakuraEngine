#include "SkrGui/framework/diagnostics.hpp"
#include "platform/guid.hpp"

namespace skr::gui
{

IDiagnosticsProperty::IDiagnosticsProperty(const char8_t* name, const char8_t* description) SKR_NOEXCEPT
    : name(name),
      description(description)
{
}

IDiagnosticsProperty::~IDiagnosticsProperty() SKR_NOEXCEPT
{
}

const char8_t* IDiagnosticsProperty::get_name() const SKR_NOEXCEPT
{
    return name.u8_str();
}

const char8_t* IDiagnosticsProperty::get_description() const SKR_NOEXCEPT
{
    return description.u8_str();
}

const char8_t* TextDiagnosticProperty::get_value() const SKR_NOEXCEPT
{
    return value.get().u8_str();
}

const char8_t* BoolDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get() ? u8"true" : u8"false";
}

const char8_t* TextDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get().u8_str();
}

IDiagnosticsProperty* DiagnosticsBuilder::add_property(IDiagnosticsProperty* property) SKR_NOEXCEPT
{
    return diagnostic_properties.emplace_back(property);
}

IDiagnosticsProperty* DiagnosticsBuilder::find_property(const char8_t* name) const SKR_NOEXCEPT
{
    for (const auto& property : diagnostic_properties)
    {
        if (strcmp((const char*)property->get_name(), (const char*)name) == 0)
        {
            return property;
        }
    }
    return nullptr;
}

DiagnosticsBuilder::~DiagnosticsBuilder() SKR_NOEXCEPT
{
    for (auto& property : diagnostic_properties)
    {
        SkrDelete(property);
    }
}

Span<IDiagnosticsProperty* const> DiagnosticsBuilder::get_diagnostics_properties() const SKR_NOEXCEPT
{
    return { diagnostic_properties.data(), diagnostic_properties.size() };
}

Diagnosticable::~Diagnosticable() SKR_NOEXCEPT
{
}

IDiagnosticsProperty* Diagnosticable::find_property(const char8_t* name) const SKR_NOEXCEPT
{
    return diagnostic_builder.find_property(name);
}

Span<IDiagnosticsProperty* const> Diagnosticable::get_diagnostics_properties() const SKR_NOEXCEPT
{
    return diagnostic_builder.get_diagnostics_properties();
}

DiagnosticableTree::~DiagnosticableTree() SKR_NOEXCEPT
{
}

DiagnosticableTreeNode::~DiagnosticableTreeNode() SKR_NOEXCEPT
{
}

} // namespace skr::gui