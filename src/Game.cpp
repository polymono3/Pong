#include "Game.h"
#include <cmath>
#include "Constants.h"
#include <iostream>
#include <random>
#include <vector>

Game::Game() : mWindow(nullptr), mRenderer(nullptr), mIsRunning(true), mState(RESET),
mPaddleDir(0), mPlayerScore(0), mOpponentScore(0), mTicksCount(0), mResetTicks(2000)
{
}

bool Game::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Failed to initialise SDL. Error: %s\n", SDL_GetError());
		return false;
	}

	mSoundManager.Init();

	mTextGenerator.Init();

	mWindow = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (!mWindow)
	{
		SDL_Log("Failed to create window. Error: %s\n", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer. Error: %s\n", SDL_GetError());
		return false;
	}

	mSoundManager.LoadSoundFromFile(SOUND_ID::HIT_PADDLE, "Sounds/paddle_hit.wav");
	mSoundManager.LoadSoundFromFile(SOUND_ID::HIT_WALL, "Sounds/wall_hit.wav");
	mSoundManager.LoadSoundFromFile(SOUND_ID::POINT_SCORED, "Sounds/point_scored.wav");

	mTextGenerator.LoadFont("Fonts/VCR_OSD_MONO.ttf", 70);
	
	Paddle player{ 15.0f, SCREEN_HEIGHT / 2.0f, 0 };
	Paddle opponent{ SCREEN_WIDTH - 15.0f, SCREEN_HEIGHT / 2.0f, 0 };
	mPaddles[PLAYER] = player;
	mPaddles[OPPONENT] = opponent;

	mBallPos.x = SCREEN_WIDTH / 2.0f;
	mBallPos.y = SCREEN_HEIGHT / 2.0f;
	InitBallVelocity();

	return true;
}

void Game::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}

void Game::Run()
{
	while (mIsRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	//Paddle input
	mPaddles[PLAYER].dir = 0;

	if (state[SDL_SCANCODE_W])
	{
		mPaddles[PLAYER].dir -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddles[PLAYER].dir += 1;
	}
}

void Game::Update()
{
	// Cap frame rate to 60fps
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	// time elapsed since last frame
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	// if debugging, prevent deltaTime becoming too large
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// time elapsed since application started
	mTicksCount = SDL_GetTicks();
	
	// update position of each paddle
	for (auto &pair : mPaddles)
	{
		if (pair.second.dir != 0)
		{
			pair.second.pos.y += pair.second.dir * PADDLE_SPEED * deltaTime;

			if (pair.second.pos.y < WALL_THICKNESS + PADDLE_HEIGHT / 2)
			{
				pair.second.pos.y = WALL_THICKNESS + PADDLE_HEIGHT / 2;
			}

			else if (pair.second.pos.y > SCREEN_HEIGHT - WALL_THICKNESS - PADDLE_HEIGHT / 2)
			{
				pair.second.pos.y = SCREEN_HEIGHT - WALL_THICKNESS - PADDLE_HEIGHT / 2;
			}
		}
	}

	// Set the direction of the opponent paddle
	if (mBallPos.x > SCREEN_WIDTH / 4)
	{
		mPaddles[OPPONENT].dir = sign(mBallPos.y - mPaddles[OPPONENT].pos.y, 5.0f);
	}
	else
	{
		mPaddles[OPPONENT].dir = sign(SCREEN_HEIGHT / 2.0f - mPaddles[OPPONENT].pos.y, 5.0f);
	}

	if (mState == RESET)
	{
		mResetTicks -= deltaTime * 1000.0f;
		if (mResetTicks < 0)
		{
			mState = ACTIVE;
			mResetTicks = 0;
		}
	}

	if (mState == ACTIVE)
	{
		mBallPos.x += mBallVel.x * deltaTime;
		mBallPos.y += mBallVel.y * deltaTime;
	}
	
	HandleCollisions();
}

void Game::HandleCollisions()
{
	// Collisions with ball
	// Top wall
	if (mBallPos.y < WALL_THICKNESS + BALL_SIZE / 2 && mBallVel.y < 0.0f)
	{
		mBallVel.y *= -1.0f;
		mSoundManager.PlaySound(SOUND_ID::HIT_WALL);
	}
	// Bottom wall
	else if (mBallPos.y > SCREEN_HEIGHT - WALL_THICKNESS - BALL_SIZE / 2 && mBallVel.y > 0.0f)
	{
		mBallVel.y *= -1.0f;
		mSoundManager.PlaySound(SOUND_ID::HIT_WALL);
	}
	// Paddles
	for (auto &pair : mPaddles)
	{
		float diffY = mBallPos.y - pair.second.pos.y;
		float diffX = mBallPos.x - pair.second.pos.x;

		// If there is a collision between the ball and a paddle
		if (fabs(diffY) <= PADDLE_HEIGHT / 2.0f + BALL_SIZE / 2.0f && fabs(diffX) <= PADDLE_THICKNESS / 2.0f + BALL_SIZE / 2.0f && BallTowardsPad(pair.second))
		{
			// Distance from point of impact to paddle centre as a fraction of the paddle height
			float diffYFraction = fabs(diffY) / (PADDLE_HEIGHT / 2.0f);

			float bounceAngle = diffYFraction * MAX_BOUNCE_ANGLE;
			float ballSpeed = diffYFraction * MAX_BALL_SPEED;
			clamp(ballSpeed, INITIAL_BALL_SPEED, MAX_BALL_SPEED);
			mBallVel.x = cosf(bounceAngle) * sign(mBallVel.x) * ballSpeed;
			mBallVel.y = sinf(bounceAngle) * sign(mBallVel.y) * ballSpeed;

			if ((diffY > 0 && mBallVel.y < 0) || (diffY < 0 && mBallVel.y > 0))
			{
				mBallVel.y *= -1.0f;
			}

			mBallVel.x *= -1.0f;
			mSoundManager.PlaySound(SOUND_ID::HIT_PADDLE);
		}
	}
	
	// Ball moves off screen?
	if (mBallPos.x < 0.0f)
	{
		mSoundManager.PlaySound(SOUND_ID::POINT_SCORED);
		++mOpponentScore;
		ResetBall();
	}
	else if (mBallPos.x > SCREEN_WIDTH)
	{
		mSoundManager.PlaySound(SOUND_ID::POINT_SCORED);
		++mPlayerScore;
		ResetBall();
	}

}

void Game::ResetBall()
{
	mBallPos.x = SCREEN_WIDTH / 2.0f;
	mBallPos.y = SCREEN_HEIGHT / 2.0f;
	InitBallVelocity();
	mResetTicks = 2000;
	mState = RESET;
}

void Game::Render()
{
	// Clear back buffer
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mRenderer);
	
	// Draw scoreboard
	mTextGenerator.Render(mRenderer, std::to_string(mPlayerScore), SCREEN_WIDTH * 0.25f, 20.0f);
	mTextGenerator.Render(mRenderer, std::to_string(mOpponentScore), SCREEN_WIDTH * 0.75f, 20.0f);

	// Draw dashed line down the centre
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	for (int y = 0; y < SCREEN_HEIGHT; y += SCREEN_HEIGHT / 20)
	{
		SDL_Rect dashedLine{ (SCREEN_WIDTH - 2) / 2, y, 2, 20 };
		SDL_RenderFillRect(mRenderer, &dashedLine);
	}

	// Draw walls:
	SDL_SetRenderDrawColor(mRenderer, 200, 200, 200, 255);
	SDL_Rect wall{ 0,0,SCREEN_WIDTH,WALL_THICKNESS };
	// Top wall
	SDL_RenderFillRect(mRenderer, &wall);
	// Bottom wall
	wall.y = SCREEN_HEIGHT - WALL_THICKNESS;
	SDL_RenderFillRect(mRenderer, &wall);

	// Draw paddles
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	for (auto &pair : mPaddles)
	{
		SDL_Rect rect{
			static_cast<int>(pair.second.pos.x - PADDLE_THICKNESS / 2),
			static_cast<int>(pair.second.pos.y - PADDLE_HEIGHT / 2),
			PADDLE_THICKNESS,
			PADDLE_HEIGHT
		};

		SDL_RenderFillRect(mRenderer, &rect);
	}

	// Draw ball
	SDL_Rect ball{
		static_cast<int>(mBallPos.x - BALL_SIZE/2),
		static_cast<int>(mBallPos.y - BALL_SIZE/2),
		BALL_SIZE,
		BALL_SIZE
	};
	SDL_RenderFillRect(mRenderer, &ball);

	// Swap front and back buffers
	SDL_RenderPresent(mRenderer);
}

void Game::InitBallVelocity()
{
	// Randomise the ball's initial velocity
	std::vector<int> direction{ -1, 1 };
	std::uniform_int_distribution<unsigned> u(0, 1);
	std::default_random_engine e(SDL_GetTicks());
	mBallVel.x = direction[u(e)] * cosf(45.0f * PI / 180.0f) * INITIAL_BALL_SPEED;
	mBallVel.y = direction[u(e)] * sinf(45.0f * PI / 180.0f) * INITIAL_BALL_SPEED;
}

bool Game::BallTowardsPad(Paddle &paddle)
{
	if (paddle.pos.x < SCREEN_WIDTH / 2.0f && mBallVel.x < 0.0f)
		return true;
	if (paddle.pos.x > SCREEN_WIDTH / 2.0f && mBallVel.x > 0.0f)
		return true;

	return false;
}

int sign(float x, float tolerance)
{
	if (x > tolerance)
		return 1;
	if (x < -tolerance)
		return -1;

	return 0;
}

void clamp(float &x, float min, float max)
{
	if (x < min)
		x = min;
	else if (x > max)
		x = max;
}