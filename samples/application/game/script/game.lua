local module = {}

function module:init()
    self.value = 0.0
    self.selected = 0
    self.counter = 0
end

---@type IMGUI
local imgui = skr.imgui
function module:update()
    imgui.Begin("Hello, world!")
    self.counter = self.counter + 1
    local succ, err = pcall(function()
        imgui.Text("This is some useful text." .. tostring(self.counter))
        _, self.value = imgui.DragFloat("float", self.value)
        _, self.selected = imgui.ListBoxCallback("item1", self.selected, function(idx) 
            return true, "item" .. idx
        end, 3)
    end)
    if not succ then
        skr.print(err)
    end
    imgui.End()
end

return module