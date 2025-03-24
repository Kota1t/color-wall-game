#pragma once
#include <SDL2/SDL.h>

#include "Constants.h"

class Player {
   public:
    Player();
    void reset();
    void setMovementTarget(Direction dir);
    void update(Uint32 currentTime);
    void render(SDL_Renderer* renderer);
    bool checkCollision(Direction dir, SDL_Color wallTopColor,
                        SDL_Color wallBottomColor, SDL_Color wallLeftColor,
                        SDL_Color wallRightColor, SDL_Color directiveColor);
    bool isMoving() const;
    bool isMovementComplete(Uint32 currentTime) const;

    // アクセサ
    float getX() const { return x; }
    float getY() const { return y; }
    Direction getMoveDir() const { return moveDir; }
    Uint32 getMoveStartTime() const { return moveStartTime; }
    float getTargetX() const { return targetX; }
    float getTargetY() const { return targetY; }

    // 座標設定
    void setPosition(float newX, float newY);

   private:
    float x, y;              // 現在位置
    float startX, startY;    // 移動開始位置
    float targetX, targetY;  // 移動目標位置
    Direction moveDir;       // 現在の移動方向
    Uint32 moveStartTime;    // 移動開始時刻
};