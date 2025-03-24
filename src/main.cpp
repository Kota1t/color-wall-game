#include <cstdlib>
#include <ctime>

#include "game.h"

int main(int argc, char* argv[]) {
    // 乱数初期化
    srand(static_cast<unsigned int>(time(nullptr)));

    // ゲームインスタンス作成
    Game game;

    // ゲーム初期化
    if (game.initialize()) {
        // ゲーム実行
        game.runGame();
    }

    return 0;
}