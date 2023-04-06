#include "SkrGui/framework/diagnostics.hpp"
#include "platform/guid.hpp"

namespace skr {
namespace gui {

IDiagnosticsProperty::IDiagnosticsProperty(const char8_t* name, const char8_t* description) SKR_NOEXCEPT
    : name(name), description(description)
{
    
}

IDiagnosticsProperty::~IDiagnosticsProperty() SKR_NOEXCEPT
{
    
}

const char8_t* IDiagnosticsProperty::get_name() const SKR_NOEXCEPT
{
    return name.get().u8_str();
}

const char8_t* IDiagnosticsProperty::get_description() const SKR_NOEXCEPT
{
    return description.get().u8_str();
}

const char8_t* TextDiagnosticProperty::get_value() const SKR_NOEXCEPT
{
    return value.get().get().u8_str();
}

const char8_t* BoolDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get() ? u8"true" : u8"false";
}

const char8_t* TextDiagnosticProperty::get_value_as_string() const SKR_NOEXCEPT
{
    return value.get().get().u8_str();
}

IDiagnosticsProperty* DiagnosticsBuilder::add_property(IDiagnosticsProperty* property) SKR_NOEXCEPT
{
    return diagnostic_properties.get().emplace_back(property);
}

IDiagnosticsProperty* DiagnosticsBuilder::find_property(const char8_t* name) const SKR_NOEXCEPT
{
    for (const auto& property : diagnostic_properties.get())
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

IDiagnosticsProperty* Diagnosticable::find_property(const char8_t* name) const SKR_NOEXCEPT
{
    return diagnostic_builder.find_property(name);
}

LiteSpan<IDiagnosticsProperty* const> Diagnosticable::get_diagnostics_properties() const SKR_NOEXCEPT
{
    return diagnostic_builder.get_diagnostics_properties();
}

uint32_t Diagnosticable::add_refcount()
{
    auto last = skr_atomicu32_add_relaxed(&rc, 1);
    return last + 1;
}

uint32_t Diagnosticable::release()
{
    skr_atomicu32_add_relaxed(&rc, -1);
    return skr_atomicu32_load_acquire(&rc);
}

DiagnosticableTree::~DiagnosticableTree() SKR_NOEXCEPT
{

}

DiagnosticableTreeNode::~DiagnosticableTreeNode() SKR_NOEXCEPT
{

}

SKR_GUI_TYPE_IMPLMENTATION(Diagnosticable);
SKR_GUI_TYPE_IMPLMENTATION(DiagnosticableTree);
SKR_GUI_TYPE_IMPLMENTATION(DiagnosticableTreeNode);

} }