#include "detail/sdfpath_impl.hpp"

skd::SUSDSdfPathImpl::SUSDSdfPathImpl(pxr::SdfPath path)
    : path(path)
{
}

skd::SUSDSdfPathImpl::~SUSDSdfPathImpl()
{
}

bool skd::SUSDSdfPathImpl::IsEmpty() const SKR_NOEXCEPT
{
    return path.IsEmpty();
}

bool skd::SUSDSdfPathImpl::IsAbsoluteRootOrPrimPath() const SKR_NOEXCEPT
{
    return path.IsAbsoluteRootOrPrimPath();
}

skr::string skd::SUSDSdfPathImpl::GetName() const SKR_NOEXCEPT
{
    auto usdName = path.GetName();
    return usdName.c_str();
}

skr::string skd::SUSDSdfPathImpl::GetElementString() const SKR_NOEXCEPT
{
    auto usdElementString = path.GetElementString();
    return usdElementString.c_str();
}

skd::SUSDSdfPathImpl::SharedId skd::SUSDSdfPathImpl::GetAbsoluteRootOrPrimPath() const SKR_NOEXCEPT
{
    auto usdPath = path.GetAbsoluteRootOrPrimPath();
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(usdPath);
}

skd::SUSDSdfPathImpl::SharedId skd::SUSDSdfPathImpl::ReplaceName(const char* newName) SKR_NOEXCEPT
{
    auto usdPath = path.ReplaceName(pxr::TfToken(newName));
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(usdPath);
}

skd::SUSDSdfPathImpl::SharedId skd::SUSDSdfPathImpl::GetParentPath() const SKR_NOEXCEPT
{
    auto usdPath = path.GetParentPath();
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(usdPath);
}

skd::SUSDSdfPathImpl::SharedId skd::SUSDSdfPathImpl::AppendChild(const char* childName) SKR_NOEXCEPT
{
    auto usdPath = path.AppendChild(pxr::TfToken(childName));
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(usdPath);
}

skd::SUSDSdfPathImpl::SharedId skd::SUSDSdfPathImpl::StripAllVariantSelections() SKR_NOEXCEPT
{
    auto usdPath = path.StripAllVariantSelections();
    return skr::SObjectPtr<SUSDSdfPathImpl>::Create(usdPath);
}

skr::string skd::SUSDSdfPathImpl::GetString() const SKR_NOEXCEPT
{
    auto usdString = path.GetString();
    return usdString.c_str();
}