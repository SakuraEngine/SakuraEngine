local module = {}

---@type IMGUI
local imgui = skr.imgui
function module:update()
    imgui.Begin("Hello, world!")
    imgui.Text("This is some useful text.")
    if self.value == nil then
        self.value = 0.0
    end
    _, self.value = imgui.DragFloat("float", self.value)
    imgui.End()
end

return module