#include "UsdCore/core.hpp"
#include "platform/configure.h"

#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
#include <direct.h>
#define curdir _getcwd
#else
#include <unistd.h>
#define curdir getcwd
#endif

#include "pxr/base/plug/registry.h"

namespace skd
{
int USDCoreInitialize()
{
    char buff[FILENAME_MAX];
    curdir(buff, FILENAME_MAX);
    std::string pluginPath = std::string(buff) + std::string("/usd_plugins/");
    auto plugins = pxr::PlugRegistry::GetInstance().RegisterPlugins(pluginPath);
    return (plugins.size() != 0);
}
}