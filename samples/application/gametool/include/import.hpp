#pragma once
#include "platform/configure.h"
#include "GameTool/module.configure.h"
#include "backend_config.h"
#include "asset/config_asset.hpp"

namespace gametool
{
    simport_struct(config_backend_t, "b537f7b1-6d2d-44f6-b313-bcb559d3f490")
    sregister_config_asset();
}