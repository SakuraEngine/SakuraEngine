#include "SkrAnim/resources/animation_resource.h"

namespace game sreflect
{  
    sreflect_struct("guid" : "E06E11F7-6F3A-4BFF-93D8-37310EF0FB87")
    sattr("component" : true) 
    animation_state_t
    {
        SKR_RESOURCE_FIELD(skr_anim_resource_t, animation_resource);
        float currtime;
    };
}