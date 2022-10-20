#pragma once
#include "platform/configure.h"
#include "GameTool/module.configure.h"
#include "backend_config.h"
#include "asset/config_asset.hpp"

namespace gametool sreflect
{
    simport_struct(config_backend_t)
    sregister_config_asset();
} 