#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

#include "Constants.h"
#include "Player.h"

class Game {
   public:
    Game();
    ~Game();

    bool initialize();
    void runGame();

   private:
    void initRound();
    void runCountdown();
    void handleEvents();
    void update(float deltaTime);
    void render();
    SDL_Texture* renderText(const std::string& message, SDL_Color color,
                            int& textW, int& textH);
    void drawFilledCircle(int centerX, int centerY, int radius);
    SDL_Color getRandomColor();
    void randomizeWallColors();

    // SDL関連
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    // ゲーム状態
    GameState gameState;
    Uint32 gameOverTime;

    // スコア関連
    int score;
    int successCount;

    // タイマー関連
    float currentTime;
    float currentMaxTime;
    Uint32 lastBlinkTime;
    bool blinkOn;

    // 色関連
    SDL_Color wallTopColor, wallBottomColor, wallLeftColor, wallRightColor;
    SDL_Color directiveColor;

    // プレイヤー
    Player player;

    // 座標・矩形
    SDL_Rect topWall, bottomWall, leftWall, rightWall;
    SDL_Rect directiveRect;
    SDL_Rect gaugeRect;
};