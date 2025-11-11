//------------------------------------------------------------
// @file        EOSManager.h
//------------------------------------------------------------
#ifndef _EOSMANAGER_H_
#define _EOSMANAGER_H_

#include "includemanager.h"
#include <string>

class EOSManager
{
public:
    EOSManager(const char* productName, const char* productVersion);
    ~EOSManager();

    bool Initialize();
    void Tick();
    void AnonymousConnectLogin();
    void SearchLobbies();
    void CreateLobby(const std::string& roomName, int maxPlayers, const std::string& hostName);
    void CreateLobbyWithCleanup(const std::string& roomName, int maxPlayers, const std::string& hostName);

    bool IsLoggedIn() const { return m_bLoggedIn; }
    bool IsLobbyCreated() const { return m_bLobbyCreated; }
    bool IsLobbySearchComplete() const { return m_bLobbySearchComplete; }

private:
    void LoginWithDeviceID();

    std::string m_PendingRoomName;
    int m_PendingMaxPlayers;
    std::string m_PendingHostName;

    std::string m_ProductName;
    std::string m_ProductVersion;

    EOS_HPlatform m_Platform;
    EOS_HConnect m_ConnectHandle;
    EOS_HLobby m_LobbyHandle;
    EOS_ProductUserId m_LocalUserId;

    bool m_bLoggedIn = false;
    bool m_bLobbyCreated = false;
    bool m_bLobbySearchComplete = false;
    EOS_HLobbySearch m_SearchHandle = nullptr;

    static void EOS_CALL OnLobbySearchFindCompleteStatic(const EOS_LobbySearch_FindCallbackInfo* data);
    static void EOS_CALL OnCreateLobbyCompleteStatic(const EOS_Lobby_CreateLobbyCallbackInfo* data);
};

#endif




//2️⃣ 作る手順（概要）
//
//Epic Developer Portal
//にログイン
//
//「製品（Products）」 → 「新しい製品を作成」
//
//名前やプラットフォームを入力（ゲームのタイトルや任意の文字列で OK）
//
//作成後に Product ID が発行される → SDK に設定
//
//同時に Sandbox（環境） と Deployment（配置先環境） も作成
//
//Sandbox は通常 Dev、Deployment は Production で OK
//
//Client ID / Client Secret も作成 → 匿名ログインでも必要
//
//3️⃣ 匿名ログインでも必要な理由
//
//匿名ログインは Epic アカウント不要 ですが、EOS が誰のゲームかを識別する必要があります
//
//そのため 製品（Product）ID / Sandbox / Deployment / ClientCredentials が必須です
//
//💡 まとめ
//
//「製品を作る」とは EOS 上であなたのゲームを登録すること
//
//これを行わないと EOS_Platform_Create は nullptr を返す
//
//匿名ログインでも必ず必要