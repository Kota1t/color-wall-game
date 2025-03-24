#pragma once
#include <SDL2/SDL.h>

// ヘルパー：SDL_Color同士の比較
bool isSameColor(const SDL_Color& a, const SDL_Color& b);

// ヘルパー：円を描画する
void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY,
                      int radius);