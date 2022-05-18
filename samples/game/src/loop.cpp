#include "ecs/callback.hpp"
#include "ecs/dual.h"
#include "gamert.h"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "scene.h"

namespace skg
{
bool GameLoop(GameContext& ctx)
{
    auto world = gamert_get_ecs_world();
    ImGui::Begin(u8"Game");
    dual_filter_t filter;
    std::memset(&filter, 0, sizeof(dual_filter_t));
    auto type_name = dual_id_of<skr_name_t>::get();
    filter.all.data = &type_name;
    filter.all.length = 1;
    dual_meta_filter_t metaFilter;
    std::memset(&metaFilter, 0, sizeof(dual_meta_filter_t));
    auto drawList = [&](dual_chunk_view_t* view) {
        auto names = (skr_name_t*)dualV_get_owned_ro(view, type_name);
        forloop (i, 0, view->count)
            ImGui::Text("%s", names[i].str);
    };
    dualS_query(world, &filter, &metaFilter, DUAL_LAMBDA(drawList));
    ImGui::End();
    return false;
}
} // namespace skg