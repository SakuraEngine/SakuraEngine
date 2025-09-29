#include "SkrObject/object_ptr_basic.hpp"
#include "./gc_private.hpp"

// ObjPtrWeakBasic
namespace skr
{
// ctor & dtor
ObjPtrWeakBasic::ObjPtrWeakBasic(Object* object)
{
    ObjectManager::get_object_index_and_generation(
        object,
        _object_array_index,
        _object_generation
    );
}

// resolve
Object* ObjPtrWeakBasic::resolve() const
{
    return ObjectManager::try_solve_object(
        _object_array_index,
        _object_generation
    );
}

// reset
void ObjPtrWeakBasic::reset(Object* object)
{
    if (object)
    {
        ObjectManager::get_object_index_and_generation(
            object,
            _object_array_index,
            _object_generation
        );
    }
    else
    {
        reset();
    }
}

// hash
skr_hash ObjPtrWeakBasic::_skr_hash(const ObjPtrWeakBasic& obj)
{
    Hash<uint64_t> hasher;
    return hash_combine(
        hasher(obj._object_array_index),
        hasher(obj._object_generation)
    );
}
} // namespace skr

// ObjPtrLockBasic
namespace skr
{
// ctor & dtor
ObjPtrLockBasic::ObjPtrLockBasic(Object* object)
    : _object(object)
{
    if (_object)
    {
        ObjectManager::root_add(_object);
    }
}
ObjPtrLockBasic::~ObjPtrLockBasic()
{
    if (_object)
    {
        ObjectManager::root_release(_object);
    }
}

// copy & move
ObjPtrLockBasic::ObjPtrLockBasic(const ObjPtrLockBasic& rhs)
    : _object(rhs._object)
{
    if (_object)
    {
        ObjectManager::root_add(_object);
    }
}
ObjPtrLockBasic::ObjPtrLockBasic(ObjPtrLockBasic&& rhs)
    : _object(rhs._object)
{
    rhs._object = nullptr;
}

// assign & move assign
ObjPtrLockBasic& ObjPtrLockBasic::operator=(const ObjPtrLockBasic& rhs)
{
    reset(rhs._object);
    return *this;
}
ObjPtrLockBasic& ObjPtrLockBasic::operator=(ObjPtrLockBasic&& rhs)
{
    reset(rhs._object);
    rhs._object = nullptr;
    return *this;
}

// reset
void ObjPtrLockBasic::reset()
{
    if (_object)
    {
        ObjectManager::root_release(_object);
        _object = nullptr;
    }
}
void ObjPtrLockBasic::reset(Object* object)
{
    if (_object != object)
    {
        // release old
        if (_object)
        {
            ObjectManager::root_release(_object);
        }

        // assign
        _object = object;

        // add new
        if (_object)
        {
            ObjectManager::root_add(_object);
        }
    }
}
} // namespace skr