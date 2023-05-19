local imgui = require "imgui"
local module = {}
function module:init()
    self.animQuery = skr.create_query(game.GetStorage(), "[in]game::anim_state_t")
    -- root entities
    self.outlineQuery = skr.create_query(game.GetStorage(), "[in]?skr_name_comp_t, [in]?skr_child_comp_t, [has]!skr_parent_comp_t")
end


function module:DrawEntity(entity : number, name : string, children : CompArr<Entity>, view : View<string, CompArr<Entity>>)
    if name == nil then
        name = "entity" .. tostring(entity)
    end
    local flag = imgui.TreeNodeFlags_OpenOnArrow
    if children == nil then
        flag = flag + imgui.TreeNodeFlags_Leaf + imgui.TreeNodeFlags_NoTreePushOnOpen
    end
    imgui.PushIDInt(entity)
    local opened = imgui.TreeNodeEx(name, flag)
    imgui.PopID()
    
    if opened and children~=nil then
        local childrenTable : {number} = newtable(children.length, 0)
        --skr.print("entity: " .. tostring(entity) .. " name: " .. tostring(name) .. " children: " .. tostring(children) .. " children.length " .. tostring(children.length));
        for i = 0, children.length - 1 do
            table.insert(childrenTable, children:get(i))
        end
        view:with(childrenTable, function(cview)
            for i = 0, cview.length - 1 do
                local centity, cname, cchildren = cview:unpack(i);
                --skr.print("centity: " .. tostring(centity))
                self:DrawEntity(centity, cname, cchildren, cview)
            end
        end)
        imgui.TreePop()
    end
end

function module:DrawHireachy()
    skr.iterate_query(self.outlineQuery, function(view : View<string, CompArr<Entity>>)
        for i = 0, view.length - 1 do
            local ent, name, children = view:unpack(i)
            self:DrawEntity(ent, name, children, view)
        end
    end)
end

function module:DrawAnimState()
    skr.iterate_query(self.animQuery, function(view : View<any>)
        for i = 0, view.length - 1 do
            -- entity, comp1, comp2, comp3, ...
            local ent, state = view:unpack(i)
            imgui.Text("state: " .. tostring(state.currtime))
        end
    end)
end

function module:update()
    imgui.ShowDemoWindow(true)
    imgui.Begin("Hello, world!")
    local succ, err = pcall(function()
        self:DrawHireachy();
    end) 
    if not succ then
        skr.print(err)
    end
    imgui.End()
end

return module