#pragma once

#define sreflect_managed_component(...) sreflect_struct("ecs::comp" : {"custom" : "::sugoi::managed_component"}) sattr(__VA_ARGS__)
