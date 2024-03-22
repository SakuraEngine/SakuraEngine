#pragma once

#define sreflect_managed_component(...) struct sreflect sattr("component" : {"custom" : "::sugoi::managed_component"}) sattr(__VA_ARGS__)
