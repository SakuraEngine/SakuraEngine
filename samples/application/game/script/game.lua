local module = {}

function module:init()
    self.animQuery = skr.create_query(game.GetStorage(), "[in]game::anim_state_t")
    -- root entities
    self.outlineQuery = skr.create_query(game.GetStorage(), "[in]?skr_name_comp_t, [in]?skr_child_comp_t, [has]!skr_parent_comp_t")
end

---@type IMGUI
local imgui = skr.imgui

function module:DrawEntity(entity, name, children, view)
    if name == nil then
        name = "entity" .. tostring(entity)
    end
    local flag = imgui.TreeNodeFlags_OpenOnArrow
    if children == nil then
        flag = flag + imgui.TreeNodeFlags_Leaf
    end
    imgui.PushIDInt(entity)
    local opened = imgui.TreeNodeEx(name, flag)
    imgui.PopID()
    if opened and children~=nil then
        local childrenTable = newtable(children.length, 0)
        for i = 0, children.length - 1 do
            table.insert(childrenTable, children(i))
        end
        view:with(childrenTable, function(cview)
            for i = 0, cview.length - 1 do
                local ent, name, children = cview(i);
                self:DrawEntity(ent, name, children, cview)
            end
        end)
    end
    imgui.TreePop()
end

function module:DrawHireachy()
    skr.iterate_query(self.outlineQuery, function(view)
        for i = 0, view.length - 1 do
            local ent, name, children = view(i)
            self:DrawEntity(ent, name, children, view)
        end
    end)
end

function module:DrawAnimState()
    skr.iterate_query(self.animQuery, function(view)
        for i = 0, view.length - 1 do
            -- entity, comp1, comp2, comp3, ...
            local ent, state = view(i)
            imgui.Text("state: " .. tostring(state.currtime))
        end
    end)
end

function module:update()
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