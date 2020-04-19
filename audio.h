//AUDIO.H
//-------
//Everything about audio. Kernel call, FMOD wrapping...
#pragma once
#include "support\fmod\fmod.hpp"

class Audio
{
private:
	static FMOD::System* Audio::system;
	static std::vector<Audio*> playing;
	FMOD::Sound* theSound;
	FMOD::Channel* theChannel;
	int status;
	std::string filename;
public:
	///<summary>Initializes FMOD if audio is enabled, and hooks the Audio class up to Sol.</summary>
	static void Initialize();
	///<summary>Stops all currently-playing tracks and unloads them.</summary>
	static void CloseAll();
	///<summary>Instantiates an Audio object with the given source file. Will also set up loop points for OGG files with LOOPSTART tags.</summary>
	///<param name="filename">The audio file to load from.</param>
	Audio(std::string filename);
	///<summary>Stops playing and releases all FMOD handles associated with this track.</param>
	~Audio();
	///<summary>Begins or resumes playing this track, if audio is enabled.</summary>
	void Play();
	///<summary>Pauses this track if it's playing and audio is enabled.</summary>
	void Pause();
	///<summary>Stops this track if it's playing and audio is enabled.</summary>
	void Stop();
	///<summary>Reloads a track's playstate and position from serialization.</summary>
	void Deserialize();
	///<summary>Saves a track's playstate and position to a serialization.</summary>
	void Serialize();
};

