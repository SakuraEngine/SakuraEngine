local module = {}

function module:init()
    self.value = 0.0
    self.selected = 0
    self.counter = 0
    self.animQuery = skr.create_query(game.GetStorage(), "[in]game::anim_state_t")
    self.outlineQuery = skr.create_query(game.GetStorage(), "[in]?skr_name_comp_t")
end

---@type IMGUI
local imgui = skr.imgui

function module:DrawChildrens(entity)
end

function module:update()
    imgui.Begin("Hello, world!")
    self.counter = self.counter + 1
    local succ, err = pcall(function()
        imgui.Text("This is some useful text." .. tostring(self.counter))
        _, self.value = imgui.DragFloat("float", self.value)
        _, self.selected = imgui.ListBoxCallback("item1", self.selected, function(idx) 
            return true, "item" .. idx
        end, 3)
        skr.iterate_query(self.animQuery, function(view)
            for i = 0, view.length - 1 do
                -- entity, comp1, comp2, comp3, ...
                local ent, state = view(i)
                imgui.Text("state: " .. tostring(state.currtime))
            end
        end)
    end) 
    if not succ then
        skr.print(err)
    end
    imgui.End()
end

return module