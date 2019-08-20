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

function GameLoop()
	while (not quit) do
		currentScene.Tick()
		events = {}
		ShowFrame(cast)		
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
