#include "SoundManager.h"

bool SoundManager::Init()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
	{
		SDL_Log("Failed to initialise SDL_mixer. Error: %s\n", Mix_GetError());
		return false;
	}

	return true;
}

bool SoundManager::LoadSoundFromFile(SOUND_ID id, const std::string& filename)
{
	Mix_Chunk* sound = Mix_LoadWAV(filename.c_str());
	if (!sound)
	{
		SDL_Log("Failed to load file %s\n", filename.c_str());
		return false;
	}

	SoundMap[id] = sound;
	return true;
}

void SoundManager::PlaySound(SOUND_ID id)
{
	if (SoundMap.find(id) != SoundMap.end())
	{
		Mix_PlayChannel(-1, SoundMap[id], 0);
	}
	else
	{
		SDL_Log("Error: sound not found\n");
	}
	
}

SoundManager::~SoundManager()
{
	Mix_Quit();
}