#pragma once
#include <SkrObject/fwd.hpp>
#include <SkrObject/object.hpp>

// basic
namespace skr
{
struct ObjPtrBasic
{
    // flag basic
    inline static constexpr size_t kFlagBits = 2;
    inline static constexpr size_t kFlagMask = (1 << kFlagBits) - 1;
    inline static constexpr size_t kPtrMask = ~kFlagMask;

    // flag
    enum class EFlag : size_t
    {
        // 存储的是解析过后的对象指针
        None = 0,
        // 存储的是 Resolver 指针
        IsResolver = 1 << 0,
        // 存储的是 TableID，只存在于序列化过程中，值为右移 kFlagBits 位后的值
        // 最高位为符号位，正数代表包内 id，负数代表包外 id
        IsTableID = 1 << 1,
    };

    // ctor & dtor
    ObjPtrBasic();
    ObjPtrBasic(std::nullptr_t);
    ObjPtrBasic(Object* object);
    ObjPtrBasic(ObjectResolver* resolver);
    ObjPtrBasic(int64_t table_id); // only for serialization
    ~ObjPtrBasic();

    // copy & move
    ObjPtrBasic(const ObjPtrBasic& rhs);
    ObjPtrBasic(ObjPtrBasic&& rhs);

    // assign & move assign
    ObjPtrBasic& operator=(const ObjPtrBasic& rhs);
    ObjPtrBasic& operator=(ObjPtrBasic&& rhs);

    // check flags
    bool is_resolver() const;
    bool is_table_id() const;
    bool is_object() const;
    bool is_resolved() const;

    // part getter
    size_t part_flags() const;
    size_t part_data() const;

    // data getter
    Object* get_object() const;
    ObjectResolver* get_resolver() const;
    int64_t get_table_id() const;

    // is empty
    bool is_empty() const;
    operator bool() const;

    // resolve
    Object* try_resolve();

    // reset
    void reset();
    void reset(Object* object);

    // hash
    static skr_hash _skr_hash(const ObjPtrBasic& obj);
    static skr_hash _skr_hash(Object* ptr);

private:
    size_t _ptr_holder = 0;
};
struct SKR_CORE_API ObjPtrWeakBasic
{
    // ctor & dtor
    ObjPtrWeakBasic();
    ObjPtrWeakBasic(std::nullptr_t);
    ObjPtrWeakBasic(Object* object);
    ~ObjPtrWeakBasic();

    // copy & move
    ObjPtrWeakBasic(const ObjPtrWeakBasic& rhs);
    ObjPtrWeakBasic(ObjPtrWeakBasic&& rhs);

    // assign & move assign
    ObjPtrWeakBasic& operator=(const ObjPtrWeakBasic& rhs);
    ObjPtrWeakBasic& operator=(ObjPtrWeakBasic&& rhs);

    // resolve
    Object* resolve() const;

    // is empty
    bool is_empty() const;
    operator bool() const;

    // reset
    void reset();
    void reset(Object* object);

    // hash
    static skr_hash _skr_hash(const ObjPtrWeakBasic& obj);

private:
    uint64_t _object_array_index = 0;
    uint64_t _object_generation = 0;
};
struct SKR_CORE_API ObjPtrLockBasic
{
    // ctor & dtor
    ObjPtrLockBasic();
    ObjPtrLockBasic(std::nullptr_t);
    ObjPtrLockBasic(Object* object);
    ~ObjPtrLockBasic();

    // copy & move
    ObjPtrLockBasic(const ObjPtrLockBasic& rhs);
    ObjPtrLockBasic(ObjPtrLockBasic&& rhs);

    // assign & move assign
    ObjPtrLockBasic& operator=(const ObjPtrLockBasic& rhs);
    ObjPtrLockBasic& operator=(ObjPtrLockBasic&& rhs);

    // getter
    Object* get() const;

    // is empty
    bool is_empty() const;
    operator bool() const;

    // reset
    void reset();
    void reset(Object* object);

    // hash
    static skr_hash _skr_hash(const ObjPtrLockBasic& obj);
    static skr_hash _skr_hash(Object* ptr);

private:
    Object* _object = nullptr;
};
} // namespace skr

// impl ObjPtrBasic
namespace skr
{
// ctor & dtor
inline ObjPtrBasic::ObjPtrBasic()
    : _ptr_holder(0)
{
}
inline ObjPtrBasic::ObjPtrBasic(std::nullptr_t)
    : _ptr_holder(0)
{
}
inline ObjPtrBasic::ObjPtrBasic(Object* object)
    : _ptr_holder(reinterpret_cast<size_t>(object))
{
}
inline ObjPtrBasic::ObjPtrBasic(ObjectResolver* resolver)
    : _ptr_holder(reinterpret_cast<size_t>(resolver) | size_t(EFlag::IsResolver))
{
}
inline ObjPtrBasic::ObjPtrBasic(int64_t table_id)
{
    constexpr size_t sign_bit_mask = size_t(1) << (std::numeric_limits<size_t>::digits - 1);
    constexpr size_t data_bits_mask = ~sign_bit_mask;
    size_t sign = table_id & sign_bit_mask;
    size_t data = static_cast<size_t>(table_id & data_bits_mask);
    _ptr_holder = (data << kFlagBits) | size_t(EFlag::IsTableID) | sign;
}
inline ObjPtrBasic::~ObjPtrBasic()
{
}

// copy & move
inline ObjPtrBasic::ObjPtrBasic(const ObjPtrBasic& rhs) = default;
inline ObjPtrBasic::ObjPtrBasic(ObjPtrBasic&& rhs) = default;

// assign & move assign
inline ObjPtrBasic& ObjPtrBasic::operator=(const ObjPtrBasic& rhs) = default;
inline ObjPtrBasic& ObjPtrBasic::operator=(ObjPtrBasic&& rhs) = default;

// check flags
inline bool ObjPtrBasic::is_resolver() const
{
    return _ptr_holder & size_t(EFlag::IsResolver);
}
inline bool ObjPtrBasic::is_table_id() const
{
    return _ptr_holder & size_t(EFlag::IsTableID);
}
inline bool ObjPtrBasic::is_object() const
{
    return !(_ptr_holder & size_t(kFlagMask));
}
inline bool ObjPtrBasic::is_resolved() const
{
    return is_object();
}

// part getter
inline size_t ObjPtrBasic::part_flags() const
{
    return _ptr_holder & kFlagMask;
}
inline size_t ObjPtrBasic::part_data() const
{
    return _ptr_holder & kPtrMask;
}

// data getter
inline Object* ObjPtrBasic::get_object() const
{
    SKR_ASSERT(is_object());
    return reinterpret_cast<Object*>(part_data());
}
inline ObjectResolver* ObjPtrBasic::get_resolver() const
{
    SKR_ASSERT(is_resolver());
    return reinterpret_cast<ObjectResolver*>(part_data());
}
inline int64_t ObjPtrBasic::get_table_id() const
{
    SKR_ASSERT(is_table_id());
    constexpr size_t sign_bit_mask = size_t(1) << (std::numeric_limits<size_t>::digits - 1);
    constexpr size_t data_bits_mask = ~sign_bit_mask;
    const size_t sign = _ptr_holder & sign_bit_mask;
    const size_t data = (_ptr_holder & data_bits_mask) >> kFlagBits;
    return sign ? -static_cast<int64_t>(data) : static_cast<int64_t>(data);
}

// is empty
inline bool ObjPtrBasic::is_empty() const
{
    return _ptr_holder == 0;
}
inline ObjPtrBasic::operator bool() const
{
    return !is_empty();
}

// reset
inline void ObjPtrBasic::reset()
{
    _ptr_holder = 0;
}
inline void ObjPtrBasic::reset(Object* object)
{
    _ptr_holder = reinterpret_cast<size_t>(object);
}

// hash
inline skr_hash ObjPtrBasic::_skr_hash(const ObjPtrBasic& obj)
{
    return skr_hash(obj._ptr_holder);
}
inline skr_hash ObjPtrBasic::_skr_hash(Object* ptr)
{
    return (skr_hash) reinterpret_cast<intptr_t>(ptr);
}
} // namespace skr

// impl ObjPtrWeakBasic
namespace skr
{
// ctor & dtor
inline ObjPtrWeakBasic::ObjPtrWeakBasic()
{
}
inline ObjPtrWeakBasic::ObjPtrWeakBasic(std::nullptr_t)
{
}
inline ObjPtrWeakBasic::~ObjPtrWeakBasic()
{
}

// copy & move
inline ObjPtrWeakBasic::ObjPtrWeakBasic(const ObjPtrWeakBasic& rhs) = default;
inline ObjPtrWeakBasic::ObjPtrWeakBasic(ObjPtrWeakBasic&& rhs) = default;

// assign & move assign
inline ObjPtrWeakBasic& ObjPtrWeakBasic::operator=(const ObjPtrWeakBasic& rhs) = default;
inline ObjPtrWeakBasic& ObjPtrWeakBasic::operator=(ObjPtrWeakBasic&& rhs) = default;

// is empty
inline bool ObjPtrWeakBasic::is_empty() const
{
    return _object_generation == 0;
}
inline ObjPtrWeakBasic::operator bool() const
{
    return !is_empty();
}

// reset
inline void ObjPtrWeakBasic::reset()
{
    _object_array_index = 0;
    _object_generation = 0;
}
} // namespace skr

// impl ObjPtrLockBasic
namespace skr
{
// ctor & dtor
inline ObjPtrLockBasic::ObjPtrLockBasic()
{
}
inline ObjPtrLockBasic::ObjPtrLockBasic(std::nullptr_t)
{
}

// is empty
inline bool ObjPtrLockBasic::is_empty() const
{
    return _object == nullptr;
}
inline ObjPtrLockBasic::operator bool() const
{
    return !is_empty();
}

// getter
inline Object* ObjPtrLockBasic::get() const
{
    return _object;
}

// hash
inline skr_hash ObjPtrLockBasic::_skr_hash(const ObjPtrLockBasic& obj)
{
    return skr_hash(reinterpret_cast<intptr_t>(obj._object));
}
inline skr_hash ObjPtrLockBasic::_skr_hash(Object* ptr)
{
    return skr_hash(reinterpret_cast<intptr_t>(ptr));
}

} // namespace skr
