//------------------------------------------------------------
// @file        EOSManager.cpp
// @brief       EOS SDK 管理クラス（最新版対応・DeviceID匿名ログイン対応）
//------------------------------------------------------------
#include "EOSManager.h"

EOSManager::EOSManager(const char* productName, const char* productVersion)
    : m_ProductName(productName), m_ProductVersion(productVersion),
    m_Platform(nullptr), m_ConnectHandle(nullptr)
{}

EOSManager::~EOSManager()
{
    if (m_Platform)
    {
        EOS_Platform_Release(m_Platform);
        m_Platform = nullptr;
    }
    EOS_Shutdown();
}

bool EOSManager::Initialize()
{
    EOS_InitializeOptions options{};
    options.ApiVersion = EOS_INITIALIZE_API_LATEST;
    options.ProductName = m_ProductName.c_str();
    options.ProductVersion = m_ProductVersion.c_str();

    EOS_EResult result = EOS_Initialize(&options);
    if (result != EOS_EResult::EOS_Success)
    {
        std::cout << "EOS SDK 初期化失敗: " << (int)result << "\n";
        return false;
    }

    std::cout << "EOS SDK 初期化成功\n";

    EOS_Platform_Options platformOptions{};
    platformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    platformOptions.ProductId = "8febe97d165c4d69bb1190d54172e7b5";
    platformOptions.SandboxId = "26e72d76654947679ecddb972041700c";
    platformOptions.DeploymentId = "f58934a23b394ac1ae85d13ebc7e0c85";
    platformOptions.ClientCredentials.ClientId = "xyza7891dVcoBtQ9kGUqUJcoQlmVcG9G";
    platformOptions.ClientCredentials.ClientSecret = "u8jai0k/RlJSHBo58fBpOjcx4aqB6kqrV4qBuYncefg";
    platformOptions.bIsServer = false;

    m_Platform = EOS_Platform_Create(&platformOptions);
    if (!m_Platform)
    {
        std::cout << "EOS Platform作成失敗\n";
        return false;
    }

    m_ConnectHandle = EOS_Platform_GetConnectInterface(m_Platform);
    return true;
}

void EOSManager::Tick()
{
    if (m_Platform)
        EOS_Platform_Tick(m_Platform);
}

//---------------------------------------------
// 匿名ログイン(DeviceID)対応
//---------------------------------------------
void EOSManager::AnonymousConnectLogin()
{
    if (!m_ConnectHandle) return;

    // Step 1: デバイスID作成（既に存在する場合もOK）
    EOS_Connect_CreateDeviceIdOptions createOptions{};
    createOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
    createOptions.DeviceModel = "WindowsPC";

    EOS_Connect_CreateDeviceId(m_ConnectHandle, &createOptions, this,
        [](const EOS_Connect_CreateDeviceIdCallbackInfo* data)
        {
            EOSManager* self = static_cast<EOSManager*>(data->ClientData);
            if (data->ResultCode == EOS_EResult::EOS_Success ||
                data->ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
            {
                std::cout << "DeviceID作成済み or 既に存在\n";
                self->LoginWithDeviceID();
            }
            else
            {
                std::cout << "DeviceID作成失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
            }
        });
}

//---------------------------------------------
// DeviceIDログイン本体
//---------------------------------------------
void EOSManager::LoginWithDeviceID()
{
    EOS_Connect_Credentials creds{};
    creds.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
    creds.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
    creds.Token = nullptr;

    EOS_Connect_UserLoginInfo userLoginInfo{};
    userLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
    userLoginInfo.DisplayName = "LocalUser"; // ← 任意（ユーザー名でもOK）

    EOS_Connect_LoginOptions loginOptions{};
    loginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
    loginOptions.Credentials = &creds;
    loginOptions.UserLoginInfo = &userLoginInfo; // ← ★追加必須！

    EOS_Connect_Login(m_ConnectHandle, &loginOptions, nullptr,
        [](const EOS_Connect_LoginCallbackInfo* data)
        {
            if (data->ResultCode == EOS_EResult::EOS_Success)
            {
                char buffer[64]{};
                int32_t bufLen = sizeof(buffer);
                EOS_ProductUserId_ToString(data->LocalUserId, buffer, &bufLen);
                std::cout << "Connect DeviceIDログイン成功: UserID=" << buffer << "\n";
            }
            else
            {
                std::cout << "Connect DeviceIDログイン失敗: "
                    << EOS_EResult_ToString(data->ResultCode) << "\n";
            }
        });
}

