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
	static void Initialize();
	static void CloseAll();
	Audio(std::string filename);
	~Audio();
	void Play();
	void Pause();
	void Stop();
	void Deserialize();
	void Serialize();
};

