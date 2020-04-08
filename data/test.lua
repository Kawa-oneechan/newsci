currentScene = {
Initialize = function()
	LoadScene("testroom.pic.json")

	egoView = View.new("ego.view.json")
	egoObject = ViewObj.new(egoView, 128, 180)
	idView = View.new("id.view.json")
	idObject = ViewObj.new(idView, 156, 150)
	table.insert(cast, egoObject)
	table.insert(cast, idObject)

	egoObject.cycler = CycleForward
	idObject.loop = 8
	idObject.cel = 7
	idObject.looper = StandAndLook
	idObject.target = egoObject
	egoX = -64
	egoDir = 1
	
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

	backgroundMusic = Audio.new("beachhouse.ogg")
	backgroundMusic:Play()
end,

Tick = function()
	if egoDir == 1 then
		egoX = egoX + 4
		egoObject.loop = 0
		if egoX > 270 then egoDir = 0 end
	else
		egoX = egoX - 4
		egoObject.loop = 1
		if egoX < -20 then egoDir = 1 end
	end

	idObject:SetHeading(GetAngle(idObject.x, idObject.y, egoObject.x, egoObject.y))

	egoObject:Move(egoX, egoObject.y)
		
	for k, v in pairs(events) do
		if not v.handled then
			if v.type == 3 then -- mouse click
				idObject:Move(v.x, v.y)
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

	DrawPolys()	

end,

Serialize = function()
	Serializer.SetInteger(egoX)
	Serializer.SetInteger(egoDir)
end,

Deserialize = function()
	currentScene.Initialize()
	egoX = Serializer.GetInteger()
	egoDir = Serializer.GetInteger()
	egoObject:Move(egoX, 148)
end
}
