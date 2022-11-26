#pragma once
#include "SkrAnim/resources/skeleton_resource.h"
#ifndef __meta__
    #include "SkrAnim/components/skeleton_component.generated.h"
#endif

sreflect_struct("guid"
                : "05622CB2-9D73-402B-B6C5-8075E13D5063")
sattr("component" : true)
skr_skeleton_component_t
{
    SKR_RESOURCE_FIELD(skr_skeleton_resource_t, skeleton);
};