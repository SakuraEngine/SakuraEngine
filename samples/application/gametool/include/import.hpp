#pragma once
#include "platform/configure.h"
#include "GameTool/module.configure.h"
#include "GameRuntime/backend_config.h"
#include "SkrToolCore/assets/config_asset.hpp"

namespace gametool sreflect
{
    simport_struct(config_backend_t)
    sregister_config_asset();
} 