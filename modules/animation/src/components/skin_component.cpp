#include "SkrAnim/components/skin_component.h"
#include "SkrAnim/components/skeleton_component.h"

void skr_initialize_skin_component(skr_skin_component_t *component, skr_skeleton_component_t *skelComponent)
{
    auto skin = component->skin.get_resolved();
    auto skeleton = skelComponent->skeleton.get_resolved();
    SKR_ASSERT(skin && skeleton);
    auto size = skin->bin.joint_remaps.size();
    component->joint_remaps.resize(size);
    for (size_t i = 0; i < size; ++i)
    {   
        for(size_t j = 0; j < skeleton->skeleton.num_joints(); ++j)
        {
            if (std::strcmp(skeleton->skeleton.joint_names()[j], skin->bin.joint_remaps[i].data()) == 0)
            {
                component->joint_remaps[i] = j;
                break;
            }
        }
    }
}