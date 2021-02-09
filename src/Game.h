#pragma once
#include <SDL.h>
#include <map>
#include "SoundManager.h"
#include "TextGenerator.h"

struct Vector2
{
	float x, y;
};

struct Paddle
{
	Vector2 pos;
	int dir;
};

enum State {ACTIVE, RESET};
enum PADDLE_ID {PLAYER, OPPONENT};

class Game
{
public:
	Game();
	bool Init();
	void Run();
	void Shutdown();

private:
	void ProcessInput();
	void Update();
	void Render();

	void HandleCollisions();
	void InitBallVelocity();
	bool BallTowardsPad(Paddle &paddle);
	void ResetBall();

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;

	SoundManager mSoundManager;
	TextGenerator mTextGenerator;

	bool mIsRunning;
	State mState;

	std::map<PADDLE_ID, Paddle> mPaddles;

	Vector2 mPaddlePos;
	Vector2 mBallPos;
	Vector2 mBallVel;
	int mPaddleDir;

	Vector2 mOpponentPos;
	int mOpponentDir;

	int mPlayerScore;
	int mOpponentScore;

	Uint32 mTicksCount;
	int mResetTicks;
};

int sign(float x, float tolerance = 0.0f);

void clamp(float &x, float min, float max);