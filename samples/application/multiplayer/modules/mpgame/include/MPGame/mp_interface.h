
#pragma once
#include "MPGame/module.configure.h"
struct MP_GAME_API IApplication
{
    virtual int Initialize() = 0;
    virtual void Run() = 0;
    virtual void Shutdown() = 0;
    virtual ~IApplication() {}
};

MP_GAME_API IApplication* CreateMPApplication();