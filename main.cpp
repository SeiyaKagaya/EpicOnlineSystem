//------------------------------------------------------------
// @file        main.cpp
// @brief       EpicGamesのオンラインSDKを使用した通信システム
//------------------------------------------------------------
#include "main.h"
#include "EOSManager.h"


//----------------------------------------------
// メイン
//----------------------------------------------
int main()
{
    EOSManager eos("CityCleaners", "0.1a");

    if (!eos.Initialize())
        return -1;

    GameContext game = InitUser();

    std::cout << "ユーザー名: " << game.username << "\n";
    std::cout << "モード: " << (game.isHost ? "ホスト" : "クライアント") << "\n";

    std::cout << "Connect匿名ログイン試行中\n";
    eos.AnonymousConnectLogin();

    // メインループ
    while (true)
    {
        eos.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Lobby作成/検索やP2P通信はこの後に続ける
    getchar();
    getchar();
    getchar();
    return 0;
}

//初回プロセス
GameContext InitUser()
{
    GameContext context;

    // ユーザー名入力
    std::cout << "ユーザー名を入力してください: ";
    std::getline(std::cin, context.username);

    // ホスト/クライアント選択 (y/n)
    char choice = '\0';
    while (choice != 'Y' && choice != 'N')
    {
        std::cout << "ホストにする場合は 'y'、クライアントにする場合は 'n' を入力してください: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // バッファクリア
        choice = std::toupper(choice); // 大文字化
    }

    context.isHost = (choice == 'Y');
    std::cout << (context.isHost ? "ホストとして起動します\n" : "クライアントとして起動します\n");

    return context;
}