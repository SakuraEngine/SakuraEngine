
#include "SkrRT/ecs/sugoi.h"
#include "GameRuntime/gamert.h"
#include "SkrImGui/skr_imgui.h"
#include "SkrScene/scene.h"
#include "string.h"

namespace skg
{
bool GameLoop(GameContext& ctx, sugoi_storage_t* world)
{
    ImGui::Begin("Game");
    sugoi_filter_t filter;
    ::memset(&filter, 0, sizeof(sugoi_filter_t));
    auto type_name = sugoi_id_of<skr_name_comp_t>::get();
    filter.all.data = &type_name;
    filter.all.length = 1;
    sugoi_meta_filter_t metaFilter;
    ::memset(&metaFilter, 0, sizeof(sugoi_meta_filter_t));
    auto drawList = [&](sugoi_chunk_view_t* view) {
        auto names = (skr_name_comp_t*)sugoiV_get_owned_ro(view, type_name);
        if (names)
            forloop (i, 0, view->count)
                ImGui::Text("%s", names[i].str);
        else
        {
            auto es = sugoiV_get_entities(view);
            forloop (i, 0, view->count)
                ImGui::Text("%d : %d", es[i] & SUGOI_ENTITY_ID_MASK, (es[i] >> SUGOI_ENTITY_VERSION_OFFSET) & SUGOI_ENTITY_VERSION_MASK);
        };
    };
    sugoiS_all(world, false, false, SUGOI_LAMBDA(drawList));
    ImGui::End();
    return false;
}
} // namespace skg