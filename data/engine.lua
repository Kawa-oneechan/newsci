starting = true
restoring = false
quit = false
delay = 50
cast = {}
events = {}
backgroundMusic = nil

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

dofile("class.lua")

ViewObj = {}
ViewObj.new = class(function(v, theView, theX, theY)
	v.view = theView
	v.x = theX or 0
	v.y = theY or 0
	v.pri = v.y
	v.loop = 0
	v.cel = 0
	v.Draw = vobDraw
	v.Move = vobMove
end)

function vobDraw(v)
	v.view:Draw(v.loop, v.cel, v.x, v.y, v.pri, false)
end

function vobMove(v, theX, theY)
	v.x = theX
	v.y = theY
	if v.pri ~= -1 then
		v.pri = v.y
	end
end
