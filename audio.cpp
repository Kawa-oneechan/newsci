#include "NewSCI.h"
#include "support\fmod\fmod.hpp"

FMOD::System* Audio::system;
std::vector<Audio*> Audio::playing;

void Audio::Initialize()
{
	auto r = FMOD::System_Create(&system);
	r = system->init(4, FMOD_INIT_NORMAL, NULL);
	Sol.new_usertype<Audio>(
		"Audio",
		sol::constructors<Audio(std::string)>(),
		"Play", &Play,
		"Pause", &Pause,
		"Stop", &Stop,
		"Serialize", &Serialize,
		"Deserialize", &Deserialize
	);
}

void Audio::CloseAll()
{
	while (!playing.empty())
	{
		auto song = playing[0];
		song->Stop();
	}
	playing.clear();
}

Audio::Audio(std::string filename)
{
	this->theSound = NULL;
	this->theChannel = NULL;
	this->status = -1;
	unsigned long size = 0;
	Handle data = LoadFile(filename, &size);
	if (!data)
		throw "Could not load file.";
	this->filename = std::string(filename);
	auto soundEx = FMOD_CREATESOUNDEXINFO();
	memset(&soundEx, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	soundEx.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	soundEx.length = size;
	auto r = system->createStream(data, FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D | FMOD_OPENMEMORY, &soundEx, &this->theSound);
	if (r != FMOD_OK)
		throw "Could not create stream.";
	auto ext = filename.substr(filename.length() - 4, 4);
	if (ext == ".ogg")
	{
		FMOD_TAG tag;
		r = this->theSound->getTag("LOOPSTART", 0, &tag);
		if (r == FMOD_OK)
		{
			unsigned int start = atoi((char*)tag.data);
			unsigned int end = 0;
			this->theSound->getLength(&end, FMOD_TIMEUNIT_PCM);
			r = this->theSound->setLoopPoints(start, FMOD_TIMEUNIT_PCM, end, FMOD_TIMEUNIT_PCM);
			if (r != FMOD_OK)
				SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Wanted to set loop point, could not.");
		}
	}
	this->status = 0;
}

Audio::~Audio()
{
	this->Stop();
	this->theSound->release();
	this->theChannel = NULL;
	this->theSound = NULL;
}

void Audio::Play()
{
	if (this->status == 0)
	{
		auto r = system->playSound(FMOD_CHANNEL_FREE, this->theSound, false, &this->theChannel);
		if (r != FMOD_OK)
			throw "Could not play stream.";
		Audio::playing.push_back(this);
	}
	else if (this->status == 2)
	{
		this->theChannel->setPaused(false);
	}
	this->status = 1;
}

void Audio::Pause()
{
	this->theChannel->setPaused(true);
	this->status = 2;
}

void Audio::Stop()
{
	if (this->status != 0)
	{
		this->theChannel->stop();
	}
	this->status = 0;
	Audio::playing.erase(std::remove(Audio::playing.begin(), Audio::playing.end(), this), Audio::playing.end());
}

void Audio::Serialize()
{
	unsigned int pos = 0;
	this->theChannel->getPosition(&pos, FMOD_TIMEUNIT_PCM);
	Serializer::SetInteger(this->status);
	Serializer::SetInteger(pos);
}

void Audio::Deserialize()
{
	int status = Serializer::GetInteger();
	unsigned int pos = Serializer::GetInteger();
	switch (status)
	{
	case 0: 
		this->Stop();
		return;
	case 1:
		this->Play();
		break;
	case 2:
		this->Play();
		this->Pause();
	}
	if (status)
		this->theChannel->setPosition(pos, FMOD_TIMEUNIT_PCM);
}
