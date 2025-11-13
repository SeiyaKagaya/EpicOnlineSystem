// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

struct SampleConstants
{
	/** The product id for the running application, found on the dev portal */
	static constexpr char ProductId[] = "8febe97d165c4d69bb1190d54172e7b5";
	/** The application id for the running application, found on the dev portal */
	static constexpr char ApplicationId[] = "";
	/** The sandbox id for the running application, found on the dev portal */
	static constexpr char SandboxId[] = "26e72d76654947679ecddb972041700c";
	/** The deployment id for the running application, found on the dev portal */
	static constexpr char DeploymentId[] = "f58934a23b394ac1ae85d13ebc7e0c85";
	/** Client id of the service permissions entry, found on the dev portal */
	static constexpr char ClientCredentialsId[] = "xyza7891dVcoBtQ9kGUqUJcoQlmVcG9G";
	/** Client secret for accessing the set of permissions, found on the dev portal */
	static constexpr char ClientCredentialsSecret[] = "u8jai0k/RlJSHBo58fBpOjcx4aqB6kqrV4qBuYncefg";
	/** Game name */
	static constexpr char GameName[] = "Your game name";
	/** Encryption key. Not used by this sample. */
	static constexpr char EncryptionKey[] = "1111111111111111111111111111111111111111111111111111111";

	/** The Minimum window Width for this sample. */
	static constexpr int32_t MinimumWindowWidth = 1024;

	/** The Minimum window Height for this sample. */
	static constexpr int32_t MinimumWindowHeight = 800;

	/** The Default window Width for this sample. */
	static constexpr int32_t DefaultWindowWidth = 1024;

	/** The Default window Height for this sample. */
	static constexpr int32_t DefaultWindowHeight = 800;
};



