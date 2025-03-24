#include "game.h"

#include <cstdlib>
#include <ctime>
#include <sstream>

#include "Constants.h"
#include "Player.h"
#include "Utility.h"

Game::Game()
    : window(nullptr),
      renderer(nullptr),
      font(nullptr),
      gameState(STATE_COUNTDOWN),
      gameOverTime(0),
      score(0),
      successCount(0),
      currentTime(INITIAL_MAX_TIME),
      currentMaxTime(INITIAL_MAX_TIME),
      lastBlinkTime(0),
      blinkOn(false) {
    // 壁の矩形初期化
    topWall = {0, 0, WINDOW_WIDTH, WALL_THICKNESS};
    bottomWall = {0, WINDOW_HEIGHT - WALL_THICKNESS, WINDOW_WIDTH,
                  WALL_THICKNESS};
    leftWall = {0, 0, WALL_THICKNESS, WINDOW_HEIGHT};
    rightWall = {WINDOW_WIDTH - WALL_THICKNESS, 0, WALL_THICKNESS,
                 WINDOW_HEIGHT};

    // UIの矩形初期化
    directiveRect = DIRECTIVE_RECT;
    gaugeRect = GAUGE_RECT;
}

Game::~Game() {
    // リソース解放
    if (font) {
        TTF_CloseFont(font);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }

    // SDL終了
    TTF_Quit();
    SDL_Quit();
}

bool Game::initialize() {
    // SDL初期化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return false;
    }

    // SDL_ttf初期化
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init Error: %s", TTF_GetError());
        SDL_Quit();
        return false;
    }

    // ウィンドウ作成
    window = SDL_CreateWindow("Wall Color Game", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // レンダラー作成
    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // フォント読み込み
    font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 24);
    if (!font) {
        // 代替フォントを試す
        font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 24);
        if (!font) {
            SDL_Log("TTF_OpenFont Error: %s", TTF_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return false;
        }
    }

    // ゲームの初期設定
    initRound();

    return true;
}

void Game::runGame() {
    // ゲームループの変数
    bool quit = false;
    Uint32 lastTime = SDL_GetTicks();
    const float deltaTimeStep = 1.0f / 60.0f;  // 60fps想定

    // メインゲームループ
    while (!quit) {
        // デルタタイム計算
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // イベント処理
        handleEvents();

        // 更新
        update(deltaTime);

        // 描画
        render();

        // ゲームオーバー時の処理
        if (gameState == STATE_GAMEOVER &&
            (SDL_GetTicks() - gameOverTime >= 2000)) {
            quit = true;
        }

        // フレームレート調整
        SDL_Delay(16);  // 約60FPS
    }
}

void Game::initRound() {
    // プレイヤー位置を中央に
    player.reset();

    // スコア初期化
    score = 0;
    successCount = 0;

    // タイマー初期化
    currentTime = INITIAL_MAX_TIME;
    currentMaxTime = INITIAL_MAX_TIME;
    lastBlinkTime = SDL_GetTicks();
    blinkOn = false;

    // 色をランダムに設定
    directiveColor = getRandomColor();
    randomizeWallColors();

    // ゲーム状態設定
    gameState = STATE_PLAYING;
}

void Game::runCountdown() {
    // カウントダウン表示（3, 2, 1, Go!）
    int countdown = 3;
    Uint32 lastCountdownTime = SDL_GetTicks();

    while (countdown > 0) {
        // イベント処理
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gameState = STATE_GAMEOVER;
                return;
            }
        }

        // 1秒ごとにカウントダウン
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastCountdownTime >= 1000) {
            countdown--;
            lastCountdownTime = currentTime;
        }

        // 背景描画
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // カウントダウン表示
        std::stringstream ss;
        ss << countdown;
        int textW, textH;
        SDL_Texture* countdownTex = renderText(ss.str(), WHITE, textW, textH);
        if (countdownTex) {
            SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                            WINDOW_HEIGHT / 2 - textH / 2, textW, textH};
            SDL_RenderCopy(renderer, countdownTex, nullptr, &dst);
            SDL_DestroyTexture(countdownTex);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // 60fps相当
    }

    // "Go!" 表示
    Uint32 goStartTime = SDL_GetTicks();
    bool goShown = true;

    while (goShown) {
        // イベント処理
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gameState = STATE_GAMEOVER;
                return;
            }
        }

        // 1秒間 "Go!" を表示
        if (SDL_GetTicks() - goStartTime >= 1000) {
            goShown = false;
        }

        // 描画
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // "Go!" 表示
        int textW, textH;
        SDL_Texture* goTex = renderText("Go!", GREEN, textW, textH);
        if (goTex) {
            SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                            WINDOW_HEIGHT / 2 - textH / 2, textW, textH};
            SDL_RenderCopy(renderer, goTex, nullptr, &dst);
            SDL_DestroyTexture(goTex);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // ゲーム開始
    gameState = STATE_PLAYING;
}

SDL_Color Game::getRandomColor() {
    int index = rand() % 4;
    return colorSet[index];
}

void Game::randomizeWallColors() {
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

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        // 終了イベント
        if (e.type == SDL_QUIT) {
            gameState = STATE_GAMEOVER;
            gameOverTime = SDL_GetTicks();
        }

        // キー入力（プレイ中のみ処理）
        if (gameState == STATE_PLAYING && e.type == SDL_KEYDOWN) {
            // プレイヤーが移動中でない場合のみ入力を受け付ける
            if (!player.isMoving()) {
                Direction inputDir = DIR_NONE;

                switch (e.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        inputDir = DIR_UP;
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        inputDir = DIR_DOWN;
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        inputDir = DIR_LEFT;
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        inputDir = DIR_RIGHT;
                        break;
                }

                // 有効な入力があれば移動処理を開始
                if (inputDir != DIR_NONE) {
                    player.setMovementTarget(inputDir);
                    gameState = STATE_MOVING;
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    Uint32 currentTicks = SDL_GetTicks();

    // プレイ中またはアニメーション中はタイマー更新
    if (gameState == STATE_PLAYING || gameState == STATE_MOVING) {
        currentTime -= deltaTime;

        // タイムアップ判定
        if (currentTime <= 0) {
            currentTime = 0;
            gameState = STATE_GAMEOVER;
            gameOverTime = currentTicks;
        }
    }

    // アニメーション中の更新処理
    if (gameState == STATE_MOVING) {
        // プレイヤーの位置更新
        player.update(currentTicks);

        // 移動完了判定
        if (player.isMovementComplete(currentTicks)) {
            // 衝突判定：正しい壁に接触したか
            if (player.checkCollision(player.getMoveDir(), wallTopColor,
                                      wallBottomColor, wallLeftColor,
                                      wallRightColor, directiveColor)) {
                // 成功
                score++;
                successCount++;

                // 5回成功するたびにタイマー制限を厳しくする
                if (successCount % 5 == 0) {
                    currentMaxTime -= 0.2f;
                    if (currentMaxTime < MIN_MAX_TIME) {
                        currentMaxTime = MIN_MAX_TIME;
                    }
                }

                // 次ラウンドの準備
                player.reset();
                currentTime = currentMaxTime;
                directiveColor = getRandomColor();
                randomizeWallColors();
                gameState = STATE_PLAYING;
            } else {
                // 失敗（ゲームオーバー）
                gameState = STATE_GAMEOVER;
                gameOverTime = currentTicks;
            }
        }
    }
}

void Game::render() {
    // 背景をクリア
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
    int gaugeCurrentWidth = (int)(GAUGE_WIDTH * (currentTime / currentMaxTime));
    SDL_Rect currentGauge = {gaugeRect.x, gaugeRect.y, gaugeCurrentWidth,
                             gaugeRect.h};

    // 背景を黒色に
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderFillRect(renderer, &gaugeRect);

    // ゲージの描画（残り時間に応じて色と点滅を制御）
    if (gaugeCurrentWidth <= GAUGE_WIDTH / 2) {
        Uint32 currentTicks = SDL_GetTicks();
        if (currentTicks - lastBlinkTime > BLINK_INTERVAL) {
            blinkOn = !blinkOn;
            lastBlinkTime = currentTicks;
        }

        if (blinkOn) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // 赤
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // 緑
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // 緑
    }

    SDL_RenderFillRect(renderer, &currentGauge);

    // ゲージの枠
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_RenderDrawRect(renderer, &gaugeRect);

    // 指示枠の描画（右上）
    SDL_SetRenderDrawColor(renderer, directiveColor.r, directiveColor.g,
                           directiveColor.b, directiveColor.a);
    SDL_RenderFillRect(renderer, &directiveRect);
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_RenderDrawRect(renderer, &directiveRect);

    // スコア表示
    std::stringstream ss;
    ss << "Score: " << score;
    int textW, textH;
    SDL_Texture* scoreTex = renderText(ss.str(), WHITE, textW, textH);
    if (scoreTex) {
        SDL_Rect dst = {SCORE_POS_X - textW / 2, SCORE_POS_Y - textH / 2, textW,
                        textH};
        SDL_RenderCopy(renderer, scoreTex, nullptr, &dst);
        SDL_DestroyTexture(scoreTex);
    }

    // プレイヤーの描画
    player.render(renderer);

    // ゲームオーバー表示
    if (gameState == STATE_GAMEOVER) {
        int textW, textH;
        SDL_Texture* goTex = renderText("GAME OVER", RED, textW, textH);
        if (goTex) {
            SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                            WINDOW_HEIGHT / 2 - textH / 2, textW, textH};
            SDL_RenderCopy(renderer, goTex, nullptr, &dst);
            SDL_DestroyTexture(goTex);
        }

        // スコアの表示
        std::stringstream ss;
        ss << "Final Score: " << score;
        SDL_Texture* finalScoreTex = renderText(ss.str(), WHITE, textW, textH);
        if (finalScoreTex) {
            SDL_Rect dst = {WINDOW_WIDTH / 2 - textW / 2,
                            WINDOW_HEIGHT / 2 + 50, textW, textH};
            SDL_RenderCopy(renderer, finalScoreTex, nullptr, &dst);
            SDL_DestroyTexture(finalScoreTex);
        }
    }

    // バックバッファを画面に反映
    SDL_RenderPresent(renderer);
}

SDL_Texture* Game::renderText(const std::string& message, SDL_Color color,
                              int& textW, int& textH) {
    if (!font) {
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surface) {
        SDL_Log("TTF_RenderText_Blended Error: %s", TTF_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return nullptr;
    }

    textW = surface->w;
    textH = surface->h;

    SDL_FreeSurface(surface);
    return texture;
}

void Game::drawFilledCircle(int centerX, int centerY, int radius) {
    // 円を塗りつぶして描画
    // 各ピクセルについて中心からの距離を計算し、半径以内ならば描画
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