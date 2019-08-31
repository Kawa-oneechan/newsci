currentScene = {
Initialize = function()
	LoadSimpleScene("397.png", "397-p.png")

	iliraView = View.new("ilira.view.json")
	iliraObject = ViewObj.new(iliraView, 128, 110)
	otherIli = ViewObj.new(iliraView, 156, 179)
	table.insert(cast, iliraObject)
	table.insert(cast, otherIli)

	iliraObject.cycler = CycleForward
	otherIli.loop = 8
	otherIli.cel = 7
	otherIli.looper = StandAndLook
	otherIli.target = iliraObject
	iliX = -64
	
	myWindow = {
		visible = true,
		title = "hello?",
		shadow = true,
		box = { 8, 8, 142, 72 },
		controls = {
			{
				type = 1, -- DText
				text = "Testing...",
				color = 0x0000FF
			},
			{
				type = 2, -- DButton
				text = "black",
				top = 16,
				width = 42,
				Click = function()
					events = {}
					myWindow.controls[1].color = 0
				end
			},
			{
				type = 2, -- DButton
				text = "close",
				left = 68,
				top = 16,
				width = 42,
				Click = function()
					events = {}
					myWindow.visible = false
					table.removeByVal(cast, myWindow)
				end
			},
			{
				type = 3, --DText
				text = "lol",
				top = 32,
				width = 128
			}
		},
		
		Draw = function() DrawWindow(myWindow) end, --this would be abstracted away in a class I guess
		Update = function() end -- does nothing lol
	}
	table.insert(cast, myWindow)

	backgroundMusic = Audio.new("pq2.ogg")
	backgroundMusic:Play()
end,

Tick = function()
	iliX = iliX + 4
	if iliX > 340 then iliX = -20 end
	--[[
	if iliX < 134 then
		otherIli.cel = 7
	elseif iliX < 180 then
		otherIli.cel = 3
	else
		otherIli.cel = 6
	end
	]]--
	otherIli:SetHeading(GetAngle(otherIli.x, otherIli.y, iliraObject.x, iliraObject.y))

	iliraObject:Move(iliX, 148)
		
	for k, v in pairs(events) do
		if not v.handled then
			if v.type == 3 then -- mouse click
				otherIli:Move(v.x, v.y)
				v.handled = true
			elseif v.type == 17 then -- key press
				if v.scan == 62 then -- F5: Save
					Serialize("test")
					v.handled = true
				elseif v.scan == 64 then -- F5: Save
					Serializer.Load("test")
					v.handled = true
				--[[ else
					local msg = "> sym " .. v.sym .. "\n> mod " .. v.mod .. "\n> scan " .. v.scan .. "\n> "
					if v.ctrl then msg = msg .. "ctrl " end
					if v.alt then msg = msg .. "alt " end
					if v.shift then msg = msg .. "shift " end
					v.handled = true
					Message(msg, "Key press!")
				]]--
				end
			end
		end
	end
end,

Serialize = function()
	Serializer.SetInteger(iliX)
end,

Deserialize = function()
	currentScene.Initialize()
	iliX = Serializer.GetInteger()
	iliraObject:Move(iliX, 148)
end
}
