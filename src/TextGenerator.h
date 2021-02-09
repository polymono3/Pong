#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class TextGenerator
{
public:
	TextGenerator();
	bool Init();
	bool LoadFont(const std::string& filepath, int fontSize);
	void Render(SDL_Renderer*, const std::string& text, int x, int y);
	~TextGenerator();
private:
	void FreeTexture();
	bool CreateTextTexture(SDL_Renderer* renderer, const std::string& text);

	SDL_Texture* mTexture;
	TTF_Font* mFont;
	int mWidth;
	int mHeight;
};