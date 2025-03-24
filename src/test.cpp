/*#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>

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
const SDL_Rect directiveRect = {WINDOW_WIDTH - 100, 20, 80, 80};

// タイマーゲージの表示位置（左上）
const SDL_Rect gaugeRect = {20, 20, GAUGE_WIDTH, GAUGE_HEIGHT};

// スコア表示位置（画面上部中央あたり）
const int scorePosX = WINDOW_WIDTH / 2;
const int scorePosY = 30;

// プレイヤーのサイズ（半径）
const int PLAYER_RADIUS = 15;

// 点滅間隔（ミリ秒）
const Uint32 BLINK_INTERVAL = 200;

// ゲーム状態
enum GameState { STATE_COUNTDOWN, STATE_PLAYING, STATE_MOVING, STATE_GAMEOVER };

// 移動方向
enum Direction { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

// 用いる色（赤、青、黄、緑）
SDL_Color RED = {255, 0, 0, 255};
SDL_Color BLUE = {0, 0, 255, 255};
SDL_Color YELLOW = {255, 255, 0, 255};
SDL_Color GREEN = {0, 255, 0, 255};
SDL_Color WHITE = {255, 255, 255, 255};
SDL_Color BLACK = {0, 0, 0, 255};

// 配列にまとめるとランダム選択が楽
const SDL_Color colorSet[4] = {RED, BLUE, YELLOW, GREEN};

// 壁の色を格納する変数（順に：上、下、左、右）
SDL_Color wallTopColor, wallBottomColor, wallLeftColor, wallRightColor;

// 指示色（directive）
SDL_Color directiveColor;

// ゲームの各変数
GameState gameState = STATE_COUNTDOWN;
Uint32 gameOverTime = 0;
Direction moveDir = DIR_NONE;

// プレイヤーの位置（floatで管理）
float playerX = WINDOW_WIDTH / 2.0f;
float playerY = WINDOW_HEIGHT / 2.0f;

// 移動アニメーション用
float startX, startY;
float targetX, targetY;
Uint32 moveStartTime = 0;

// タイマー関連
float currentTime = INITIAL_MAX_TIME;     // 現在の残り秒数
float currentMaxTime = INITIAL_MAX_TIME;  // 現在のMAX秒
Uint32 lastBlinkTime = 0;
bool blinkOn = true;

// スコア・成功回数
int score = 0;
int successCount = 0;  // 5回毎に currentMaxTime を減らすためのカウンタ

// フォント
TTF_Font* font = nullptr;

// SDL_Rect for walls
SDL_Rect topWall = {0, 0, WINDOW_WIDTH, WALL_THICKNESS};
SDL_Rect bottomWall = {0, WINDOW_HEIGHT - WALL_THICKNESS, WINDOW_WIDTH,
                       WALL_THICKNESS};
SDL_Rect leftWall = {0, 0, WALL_THICKNESS, WINDOW_HEIGHT};
SDL_Rect rightWall = {WINDOW_WIDTH - WALL_THICKNESS, 0, WALL_THICKNESS,
                      WINDOW_HEIGHT};

// ヘルパー：SDL_Color同士の比較
bool isSameColor(const SDL_Color& a, const SDL_Color& b) {
    return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

// ヘルパー：ランダムに色を選ぶ（色セットから）
SDL_Color getRandomColor() {
    int index = rand() % 4;
    return colorSet[index];
}

// 壁の色をランダムに決定。必ずどこかに directiveColor が含まれるようにする。
void randomizeWallColors() {
    wallTopColor = getRandomColor();
    wallBottomColor = getRandomColor();
    wallLeftColor = getRandomColor();
    wallRightColor = getRandomColor();

    // 4枚の壁の中から1枚ランダムに選び、必ず directiveColor にする
    int wallIndex = rand() % 4;
    switch (wallIndex) {
        case 0:
            wallTopColor = directiveColor;
            break;
        case 1:
            wallBottomColor = directiveColor;
            break;
        case 2:
            wallLeftColor = directiveColor;
            break;
        case 3:
            wallRightColor = directiveColor;
            break;
    }
}

// ヘルパー：レンダリング用テキスト生成
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                        const std::string& message, SDL_Color color, int& textW,
                        int& textH) {
    SDL_Surface* surf = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!surf) {
        SDL_Log("TTF_RenderText_Solid Error: %s", TTF_GetError());
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!texture) {
        SDL_Log("CreateTexture Error: %s", SDL_GetError());
    }
    textW = surf->w;
    textH = surf->h;
    SDL_FreeSurface(surf);
    return texture;
}

// ヘルパー：簡易な塗りつぶし円描画（小さい円なら十分）
void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY,
                      int radius) {
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderDrawPoint(renderer, centerX + w, centerY + h);
            }
        }
    }
}

// カウントダウン表示（「3,2,1,START」）
// 1秒ずつ表示し、画面中央に描画します。
void runCountdown(SDL_Renderer* renderer) {
    const std::string texts[4] = {"3", "2", "1", "START"};
    Uint32 startTime = SDL_GetTicks();
    for (int i = 0; i < 4; ++i) {
        Uint32 targetTime = startTime + i * 1000;
        bool next = false;
        while (!next) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    exit(0);
                }
            }
            Uint32 now = SDL_GetTicks();
            if (now >= targetTime) {
                next = true;
            }
            // 画面クリア
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            // カウントダウンテキストの描画
            int textW, textH;
            SDL_Texture* txtTex =
                renderText(renderer, font, texts[i], WHITE, textW, textH);
            if (txtTex) {
                SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                                WINDOW_HEIGHT / 2 - textH / 2, textW, textH};
                SDL_RenderCopy(renderer, txtTex, nullptr, &dst);
                SDL_DestroyTexture(txtTex);
            }
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }
}

// 初期化： directiveColor をランダムに決定してから壁の色をランダム化
void initRound() {
    directiveColor = getRandomColor();
    randomizeWallColors();
}

// 入力に応じたターゲット位置の決定
void setMovementTarget(Direction dir) {
    // 現在のプレイヤー位置は中心からスタート
    startX = playerX;
    startY = playerY;
    moveDir = dir;
    moveStartTime = SDL_GetTicks();
    // 目標位置を、各壁の内側の座標に設定
    switch (dir) {
        case DIR_UP:
            targetX = WINDOW_WIDTH / 2.0f;
            targetY = WALL_THICKNESS;  // y <= WALL_THICKNESS と判定
            break;
        case DIR_DOWN:
            targetX = WINDOW_WIDTH / 2.0f;
            targetY = WINDOW_HEIGHT - WALL_THICKNESS;
            break;
        case DIR_LEFT:
            targetX = WALL_THICKNESS;
            targetY = WINDOW_HEIGHT / 2.0f;
            break;
        case DIR_RIGHT:
            targetX = WINDOW_WIDTH - WALL_THICKNESS;
            targetY = WINDOW_HEIGHT / 2.0f;
            break;
        default:
            break;
    }
}

// 衝突判定（入力方向に応じ、対応する壁の色と directiveColor の比較）
bool checkCollision(Direction dir) {
    switch (dir) {
        case DIR_UP:
            return (playerY <= WALL_THICKNESS &&
                    isSameColor(wallTopColor, directiveColor));
        case DIR_DOWN:
            return (playerY >= WINDOW_HEIGHT - WALL_THICKNESS &&
                    isSameColor(wallBottomColor, directiveColor));
        case DIR_LEFT:
            return (playerX <= WALL_THICKNESS &&
                    isSameColor(wallLeftColor, directiveColor));
        case DIR_RIGHT:
            return (playerX >= WINDOW_WIDTH - WALL_THICKNESS &&
                    isSameColor(wallRightColor, directiveColor));
        default:
            return false;
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init Error: %s", TTF_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Game Sample", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("CreateWindow Error: %s", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("CreateRenderer Error: %s", SDL_GetError());
        return 1;
    }

    // フォントのロード（MacOSのシステムフォントを使用）
    font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 48);
    if (!font) {
        // 代替フォントを試す
        font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 48);
        if (!font) {
            SDL_Log("TTF_OpenFont Error: %s", TTF_GetError());
            return 1;
        }
    }
    // 初回ラウンドの初期化
    initRound();

    // カウントダウンを実行（ゲーム開始前のみ）
    runCountdown(renderer);

    // ゲーム開始：状態を PLAYING に
    gameState = STATE_PLAYING;
    // プレイヤー初期位置はウィンドウ中央
    playerX = WINDOW_WIDTH / 2.0f;
    playerY = WINDOW_HEIGHT / 2.0f;
    // タイマーセット
    currentTime = currentMaxTime;

    Uint32 lastTime = SDL_GetTicks();

    bool quit = false;
    while (!quit) {
        Uint32 currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastTime) / 1000.0f;
        lastTime = currentTicks;

        // イベント処理
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (gameState == STATE_PLAYING && e.type == SDL_KEYDOWN) {
                // まだ移動中でなければ入力受付
                if (moveDir == DIR_NONE) {
                    Direction inputDir = DIR_NONE;
                    switch (e.key.keysym.sym) {
                        case SDLK_w:
                            inputDir = DIR_UP;
                            break;
                        case SDLK_s:
                            inputDir = DIR_DOWN;
                            break;
                        case SDLK_a:
                            inputDir = DIR_LEFT;
                            break;
                        case SDLK_d:
                            inputDir = DIR_RIGHT;
                            break;
                        default:
                            break;
                    }
                    if (inputDir != DIR_NONE) {
                        // 入力受け付けたら開始
                        setMovementTarget(inputDir);
                        gameState = STATE_MOVING;
                    }
                }
            }
        }

        // タイマー更新（PLAYINGおよびMOVING状態で減少）
        if (gameState == STATE_PLAYING || gameState == STATE_MOVING) {
            currentTime -= deltaTime;
            if (currentTime <= 0) {
                gameState = STATE_GAMEOVER;
                gameOverTime = SDL_GetTicks();  // ゲームオーバー時刻を記録
            }
        }

        // MOVING状態の場合：アニメーション処理
        if (gameState == STATE_MOVING) {
            Uint32 elapsed = SDL_GetTicks() - moveStartTime;
            float t = elapsed / (float)MOVE_DURATION;
            if (t >= 1.0f) {
                // 移動完了
                playerX = targetX;
                playerY = targetY;
                // 衝突判定：正しい壁に接触しているか？
                if (checkCollision(moveDir)) {
                    // 成功
                    score++;
                    successCount++;
                    // 5回成功するたびに MAX秒 を0.2秒減らす（下限1.5秒）
                    if (successCount % 5 == 0) {
                        currentMaxTime -= 0.2f;
                        if (currentMaxTime < MIN_MAX_TIME) {
                            currentMaxTime = MIN_MAX_TIME;
                        }
                    }
                    //
次ラウンドの準備：プレイヤーを中央に戻し、タイマーリセット、壁の色と指示色更新
                    playerX = WINDOW_WIDTH / 2.0f;
                    playerY = WINDOW_HEIGHT / 2.0f;
                    currentTime = currentMaxTime;
                    directiveColor = getRandomColor();
                    randomizeWallColors();
                    // 戻る
                    moveDir = DIR_NONE;
                    gameState = STATE_PLAYING;
                } else {
                    // 不正解ならゲームオーバー
                    gameState = STATE_GAMEOVER;
                    gameOverTime = SDL_GetTicks();  // ゲームオーバー時刻を記録
                }
            } else {
                // 線形補間で移動
                playerX = startX + (targetX - startX) * t;
                playerY = startY + (targetY - startY) * t;
            }
        }

        // 描画処理
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 壁の描画
        // 上壁
        SDL_SetRenderDrawColor(renderer, wallTopColor.r, wallTopColor.g,
                               wallTopColor.b, wallTopColor.a);
        SDL_RenderFillRect(renderer, &topWall);
        // 下壁
        SDL_SetRenderDrawColor(renderer, wallBottomColor.r, wallBottomColor.g,
                               wallBottomColor.b, wallBottomColor.a);
        SDL_RenderFillRect(renderer, &bottomWall);
        // 左壁
        SDL_SetRenderDrawColor(renderer, wallLeftColor.r, wallLeftColor.g,
                               wallLeftColor.b, wallLeftColor.a);
        SDL_RenderFillRect(renderer, &leftWall);
        // 右壁
        SDL_SetRenderDrawColor(renderer, wallRightColor.r, wallRightColor.g,
                               wallRightColor.b, wallRightColor.a);
        SDL_RenderFillRect(renderer, &rightWall);

        // タイマーゲージの描画
        // ゲージの横幅を currentTime/currentMaxTime に応じて算出
        int gaugeCurrentWidth =
            (int)(GAUGE_WIDTH * (currentTime / currentMaxTime));
        SDL_Rect currentGauge = {gaugeRect.x, gaugeRect.y, gaugeCurrentWidth,
                                 gaugeRect.h};
        // 背景を黒色にする（この部分を追加）
        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
        SDL_RenderFillRect(renderer, &gaugeRect);
        // ゲージが半分以下なら点滅（0.2秒ごと）
        if (gaugeCurrentWidth <= GAUGE_WIDTH / 2) {
            if (SDL_GetTicks() - lastBlinkTime > BLINK_INTERVAL) {
                blinkOn = !blinkOn;
                lastBlinkTime = SDL_GetTicks();
            }
            if (blinkOn)
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // 赤
            else
                SDL_SetRenderDrawColor(renderer, 0, 255, 0,
                                       255);  // 通常の緑（例として）
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // 通常は緑
        }
        SDL_RenderFillRect(renderer, &currentGauge);
        // ゲージの枠（オプション）
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawRect(renderer, &gaugeRect);

        // 指示枠の描画（右上）
        SDL_SetRenderDrawColor(renderer, directiveColor.r, directiveColor.g,
                               directiveColor.b, directiveColor.a);
        SDL_RenderFillRect(renderer, &directiveRect);
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_RenderDrawRect(renderer, &directiveRect);

        // スコアの描画（上部中央）
        {
            std::stringstream ss;
            ss << "Score: " << score;
            int textW, textH;
            SDL_Texture* scoreTex =
                renderText(renderer, font, ss.str(), WHITE, textW, textH);
            if (scoreTex) {
                SDL_Rect dst = {scorePosX - textW / 2, scorePosY - textH / 2,
                                textW, textH};
                SDL_RenderCopy(renderer, scoreTex, nullptr, &dst);
                SDL_DestroyTexture(scoreTex);
            }
        }

        // プレイヤー（中央の白い円）の描画
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        drawFilledCircle(renderer, (int)playerX, (int)playerY, PLAYER_RADIUS);

        // ゲームオーバーなら画面中央に "Game Over" 表示
        if (gameState == STATE_GAMEOVER) {
            quit = true;
            int textW, textH;
            SDL_Texture* goTex =
                renderText(renderer, font, "GAME OVER", RED, textW, textH);
            if (goTex) {
                SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                                WINDOW_HEIGHT / 2 - textH / 2, textW, textH};
                SDL_RenderCopy(renderer, goTex, nullptr, &dst);
                SDL_DestroyTexture(goTex);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // 約60FPS
    }

    SDL_Delay(900);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}*/
