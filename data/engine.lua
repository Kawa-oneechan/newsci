starting = true
restoring = false
quit = false
delay = 50
cast = {}
events = {}
backgroundMusic = nil
modalDialog = false
polygons = {}

currentSceneFile = ""
currentScene = {
	Initialize = function()
		-- do nothing by default
	end,
	Tick = function()
		-- do nothing by default
	end
}

function OpenScene(filename)
	currentSceneFile = filename
	cast = {}
	polygons = {}
	dofile(filename)
	if restoring then
		currentScene.Deserialize()
		restoring = false
		starting = true
		return
	end
	currentScene.Initialize()
	if starting then
		starting = false
		GameLoop()
	end
end

function Animate(theCast)
	theCast = theCast or cast
	for k, v in pairs(theCast) do
		v:Update()
		v:Draw()
	end
end

function GameLoop()
	while (not quit) do
		PrepareFrame()
		currentScene.Tick()
		events = {}
		HandleEvents()

		for k, v in pairs(events) do
			if v.type == 17 then -- key press
				if v.scan == 25 and v.alt then -- Alt-V: Show Visual
					ShowScreen(0)
					v.handled = true
				elseif v.scan == 19 and v.alt then -- Alt-P: Show Priority
					ShowScreen(1)
					v.handled = true
				end
			end
		end

		Animate()
		ShowFrame()		
		Delay(delay)
	end
end

function Serialize(filename)
	Serializer.StartSave(filename)
	Serializer.SetInteger(delay)
	Serializer.SetString(currentSceneFile)
	currentScene.Serialize()
	backgroundMusic:Serialize()
	Serializer.Finish()
end

function Deserialize(filename)
	restoring = true
	events = {}
	Serializer.StartLoad(filename)
	delay = Serializer.GetInteger()
	currentSceneFile = Serializer.GetString()
	OpenScene(currentSceneFile)
	backgroundMusic:Deserialize()
	Serializer.Finish()
	GameLoop()
end

-- ----------------------------------------------- --

function table.removeByVal(t, e)
	for i, v in pairs(t) do
		if v == e then
			table.remove(t, i)
			return
		end
	end
end

-- ----------------------------------------------- --
-- VIEW STUFF
-- ----------------------------------------------- --


dofile("class.lua")

ViewObj = {}
ViewObj.new = class(function(v, theView, theX, theY)
	v.view = theView
	v.x = theX or 0
	v.y = theY or 0
	v.pri = v.y
	v.loop = 0
	v.cel = 0
	v.looper = nil
	v.cycler = nil
	v.mover = nil
	v.Draw = vobDraw
	v.Update = vobUpdate
	v.Move = vobMove
	v.SetHeading = vobSetHeading
	v.MoveTo = vobMoveTo
end)

function vobDraw(v)
	v.view:Draw(v.loop, v.cel, v.x, v.y, v.pri, false)
end

function vobUpdate(v)
--	if v.looper then v:looper() end
	if v.mover then v:mover() end
	if v.cycler then v:cycler() end
end

function vobMove(v, theX, theY)
	v.x = theX
	v.y = theY
	if v.pri ~= -1 then
		v.pri = v.y
	end
end

function vobSetHeading(v, heading)
	if v.looper then v:looper(heading)
	else DirLoop(v, heading) end
end

function vobMoveTo(v, x, y)
	v.moveTargetX = x
	v.moveTargetY = y
	v.moveCompleted = false
	v.moveLastX = 0
	v.moveLastY = 0
	v:SetHeading(GetAngle(v.x, v.y, x, y))
	--InitBresen(v)
	v.mover = moverMoveTo
end

function moverMoveTo(v)
	v.moveLastX = v.x
	v.moveLastY = v.y
	--DoBresen(v)
	if v.x == v.moveTargetX and v.y == v.moveTargetY then
		v.moveCompleted = true
		v.mover = nil
	end
end

-- ----------------------------------------------- --
-- LOOPERS AND CYCLERS for views
-- ----------------------------------------------- --

function StandAndLook(v, angle)
	-- given a v.target, sets view to match angle hopefully
	-- requires a non-nil v.target and loop to be 8
	if v.target == nil then return end
	DirLoop(v, angle)
	local cel = v.loop
	v.loop = 8
	v.cel = cel
end

function CycleForward(v)
	local cel = v.cel + 1
	if cel == v.view:GetNumCels(v.loop) then cel = 0 end
	v.cel = cel
end

function CycleBackward(v)
	local cel = v.cel
	if cel == 0 then cel = v.view:GetNumCels(v.loop) end
	v.cel = cel - 1
end

-- ----------------------------------------------- --

function DirLoop(v, angle)
	-- Old hardcoded 4-directional results
	local loop = 0
	local numLoops = v.view:GetNumLoops()
	if numLoops < 8 then
		if (angle > 315) or (angle < 45) then
			if numLoops >= 4 then loop = 3 else loop = -1 end
		elseif (angle > 135) and (angle < 225) then
			if numLoops >= 4 then loop = 2 else loop = -1 end
		elseif (angle < 180) then
			loop = 0
		else
			loop = 1
		end
	else
		if (angle > 298) or (angle < 22) then loop = 3
		elseif (angle >= 22) and (angle < 67) then loop = 6
		elseif (angle >= 67) and (angle < 112) then loop = 0
		elseif (angle >= 112) and (angle < 157) then loop = 4
		elseif (angle >= 157) and (angle < 202) then loop = 2
		elseif (angle >= 202) and (angle < 247) then loop = 5
		elseif (angle >= 247) and (angle < 292) then loop = 1
		else loop = 7
		end
	end
	
	if loop ~= -1 then
		v.loop = loop
	end
end


function GetAngle(x1, y1, x2, y2)
	return ATan(y2, x1, y1, x2)
	--[[
	local angle = ATan(y2, x1, y1, x2)
	print ("GetAngle(" .. x1 .. ", " .. y1 .. ", " .. x2 .. ", " .. y2 .. ") => " .. angle .. "\n")
	return angle;
	]]--
end
