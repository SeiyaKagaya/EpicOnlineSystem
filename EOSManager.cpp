#include "EOSManager.h"


EOSManager::EOSManager(const char* productName, const char* productVersion)
    : m_ProductName(productName), m_ProductVersion(productVersion),
    m_Platform(nullptr), m_ConnectHandle(nullptr),
    m_LobbyHandle(nullptr), m_SearchHandle(nullptr)
{}

EOSManager::~EOSManager()
{
    if (m_Platform)
        EOS_Platform_Release(m_Platform);
    EOS_Shutdown();
}

bool EOSManager::Initialize()
{
    EOS_InitializeOptions options{};
    options.ApiVersion = EOS_INITIALIZE_API_LATEST;
    options.ProductName = m_ProductName.c_str();
    options.ProductVersion = m_ProductVersion.c_str();

    if (EOS_Initialize(&options) != EOS_EResult::EOS_Success)
    {
        std::cout << "EOS SDK 初期化失敗\n";
        return false;
    }

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
    m_LobbyHandle = EOS_Platform_GetLobbyInterface(m_Platform);

    return true;
}

void EOSManager::Tick()
{
    if (m_Platform)
        EOS_Platform_Tick(m_Platform);
}

void EOSManager::AnonymousConnectLogin()
{
    if (!m_ConnectHandle) return;

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
                std::cout << "DeviceID作成失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
        });
}

void EOSManager::LoginWithDeviceID()
{
    EOS_Connect_Credentials creds{};
    creds.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
    creds.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
    creds.Token = nullptr;

    EOS_Connect_UserLoginInfo userLoginInfo{};
    userLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
    userLoginInfo.DisplayName = "LocalUser";

    EOS_Connect_LoginOptions loginOptions{};
    loginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
    loginOptions.Credentials = &creds;
    loginOptions.UserLoginInfo = &userLoginInfo;

    EOS_Connect_Login(m_ConnectHandle, &loginOptions, this,
        [](const EOS_Connect_LoginCallbackInfo* data)
        {
            EOSManager* self = static_cast<EOSManager*>(data->ClientData);
            if (!self) return;

            if (data->ResultCode == EOS_EResult::EOS_Success)
            {
                self->m_LocalUserId = data->LocalUserId;
                self->m_bLoggedIn = true;

                char buffer[64]{};
                int32_t len = sizeof(buffer);
                EOS_ProductUserId_ToString(data->LocalUserId, buffer, &len);
                std::cout << "Connect DeviceIDログイン成功: UserID=" << buffer << "\n";
            }
            else
                std::cout << "Connect DeviceIDログイン失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
        });
}

//==================================================
// ✅ 修正版 CreateLobbyWithCleanup
//==================================================
void EOSManager::CreateLobbyWithCleanup(const std::string& roomName, int maxPlayers, const std::string& hostName)
{
    if (!m_LobbyHandle) return;

    m_PendingRoomName = roomName;
    m_PendingMaxPlayers = maxPlayers;
    m_PendingHostName = hostName;

    EOS_Lobby_CreateLobbyOptions opts{};
    opts.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
    opts.LocalUserId = m_LocalUserId;
    opts.MaxLobbyMembers = maxPlayers;
    opts.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
    opts.bAllowInvites = EOS_TRUE;

    // ⬇️ API要件を満たすため、bPresenceEnabled = TRUE に戻す
    opts.bPresenceEnabled = EOS_TRUE;

    opts.BucketId = "default";

    EOS_Lobby_CreateLobby(m_LobbyHandle, &opts, this, OnCreateLobbyCompleteStatic);
    std::cout << "ロビー作成要求送信\n";
}

//==================================================
// ✅ OnCreateLobbyComplete 修正版
//==================================================
void EOS_CALL EOSManager::OnCreateLobbyCompleteStatic(const EOS_Lobby_CreateLobbyCallbackInfo* data)
{
    EOSManager* self = static_cast<EOSManager*>(data->ClientData);
    if (!self) return;

    if (data->ResultCode == EOS_EResult::EOS_Success)
    {
        std::cout << "ロビー作成成功！ LobbyId = " << data->LobbyId << "\n";
        // ⚠️ self->m_bLobbyCreated = true; ⬅️ ここではまだセットしない

        // ✅ ロビー設定変更の準備
        EOS_HLobbyModification mod = nullptr;
        EOS_Lobby_UpdateLobbyModificationOptions modOpts{};
        modOpts.ApiVersion = EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST;
        modOpts.LobbyId = data->LobbyId;
        modOpts.LocalUserId = self->m_LocalUserId;

        if (EOS_Lobby_UpdateLobbyModification(self->m_LobbyHandle, &modOpts, &mod) == EOS_EResult::EOS_Success && mod)
        {
            // --- 属性追加 ---
            EOS_LobbyModification_AddAttributeOptions attrOpts{};
            attrOpts.ApiVersion = EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST;

            EOS_Lobby_AttributeData attrData{};
            attrData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
            attrData.Key = "test";
            attrData.Value.AsInt64 = 1;
            attrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;

            attrOpts.Attribute = &attrData;
            attrOpts.Visibility = EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
            EOS_LobbyModification_AddAttribute(mod, &attrOpts);

            // --- 🔹 広告設定を明示的に指定 ---
            EOS_LobbyModification_SetPermissionLevelOptions permOpts{};
            permOpts.ApiVersion = EOS_LOBBYMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
            permOpts.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
            EOS_LobbyModification_SetPermissionLevel(mod, &permOpts);

            // --- 🔹 招待を許可 ---
            EOS_LobbyModification_SetInvitesAllowedOptions invitesOpts{};
            invitesOpts.ApiVersion = EOS_LOBBYMODIFICATION_SETINVITESALLOWED_API_LATEST;
            invitesOpts.bInvitesAllowed = EOS_TRUE;
            EOS_LobbyModification_SetInvitesAllowed(mod, &invitesOpts);

            // --- ❌ 許可プラットフォームIDの指定は削除する ---
            // EOS_LobbyModification_SetAllowedPlatformIds(mod, &platformOpts); // 削除

            // ✅ 変更を反映 (コールバックを指定)
            EOS_Lobby_UpdateLobbyOptions updateOpts{};
            updateOpts.ApiVersion = EOS_LOBBY_UPDATELOBBY_API_LATEST;
            updateOpts.LobbyModificationHandle = mod;

            // ⬇️ 新しいコールバックを指定する
            EOS_Lobby_UpdateLobby(self->m_LobbyHandle, &updateOpts, self, OnUpdateLobbyCompleteStatic);

            EOS_LobbyModification_Release(mod);
        }
    }
    else
    {
        std::cout << "ロビー作成失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
    }
}

//==================================================
// ✅ 新しいコールバック関数
//==================================================
void EOS_CALL EOSManager::OnUpdateLobbyCompleteStatic(const EOS_Lobby_UpdateLobbyCallbackInfo* data)
{
    EOSManager* self = static_cast<EOSManager*>(data->ClientData);
    if (!self) return;

    if (data->ResultCode == EOS_EResult::EOS_Success)
    {
        std::cout << "ロビー属性 'bucket=default' および広告設定完了\n";

        // ⬇️ 属性更新が完了したここでフラグを立てる
        self->m_bLobbyCreated = true;
    }
    else
    {
        std::cout << "ロビー属性設定失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
    }
}

//==================================================
// 検索修正案
//==================================================
void EOSManager::SearchLobbies()
{
    if (!m_LobbyHandle) return;

    if (m_LocalUserId == nullptr) {
        std::cout << "UserID未初期化でロビー検索不可\n";
        return;
    }
    std::cout << "[Debug] SearchLobbies LocalUserId is OK: " << (void*)m_LocalUserId << "\n";

    EOS_Lobby_CreateLobbySearchOptions opts{};
    opts.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
    opts.MaxResults = 20;

    EOS_HLobbySearch searchHandle = nullptr;
    if (EOS_Lobby_CreateLobbySearch(m_LobbyHandle, &opts, &searchHandle) != EOS_EResult::EOS_Success || !searchHandle)
    {
        std::cout << "ロビー検索作成失敗\n";
        return;
    }
    m_SearchHandle = searchHandle;

    //// --- 1. BucketId を属性として検索に追加 (古いSDKでの方法) ---
    //EOS_Lobby_AttributeData bucketAttrData{};
    //bucketAttrData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
    //bucketAttrData.Key = "BucketId"; // EOSの内部属性キー
    //bucketAttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_String;
    //bucketAttrData.Value.AsUtf8 = "default"; // ホスト側で設定したBucketId

    //EOS_LobbySearch_SetParameterOptions bucketParamOpts{};
    //bucketParamOpts.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
    //bucketParamOpts.Parameter = &bucketAttrData;
    //bucketParamOpts.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL; // 完全一致

    //EOS_EResult ret = EOS_LobbySearch_SetParameter(searchHandle, &bucketParamOpts);
    //std::cout << "[Debug] SetParameter (BucketId) return: " << EOS_EResult_ToString(ret) << "\n";
    //if (ret != EOS_EResult::EOS_Success) return;

    // --- 2. 既存のカスタム属性での検索 ---
    EOS_Lobby_AttributeData customAttrData; // staticを削除し、ローカル変数へ
    customAttrData.ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
    customAttrData.Key = "test";
    customAttrData.ValueType = EOS_ESessionAttributeType::EOS_SAT_Int64;
    customAttrData.Value.AsInt64 = 1;

    EOS_LobbySearch_SetParameterOptions customParamOpts{};
    customParamOpts.ApiVersion = EOS_LOBBYSEARCH_SETPARAMETER_API_LATEST;
    customParamOpts.Parameter = &customAttrData;
    customParamOpts.ComparisonOp = EOS_EComparisonOp::EOS_CO_EQUAL;

    // BucketIdの ret 宣言を削除したため、ここで再定義
    EOS_EResult ret = EOS_LobbySearch_SetParameter(searchHandle, &customParamOpts);
    std::cout << "[Debug] SetParameter (test=1) return: " << EOS_EResult_ToString(ret) << "\n";
    if (ret != EOS_EResult::EOS_Success) return;

    // --- SetTargetUserId は削除済みの前提 ---

    EOS_LobbySearch_FindOptions findOpts{};
    findOpts.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
    findOpts.LocalUserId = m_LocalUserId;

    std::cout << "[Debug] Find LocalUserId addr: " << (void*)findOpts.LocalUserId << "\n";

    EOS_LobbySearch_Find(searchHandle, &findOpts, this, OnLobbySearchFindCompleteStatic);
    std::cout << "ロビー検索要求送信中\n";
}

void EOS_CALL EOSManager::OnLobbySearchFindCompleteStatic(const EOS_LobbySearch_FindCallbackInfo* data)
{
    EOSManager* self = static_cast<EOSManager*>(data->ClientData);
    if (!self) return;

    if (data->ResultCode != EOS_EResult::EOS_Success)
    {
        std::cout << "ロビー検索失敗: " << EOS_EResult_ToString(data->ResultCode) << "\n";
        self->m_bLobbySearchComplete = true;
        return;
    }

    EOS_HLobbySearch searchHandle = self->m_SearchHandle;
    if (!searchHandle)
    {
        std::cout << "検索ハンドルが無効です\n";
        self->m_bLobbySearchComplete = true;
        return;
    }

    uint32_t count = EOS_LobbySearch_GetSearchResultCount(searchHandle, nullptr);
    std::cout << "ロビー検索完了: " << count << " 件\n";

    for (uint32_t i = 0; i < count; ++i)
    {
        EOS_HLobbyDetails details = nullptr;
        EOS_LobbySearch_CopySearchResultByIndexOptions copyOpts{};
        copyOpts.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
        copyOpts.LobbyIndex = i;

        if (EOS_LobbySearch_CopySearchResultByIndex(searchHandle, &copyOpts, &details) != EOS_EResult::EOS_Success || !details)
            continue;

        EOS_LobbyDetails_Info* info = nullptr;
        EOS_LobbyDetails_CopyInfoOptions infoOpts{};
        infoOpts.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

        if (EOS_LobbyDetails_CopyInfo(details, &infoOpts, &info) == EOS_EResult::EOS_Success && info)
        {
            std::cout << "=== ロビー情報 ===\n";
            std::cout << "ロビーID: " << info->LobbyId << "\n";
            std::cout << "最大メンバー数: " << info->MaxMembers << "\n";
            std::cout << "PermissionLevel: " << static_cast<int>(info->PermissionLevel) << "\n";

            // ✅ 最新 SDK に合わせて属性を取得
            EOS_LobbyDetails_GetAttributeCountOptions attrCountOpts{};
            attrCountOpts.ApiVersion = EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST;
            uint32_t attributeCount = EOS_LobbyDetails_GetAttributeCount(details, &attrCountOpts);

            for (uint32_t j = 0; j < attributeCount; ++j)
            {
                EOS_Lobby_Attribute* attr = nullptr;
                EOS_LobbyDetails_CopyAttributeByIndexOptions attrOpts{};
                attrOpts.ApiVersion = EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST;
                attrOpts.AttrIndex = j;

                if (EOS_LobbyDetails_CopyAttributeByIndex(details, &attrOpts, &attr) == EOS_EResult::EOS_Success && attr)
                {
                    if (attr->Data->ValueType == EOS_ESessionAttributeType::EOS_SAT_String)
                        std::cout << "属性: " << attr->Data->Key << " = " << attr->Data->Value.AsUtf8 << "\n";

                    EOS_Lobby_Attribute_Release(attr);
                }
            }

            EOS_LobbyDetails_Info_Release(info);
        }

        EOS_LobbyDetails_Release(details);
    }

    // ⬇️ 処理の最後に以下を追加
    EOS_LobbySearch_Release(searchHandle);
    self->m_SearchHandle = nullptr;

    self->m_bLobbySearchComplete = true;
}
