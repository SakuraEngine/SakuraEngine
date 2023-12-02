#pragma once
#include "cdrc/rc_ptr.h"
#include "cdrc/weak_ptr.h"

namespace skr::container
{
    template<class T>
    using shared_ptr = cdrc::rc_ptr_hp<T>;

    template<class T>
    using weak_ptr = cdrc::weak_ptr_hp<T>;

    using cdrc::make_shared;
}