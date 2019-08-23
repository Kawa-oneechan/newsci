currentScene = {
Initialize = function()
	LoadSimpleScene("397.png", "397-p.png")

	iliraView = View.new("ilira.view.json")
	iliraObject = ViewObj.new(iliraView, 128, 110)
	otherIli = ViewObj.new(iliraView, 156, 179)
	table.insert(cast, iliraObject)
	table.insert(cast, otherIli)

	otherIli.loop = 8
	otherIli.cel = 7
	iliX = -64

	backgroundMusic = Audio.new("pq2.ogg")
	backgroundMusic:Play()
end,

Tick = function()
	iliX = iliX + 4
	if iliX > 340 then iliX = -20 end
	if iliX < 134 then
		otherIli.cel = 7
	elseif iliX < 180 then
		otherIli.cel = 3
	else
		otherIli.cel = 6
	end

	iliraObject:Move(iliX, 148)
	local cel = iliraObject.cel + 1
	-- if cel == iliraObject.numCels then cel = 0 end
	if cel == iliraObject.view:GetNumCels(iliraObject.loop) then cel = 0 end
	iliraObject.cel = cel
	
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
