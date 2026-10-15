#include "Account/S1AccountSubsystem.h"

#include "ClientPacketHandler.h"
#include "Engine/GameInstance.h"
#include "Network/S1NetworkSubsystem.h"
#include "Protocol.pb.h"

namespace
{
	FString GetLoginFailureMessage(const Protocol::S_LOGIN& Packet)
	{
		switch (Packet.result())
		{
		case Protocol::LOGIN_RESULT_ACCOUNT_ALREADY_ONLINE:
			return TEXT("이미 접속 중인 계정입니다.");
		case Protocol::LOGIN_RESULT_SESSION_ALREADY_AUTHENTICATED:
			return TEXT("현재 연결은 이미 로그인되어 있습니다.");
		case Protocol::LOGIN_RESULT_INVALID_CREDENTIALS:
			return TEXT("아이디 또는 비밀번호가 올바르지 않습니다.");
		case Protocol::LOGIN_RESULT_DATABASE_UNAVAILABLE:
			return TEXT("로그인 서버를 사용할 수 없습니다. 잠시 후 다시 시도해 주세요.");
		case Protocol::LOGIN_RESULT_CHARACTER_QUERY_FAILED:
			return TEXT("캐릭터 정보를 불러오지 못했습니다.");
		case Protocol::LOGIN_RESULT_INVALID_REQUEST:
			return TEXT("로그인 입력값이 올바르지 않습니다.");
		default:
			return Packet.error_message().empty()
				? TEXT("로그인에 실패했습니다.")
				: UTF8_TO_TCHAR(Packet.error_message().c_str());
		}
	}
}

void US1AccountSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<US1NetworkSubsystem>();

	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem())
	{
		NetworkSubsystem->OnLoginPacket.AddUObject(
			this,
			&US1AccountSubsystem::HandleLoginPacket);
		NetworkSubsystem->OnSignUpPacket.AddUObject(
			this,
			&US1AccountSubsystem::HandleSignUpPacket);
		NetworkSubsystem->OnCharacterCreatedPacket.AddUObject(
			this,
			&US1AccountSubsystem::HandleCharacterCreatedPacket);
		NetworkSubsystem->OnEnterGamePacket.AddUObject(
			this,
			&US1AccountSubsystem::HandleEnterGamePacket);
	}
}

void US1AccountSubsystem::Deinitialize()
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem())
	{
		NetworkSubsystem->OnLoginPacket.RemoveAll(this);
		NetworkSubsystem->OnSignUpPacket.RemoveAll(this);
		NetworkSubsystem->OnCharacterCreatedPacket.RemoveAll(this);
		NetworkSubsystem->OnEnterGamePacket.RemoveAll(this);
	}

	ResetAuthenticationState();
	Super::Deinitialize();
}

void US1AccountSubsystem::ResetAuthenticationState()
{
	AuthenticatedAccountId = 0;
	bLoginRequestPending = false;
	bLoginUiActionAuthorized = false;
	Characters.Reset();
}

void US1AccountSubsystem::Login(
	const FString& AccountName,
	const FString& Password)
{
	if (!bLoginUiActionAuthorized)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ignored Login() outside the authentication UI."));
		return;
	}

	bLoginUiActionAuthorized = false;

	US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem();
	if (NetworkSubsystem == nullptr ||
		!NetworkSubsystem->IsConnected() ||
		AccountName.IsEmpty() ||
		Password.IsEmpty())
	{
		return;
	}

	FTCHARToUTF8 ConvertedAccountName(*AccountName);
	FTCHARToUTF8 ConvertedPassword(*Password);

	Protocol::C_LOGIN Packet;
	Packet.set_account_name(
		ConvertedAccountName.Get(),
		ConvertedAccountName.Length());
	Packet.set_password(
		ConvertedPassword.Get(),
		ConvertedPassword.Length());

	bLoginRequestPending = true;
	NetworkSubsystem->SendPacket(
		ClientPacketHandler::MakeSendBuffer(Packet));
}

void US1AccountSubsystem::LoginFromAuthUI(
	const FString& AccountName,
	const FString& Password)
{
	bLoginUiActionAuthorized = true;
	Login(AccountName, Password);
}

void US1AccountSubsystem::SignUp(
	const FString& AccountName,
	const FString& Password)
{
	US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem();
	if (NetworkSubsystem == nullptr ||
		!NetworkSubsystem->IsConnected() ||
		AccountName.IsEmpty() ||
		Password.IsEmpty())
	{
		OnSignUpCompleted.Broadcast(
			false,
			static_cast<int32>(
				Protocol::SIGN_UP_RESULT_DATABASE_UNAVAILABLE),
			TEXT("게임 서버에 연결되어 있지 않습니다."));
		return;
	}

	FTCHARToUTF8 ConvertedAccountName(*AccountName);
	FTCHARToUTF8 ConvertedPassword(*Password);

	Protocol::C_SIGN_UP Packet;
	Packet.set_account_name(
		ConvertedAccountName.Get(),
		ConvertedAccountName.Length());
	Packet.set_password(
		ConvertedPassword.Get(),
		ConvertedPassword.Length());

	NetworkSubsystem->SendPacket(
		ClientPacketHandler::MakeSendBuffer(Packet));
}

void US1AccountSubsystem::CreateCharacter(
	const FString& CharacterName,
	int32 PlayerType)
{
	US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem();
	if (NetworkSubsystem == nullptr ||
		!NetworkSubsystem->IsConnected())
	{
		return;
	}

	FTCHARToUTF8 ConvertedName(*CharacterName);

	Protocol::C_CREATE_CHARACTER Packet;
	Packet.set_character_name(
		ConvertedName.Get(),
		ConvertedName.Length());
	Packet.set_player_type(
		static_cast<Protocol::PlayerType>(PlayerType));

	NetworkSubsystem->SendPacket(
		ClientPacketHandler::MakeSendBuffer(Packet));
}

void US1AccountSubsystem::EnterCharacter(int64 CharacterId)
{
	US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem();
	if (NetworkSubsystem == nullptr ||
		!NetworkSubsystem->IsConnected() ||
		CharacterId <= 0)
	{
		return;
	}

	Protocol::C_ENTER_GAME Packet;
	Packet.set_character_id(static_cast<uint64>(CharacterId));
	Packet.set_loot_presentation_version(1);
	Packet.set_loot_contract_version(1); // Activated only by the server's negotiated response.
	Packet.set_item_use_contract_version(1);
	Packet.set_inventory_move_contract_version(1);
	Packet.set_inventory_stack_contract_version(1);
	Packet.set_inventory_discard_contract_version(0); // Never fall back to permanent destruction.
	Packet.set_player_field_drop_contract_version(1);

	NetworkSubsystem->SendPacket(
		ClientPacketHandler::MakeSendBuffer(Packet));

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Enter character requested. CharacterId: %lld"),
		CharacterId);
}

void US1AccountSubsystem::EnterFirstCharacter()
{
	if (Characters.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No existing character available"));
		return;
	}

	EnterCharacter(Characters[0].CharacterId);
}

void US1AccountSubsystem::HandleLoginPacket(
	const Protocol::S_LOGIN& Packet)
{
	if (!ConsumePendingLoginResponse())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Ignored S_LOGIN without a pending login request."));
		return;
	}

	if (!Packet.success())
	{
		AuthenticatedAccountId = 0;
		const FString FailureMessage = GetLoginFailureMessage(Packet);
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Login failed. Result: %d, Message: %s"),
			static_cast<int32>(Packet.result()),
			*FailureMessage);
		OnLoginCompleted.Broadcast(false, FailureMessage);
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Login success. AccountId: %llu, Characters: %d"),
		Packet.account_id(),
		Packet.players_size());

	AuthenticatedAccountId = Packet.account_id();
	SetCharacterList(Packet);
	OnLoginCompleted.Broadcast(true, TEXT("로그인에 성공했습니다."));
}

void US1AccountSubsystem::HandleSignUpPacket(
	const Protocol::S_SIGN_UP& Packet)
{
	OnSignUpCompleted.Broadcast(
		Packet.success(),
		static_cast<int32>(Packet.result()),
		UTF8_TO_TCHAR(Packet.error_message().c_str()));
}

void US1AccountSubsystem::HandleCharacterCreatedPacket(
	const Protocol::S_CREATE_CHARACTER& Packet)
{
	if (!Packet.success())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Create character failed: %s"),
			UTF8_TO_TCHAR(Packet.error_message().c_str()));
		OnCharacterCreated.Broadcast(
			false,
			UTF8_TO_TCHAR(Packet.error_message().c_str()));
		return;
	}

	const Protocol::CharacterInfo& Character = Packet.character();
	FS1CharacterSummary Summary;
	Summary.CharacterId = static_cast<int64>(Character.character_id());
	Summary.CharacterName = UTF8_TO_TCHAR(
		Character.character_name().c_str());
	Summary.Level = static_cast<int32>(Character.level());
	Summary.PlayerType = static_cast<int32>(Character.player_type());
	Characters.Add(Summary);

	OnCharacterCreated.Broadcast(true, TEXT("Character created."));
}

void US1AccountSubsystem::HandleEnterGamePacket(
	const Protocol::S_ENTER_GAME& Packet)
{
	OnEnterGameCompleted.Broadcast(
		Packet.success(),
		Packet.success()
			? TEXT("게임 접속 성공")
			: TEXT("게임 입장에 실패했습니다."));
}

void US1AccountSubsystem::SetCharacterList(
	const Protocol::S_LOGIN& LoginPacket)
{
	Characters.Reset();

	for (const Protocol::CharacterInfo& Character : LoginPacket.players())
	{
		const uint64 CharacterId = Character.character_id();
		if (CharacterId == 0 ||
			CharacterId > static_cast<uint64>(MAX_int64))
		{
			continue;
		}

		FS1CharacterSummary Summary;
		Summary.CharacterId = static_cast<int64>(CharacterId);
		Summary.CharacterName = UTF8_TO_TCHAR(
			Character.character_name().c_str());
		Summary.Level = static_cast<int32>(Character.level());
		Summary.PlayerType = static_cast<int32>(Character.player_type());
		Characters.Add(Summary);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Character list cached. Count: %d"),
		Characters.Num());
}

bool US1AccountSubsystem::ConsumePendingLoginResponse()
{
	if (!bLoginRequestPending)
		return false;

	bLoginRequestPending = false;
	return true;
}

US1NetworkSubsystem* US1AccountSubsystem::GetNetworkSubsystem() const
{
	return GetGameInstance() != nullptr
		? GetGameInstance()->GetSubsystem<US1NetworkSubsystem>()
		: nullptr;
}
