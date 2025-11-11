//------------------------------------------------------------
// @file        main.cpp
//------------------------------------------------------------
#include "main.h"
#include "EOSManager.h"
#include <thread>
#include <chrono>
#include <limits>
#include <iostream>

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

    // ログイン待ち
    while (!eos.IsLoggedIn())
    {
        eos.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "ログイン完了！\n";

    if (game.isHost)
    {
        std::string roomName;
        int maxPlayers;
        std::cout << "部屋名を入力してください: ";
        std::getline(std::cin, roomName);
        std::cout << "最大人数を入力してください: ";
        std::cin >> maxPlayers;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        eos.CreateLobbyWithCleanup(roomName, maxPlayers, game.username);

        // ロビー作成完了待ち
        while (!eos.IsLobbyCreated())
        {
            eos.Tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    else
    {
        eos.SearchLobbies();

        // 検索完了待ち
        while (!eos.IsLobbySearchComplete())
        {
            eos.Tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "処理完了！Enterキーで終了...\n";
    std::cin.get();
    return 0;
}

// 初回プロセス
GameContext InitUser()
{
    GameContext context;

    std::cout << "ユーザー名を入力してください: ";
    std::getline(std::cin, context.username);

    char choice = '\0';
    while (choice != 'Y' && choice != 'N')
    {
        std::cout << "ホストにする場合は 'y'、クライアントにする場合は 'n' を入力してください: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = std::toupper(choice);
    }

    context.isHost = (choice == 'Y');
    std::cout << (context.isHost ? "ホストとして起動します\n" : "クライアントとして起動します\n");

    return context;
}
