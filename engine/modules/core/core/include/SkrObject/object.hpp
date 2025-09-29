#pragma once
#include <SkrObject/fwd.hpp>
#include <SkrRTTR/irttr_basic.hpp>
#include <SkrObject/object.generated.h>

// fwd
namespace skr
{
struct ArchiveRead;
struct ArchiveWrite;
} // namespace skr

namespace skr
{
// 用于对象扫描的回调
// 返回值：true 则继续扫描，false 则停止扫描
using ObjectScanCallbackGC = FunctionRef<bool(ObjPtrBasic& ptr)>;
using ObjectScanCallbackPackage = FunctionRef<bool(ObjPtrBasic& ptr)>;
using ObjectScanCallbackPackageSoft = FunctionRef<bool(ObjPtrSoftBasic& ptr)>;

struct [[sattr(
    guid = "40385029-e1b1-4ae7-bc12-6ff894ae1617"
)]] SKR_CORE_API Object : virtual IRTTRBasic
{
    SKR_GENERATE_BODY(Object)
    SKR_DELETE_COPY_MOVE(Object);

    friend struct ObjectManager;

    // ctor & dtor
    Object();
    virtual ~Object();

    //=============================Object GUID=============================
    //== 泛用性设施，一般用于对象的唯一标识，但是其唯一性需要使用这个 ID 的系统自行保证  ==
    //=====================================================================
    GUID object_id() const;
    void set_object_id(const GUID& id);

    //=============================Object Scan=============================
    //== 对 ObjectScan 的直接转发，用于快速的多态扫描                           ==
    //=====================================================================
    virtual void object_scan_gc(ObjectScanCallbackGC callback) const = 0;
    virtual void object_scan_package(ObjectScanCallbackPackage callback) const = 0;
    virtual void object_scan_package_soft(ObjectScanCallbackPackageSoft callback) const = 0;

    //=========================Functional Forward==========================
    //== 一些常用功能的转发，比从反射中查找更快                                  ==
    //=====================================================================
    virtual void object_serde_read(ArchiveRead& reader) = 0;
    virtual void object_serde_write(ArchiveWrite& writer) const = 0;

private:
    GUID _object_id = {};
    uint64_t _object_array_index = 0;
};
} // namespace skr

// object impl
namespace skr
{
// Object GUID
inline GUID Object::object_id() const
{
    return _object_id;
}
inline void Object::set_object_id(const GUID& id)
{
    _object_id = id;
}
} // namespace skr

// concepts
namespace skr::concepts
{
template <typename T>
concept BasedOnObject = std::is_base_of_v<skr::Object, T>;
template <typename From, typename To>
concept ObjConvertible =
    std::convertible_to<From*, To*> &&
    BasedOnObject<From> &&
    BasedOnObject<To>;
} // namespace skr::concepts