#pragma once
#include <SDL2/SDL.h>

// ウィンドウサイズ・壁の厚さなどの定数
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int WALL_THICKNESS = 50;

// ゲームタイマー初期値（秒）
const float INITIAL_MAX_TIME = 3.0f;
const float MIN_MAX_TIME = 1.5f;

// プレイヤー移動のアニメーション時間（ミリ秒）
const Uint32 MOVE_DURATION = 300;  // 0.3秒

// ゲージ（タイマー）表示のサイズ
const int GAUGE_WIDTH = 150;
const int GAUGE_HEIGHT = 20;

// 指示枠のサイズ・位置（右上）
const SDL_Rect DIRECTIVE_RECT = {WINDOW_WIDTH - 100, 20, 80, 80};

// タイマーゲージの表示位置（左上）
const SDL_Rect GAUGE_RECT = {20, 20, GAUGE_WIDTH, GAUGE_HEIGHT};

// スコア表示位置（画面上部中央あたり）
const int SCORE_POS_X = WINDOW_WIDTH / 2;
const int SCORE_POS_Y = 30;

// プレイヤーのサイズ（半径）
const int PLAYER_RADIUS = 15;

// 点滅間隔（ミリ秒）
const Uint32 BLINK_INTERVAL = 200;

// ゲーム状態
enum GameState { STATE_COUNTDOWN, STATE_PLAYING, STATE_MOVING, STATE_GAMEOVER };

// 移動方向
enum Direction { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

// 色の定義（extern宣言）
extern SDL_Color RED;
extern SDL_Color BLUE;
extern SDL_Color YELLOW;
extern SDL_Color GREEN;
extern SDL_Color WHITE;
extern SDL_Color BLACK;
extern const SDL_Color colorSet[4];