#include "SkrGui/framework/key.hpp"

namespace skr
{
namespace gui
{
Key::Key() SKR_NOEXCEPT : _type(EKeyType::None) {}

Key::~Key() SKR_NOEXCEPT
{
    switch (_type)
    {
        case EKeyType::Name:
        case EKeyType::NameStorage:
            _name.~TextStorage();
            break;
        default:
            break;
    }
}

Key::Key(const Key& other) SKR_NOEXCEPT
{
    _type = other._type;
    switch (_type)
    {
        case EKeyType::KeepState:
            _state = other._state;
            break;
        case EKeyType::Int:
        case EKeyType::IntStorage:
            _int = other._int;
            break;
        case EKeyType::Float:
        case EKeyType::FloatStorage:
            _float = other._float;
            break;
        case EKeyType::Name:
        case EKeyType::NameStorage:
            new (&_name) TextStorage(other._name);
            break;
        default:
            break;
    }
}

Key::Key(Key&& other) SKR_NOEXCEPT
{
    _type = other._type;
    switch (_type)
    {
        case EKeyType::KeepState:
            _state = other._state;
            break;
        case EKeyType::Int:
        case EKeyType::IntStorage:
            _int = other._int;
            break;
        case EKeyType::Float:
        case EKeyType::FloatStorage:
            _float = other._float;
            break;
        case EKeyType::Name:
        case EKeyType::NameStorage:
            new (&_name) TextStorage(std::move(other._name));
            break;
        default:
            break;
    }
}

Key& Key::operator=(const Key& other) SKR_NOEXCEPT
{
    _type = other._type;
    switch (_type)
    {
        case EKeyType::KeepState:
            _state = other._state;
            break;
        case EKeyType::Int:
        case EKeyType::IntStorage:
            _int = other._int;
            break;
        case EKeyType::Float:
        case EKeyType::FloatStorage:
            _float = other._float;
            break;
        case EKeyType::Name:
        case EKeyType::NameStorage:
            new (&_name) TextStorage(other._name);
            break;
        default:
            break;
    }
    return *this;
}

Key& Key::operator=(Key&& other) SKR_NOEXCEPT
{
    _type = other._type;
    switch (_type)
    {
        case EKeyType::KeepState:
            _state = other._state;
            break;
        case EKeyType::Int:
        case EKeyType::IntStorage:
            _int = other._int;
            break;
        case EKeyType::Float:
        case EKeyType::FloatStorage:
            _float = other._float;
            break;
        case EKeyType::Name:
        case EKeyType::NameStorage:
            new (&_name) TextStorage(std::move(other._name));
            break;
        default:
            break;
    }
    return *this;
}

bool Key::operator==(const Key& other) const SKR_NOEXCEPT
{
    if (_type != other._type) return false;
    if (_type == EKeyType::None) return true;
    if (_type == EKeyType::Unique)  return false;

    switch (_type)
    {
        case EKeyType::KeepState:
            return _state == other._state;
        case EKeyType::Int:
        case EKeyType::IntStorage:
            return _int == other._int;
        case EKeyType::Float:
        case EKeyType::FloatStorage:
            return _float == other._float;
        case EKeyType::Name:
        case EKeyType::NameStorage:
            return _name.get() == other._name.get();
        default:
            return true;
    }
}

bool Key::operator!=(const Key& other) const SKR_NOEXCEPT
{
    return !(*this == other);
}

} // namespace gui
} // namespace skr