//------------------------------------------------------------
// @file        EOSManager.h
// @brief       
//------------------------------------------------------------
#ifndef _EOSMANAGER_H_
#define _EOSMANAGER_H_

#include "includemanager.h"



class EOSManager
{
public:
    EOSManager(const char* productName, const char* productVersion);
    ~EOSManager();

    bool Initialize();
    void Tick();

    void AnonymousConnectLogin(); // ← 匿名ログイン入口

private:
    void LoginWithDeviceID();     // ← 内部処理

    std::string m_ProductName;
    std::string m_ProductVersion;

    EOS_HPlatform m_Platform;
    EOS_HConnect m_ConnectHandle;
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