#include "TextGenerator.h"

TextGenerator::TextGenerator() : mTexture(nullptr), mFont(nullptr), mWidth(0), mHeight(0)
{
}

bool TextGenerator::Init()
{
	if (TTF_Init() == -1)
	{
		SDL_Log("Failed to initialise SDL_ttf. Error: %s\n", TTF_GetError());
		return false;
	}

	return true;
}

bool TextGenerator::LoadFont(const std::string& filepath, int fontSize)
{
	mFont = TTF_OpenFont(filepath.c_str(), fontSize);
	if (!mFont)
	{
		SDL_Log("Failed to load font. Error: %s\n", TTF_GetError());
		return false;
	}

	return true;
}

void TextGenerator::Render(SDL_Renderer* renderer, const std::string& text, int x, int y)
{
	if (CreateTextTexture(renderer, text))
	{
		SDL_Rect renderQuad{ x, y, mWidth, mHeight };
		SDL_RenderCopy(renderer, mTexture, nullptr, &renderQuad);
	}
}

void TextGenerator::FreeTexture()
{
	if (mTexture)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
		mWidth = 0;
		mHeight = 0;
	}
}

bool TextGenerator::CreateTextTexture(SDL_Renderer* renderer, const std::string& text)
{
	FreeTexture();

	SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, text.c_str(), SDL_Color{ 255,255,255,255 });
	if (!textSurface)
	{
		SDL_Log("Failed to create text surface. Error: %s\n", TTF_GetError());
		return false;
	}

	mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	if (!mTexture)
	{
		SDL_Log("Failed to create texture from surface. Error: %s\n", SDL_GetError());
		return false;
	}

	mWidth = textSurface->w;
	mHeight = textSurface->h;

	SDL_FreeSurface(textSurface);

	return true;
}

TextGenerator::~TextGenerator()
{
	FreeTexture();

	TTF_CloseFont(mFont);
	mFont = nullptr;

	TTF_Quit();
}