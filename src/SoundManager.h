#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>
#include <string>

enum class SOUND_ID {HIT_PADDLE, HIT_WALL, POINT_SCORED};

class SoundManager
{
public:
	bool Init();
	bool LoadSoundFromFile(SOUND_ID id, const std::string& filepath);
	void PlaySound(SOUND_ID);
	~SoundManager();
private:
	std::map<SOUND_ID, Mix_Chunk*> SoundMap;
};