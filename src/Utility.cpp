#include "Utility.h"

// SDL_Color同士の比較
bool isSameColor(const SDL_Color& a, const SDL_Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

// 円を描画する
void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY,
                      int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx - radius,
                                    centerY + dy - radius);
            }
        }
    }
}