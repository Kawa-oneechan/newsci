starting = true
restoring = false
quit = false
delay = 50
cast = {}
events = {}
backgroundMusic = nil
modalDialog = false

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
	v.Draw = vobDraw
	v.Update = vobUpdate
	v.Move = vobMove
	v.SetHeading = vobSetHeading
end)

function vobDraw(v)
	v.view:Draw(v.loop, v.cel, v.x, v.y, v.pri, false)
end

function vobUpdate(v)
--	if v.looper then v:looper() end
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

-- ----------------------------------------------- --
-- LOOPERS AND CYCLERS for views
-- ----------------------------------------------- --

function LooperLook(v)
	-- given a v.target, sets view to match angle hopefully
	-- requires a non-nil v.target and loop to be 8
	if v.target == nil then return end
	
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
	local numLoops = v.view:GetNumCels(v.loop)
	local loop = 0
	if (angle > 315) or (angle < 45) then
		if numLoops >= 4 then loop = 3 else loop = -1 end
	elseif (angle > 135) and (angle < 225) then
		if numLoops >= 4 then loop = 2 else loop = -1 end
	elseif (angle < 180) then
		loop = 0
	else
		loop = 1
	end
	
	if loop ~= -1 then
		v.loop = loop
	end
end


function GetAngle(x1, y1, x2, y2)
	-- local angle = math.atan(y2 - y1, x2 - x1) * 180 / math.pi;
	local angle = ATan(y2, x1, y1, x2)
	print ("GetAngle(" .. x1 .. ", " .. y1 .. ", " .. x2 .. ", " .. y2 .. ") => " .. angle .. "\n")
	return angle;
end
