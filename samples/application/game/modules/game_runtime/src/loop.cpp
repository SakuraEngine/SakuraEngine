#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "GameRuntime/gamert.h"
#include "SkrImGui/skr_imgui.h"
#include "SkrScene/scene.h"
#include "string.h"

namespace skg
{
bool GameLoop(GameContext& ctx, dual_storage_t* world)
{
    ImGui::Begin("Game");
    dual_filter_t filter;
    ::memset(&filter, 0, sizeof(dual_filter_t));
    auto type_name = dual_id_of<skr_name_comp_t>::get();
    filter.all.data = &type_name;
    filter.all.length = 1;
    dual_meta_filter_t metaFilter;
    ::memset(&metaFilter, 0, sizeof(dual_meta_filter_t));
    auto drawList = [&](dual_chunk_view_t* view) {
        auto names = (skr_name_comp_t*)dualV_get_owned_ro(view, type_name);
        if (names)
            forloop (i, 0, view->count)
                ImGui::Text("%s", names[i].str);
        else
        {
            auto es = dualV_get_entities(view);
            forloop (i, 0, view->count)
                ImGui::Text("%d : %d", es[i] & DUAL_ENTITY_ID_MASK, (es[i] >> DUAL_ENTITY_VERSION_OFFSET) & DUAL_ENTITY_VERSION_MASK);
        };
    };
    dualS_all(world, false, false, DUAL_LAMBDA(drawList));
    ImGui::End();
    return false;
}
} // namespace skg