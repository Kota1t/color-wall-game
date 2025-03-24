#include "Player.h"

#include <cmath>

#include "Utility.h"

Player::Player() { reset(); }

void Player::reset() {
    // プレイヤーを画面中央に配置
    x = WINDOW_WIDTH / 2.0f;
    y = WINDOW_HEIGHT / 2.0f;
    moveDir = DIR_NONE;
    moveStartTime = 0;
    startX = x;
    startY = y;
    targetX = x;
    targetY = y;
}

void Player::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
    startX = x;
    startY = y;
    targetX = x;
    targetY = y;
}

void Player::setMovementTarget(Direction dir) {
    if (dir == DIR_NONE || isMoving()) {
        return;
    }

    moveDir = dir;
    moveStartTime = SDL_GetTicks();
    startX = x;
    startY = y;

    // 設定した方向に応じて目標座標を計算
    switch (dir) {
        case DIR_UP:
            targetX = x;
            targetY = WALL_THICKNESS + PLAYER_RADIUS;
            break;
        case DIR_DOWN:
            targetX = x;
            targetY = WINDOW_HEIGHT - WALL_THICKNESS - PLAYER_RADIUS;
            break;
        case DIR_LEFT:
            targetX = WALL_THICKNESS + PLAYER_RADIUS;
            targetY = y;
            break;
        case DIR_RIGHT:
            targetX = WINDOW_WIDTH - WALL_THICKNESS - PLAYER_RADIUS;
            targetY = y;
            break;
        default:
            break;
    }
}

void Player::update(Uint32 currentTime) {
    if (moveDir == DIR_NONE) {
        return;
    }

    // 経過時間に基づいて位置を線形補間
    Uint32 elapsed = currentTime - moveStartTime;
    float t = static_cast<float>(elapsed) / MOVE_DURATION;

    // 移動完了後は位置を固定
    if (t >= 1.0f) {
        t = 1.0f;
    }

    // 線形補間で位置を更新
    x = startX + t * (targetX - startX);
    y = startY + t * (targetY - startY);
}

void Player::render(SDL_Renderer* renderer) {
    // プレイヤーを白い円として描画
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);

    // 円の描画（簡易的な実装）
    for (int w = 0; w < PLAYER_RADIUS * 2; w++) {
        for (int h = 0; h < PLAYER_RADIUS * 2; h++) {
            int dx = PLAYER_RADIUS - w;
            int dy = PLAYER_RADIUS - h;
            if ((dx * dx + dy * dy) <= (PLAYER_RADIUS * PLAYER_RADIUS)) {
                SDL_RenderDrawPoint(renderer,
                                    static_cast<int>(x) + dx - PLAYER_RADIUS,
                                    static_cast<int>(y) + dy - PLAYER_RADIUS);
            }
        }
    }
}

bool Player::checkCollision(Direction dir, SDL_Color wallTopColor,
                            SDL_Color wallBottomColor, SDL_Color wallLeftColor,
                            SDL_Color wallRightColor,
                            SDL_Color directiveColor) {
    // 移動方向と壁の色が一致するかをチェック
    switch (dir) {
        case DIR_UP:
            return isSameColor(wallTopColor, directiveColor);
        case DIR_DOWN:
            return isSameColor(wallBottomColor, directiveColor);
        case DIR_LEFT:
            return isSameColor(wallLeftColor, directiveColor);
        case DIR_RIGHT:
            return isSameColor(wallRightColor, directiveColor);
        default:
            return false;
    }
}

bool Player::isMoving() const { return moveDir != DIR_NONE; }

bool Player::isMovementComplete(Uint32 currentTime) const {
    if (moveDir == DIR_NONE) {
        return false;
    }
    return (currentTime - moveStartTime) >= MOVE_DURATION;
}