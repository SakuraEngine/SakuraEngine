#pragma once

namespace skr
{
// object
struct Object;

// object scan
template <typename T>
struct ObjectScan;

// object ptr basic
struct ObjPtrBasic;
struct ObjPtrWeakBasic;
struct ObjPtrLockBasic;
struct ObjPtrSoftBasic;

// object ptr
template <typename T>
struct ObjPtr;
template <typename T>
struct ObjPtrWeak;
template <typename T>
struct ObjPtrLock;
template <typename T>
struct ObjPtrSoft;

// object package
struct ObjectPackage;
struct ObjectResolver;
} // namespace skr