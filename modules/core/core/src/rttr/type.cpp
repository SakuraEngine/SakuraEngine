#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"

namespace skr::rttr
{
// ctor & dtor
Type::Type()
{
}
Type::~Type()
{
    switch (_type_category)
    {
        case ETypeCategory::Invalid:
            break;
        case ETypeCategory::Primitive:
            _primitive_data.~PrimitiveData();
            break;
        case ETypeCategory::Record:
            _record_data.~RecordData();
            break;
        case ETypeCategory::Enum:
            _enum_data.~EnumData();
            break;
        default:
            SKR_UNREACHABLE_CODE()
            break;
    }
}

// init
void Type::init(ETypeCategory type_category)
{
    if (_type_category == ETypeCategory::Invalid)
    {
        switch (type_category)
        {
            case ETypeCategory::Primitive:
                new (&_primitive_data) PrimitiveData();
                break;
            case ETypeCategory::Record:
                new (&_record_data) RecordData();
                break;
            case ETypeCategory::Enum:
                new (&_enum_data) EnumData();
                break;
            default:
                SKR_UNREACHABLE_CODE()
                break;
        }
        _type_category = type_category;
    }
    else
    {
        SKR_ASSERT(type_category == this->_type_category && "Type category mismatch when init type data");
    }
}

// data getter
const PrimitiveData& Type::primitive_data() const
{
    SKR_ASSERT(_type_category == ETypeCategory::Primitive && "Type category mismatch when get primitive data");
    return _primitive_data;
}
PrimitiveData& Type::primitive_data()
{
    SKR_ASSERT(_type_category == ETypeCategory::Primitive && "Type category mismatch when get primitive data");
    return _primitive_data;
}
const RecordData& Type::record_data() const
{
    SKR_ASSERT(_type_category == ETypeCategory::Record && "Type category mismatch when get record data");
    return _record_data;
}
RecordData& Type::record_data()
{
    SKR_ASSERT(_type_category == ETypeCategory::Record && "Type category mismatch when get record data");
    return _record_data;
}
const EnumData& Type::enum_data() const
{
    SKR_ASSERT(_type_category == ETypeCategory::Enum && "Type category mismatch when get enum data");
    return _enum_data;
}
EnumData& Type::enum_data()
{
    SKR_ASSERT(_type_category == ETypeCategory::Enum && "Type category mismatch when get enum data");
    return _enum_data;
}

// basic getter
ETypeCategory Type::type_category() const
{
    return _type_category;
}
const skr::String& Type::name() const
{
    switch (_type_category)
    {
        case ETypeCategory::Primitive:
            return _primitive_data.name;
        case ETypeCategory::Record:
            return _record_data.name;
        case ETypeCategory::Enum:
            return _enum_data.name;
        default:
            SKR_UNREACHABLE_CODE()
            return _primitive_data.name;
    }
}
GUID Type::type_id() const
{
    switch (_type_category)
    {
        case ETypeCategory::Primitive:
            return _primitive_data.type_id;
        case ETypeCategory::Record:
            return _record_data.type_id;
        case ETypeCategory::Enum:
            return _enum_data.type_id;
        default:
            SKR_UNREACHABLE_CODE()
            return _primitive_data.type_id;
    }
}
size_t Type::size() const
{
    switch (_type_category)
    {
        case ETypeCategory::Primitive:
            return _primitive_data.size;
        case ETypeCategory::Record:
            return _record_data.size;
        case ETypeCategory::Enum:
            return _enum_data.size;
        default:
            SKR_UNREACHABLE_CODE()
            return _primitive_data.size;
    }
}
size_t Type::alignment() const
{
    switch (_type_category)
    {
        case ETypeCategory::Primitive:
            return _primitive_data.alignment;
        case ETypeCategory::Record:
            return _record_data.alignment;
        case ETypeCategory::Enum:
            return _enum_data.alignment;
        default:
            SKR_UNREACHABLE_CODE()
            return _primitive_data.alignment;
    }
}

// build optimize data
void Type::build_optimize_data()
{
    // TODO. build optimize data
}

// caster
void* Type::cast_to(GUID type_id, void* p) const
{
    switch (_type_category)
    {
        case ETypeCategory::Primitive: {
            return (type_id == _primitive_data.type_id) ? p : nullptr;
        }
        case ETypeCategory::Record: {
            if (type_id == _record_data.type_id)
            {
                return p;
            }
            else
            {
                // find base and cast
                for (const auto& base : _record_data.bases_data)
                {
                    if (type_id == base.type_id)
                    {
                        return base.cast_to_base(p);
                    }
                }

                // get base type and continue cast
                for (const auto& base : _record_data.bases_data)
                {
                    auto type = get_type_from_guid(base.type_id);
                    if (type)
                    {
                        auto casted = type->cast_to(type_id, p);
                        if (casted)
                        {
                            return casted;
                        }
                    }
                    else
                    {
                        SKR_LOG_FMT_ERROR(u8"Type \"{}\" not found when doing cast from \"{}\"", type_id, _record_data.type_id);
                    }
                }
            }
        }
        case ETypeCategory::Enum: {
            return (type_id == _enum_data.type_id || type_id == _enum_data.underlying_type_id) ? p : nullptr;
        }
        default:
            SKR_UNREACHABLE_CODE()
            return nullptr;
    }
}

} // namespace skr::rttr