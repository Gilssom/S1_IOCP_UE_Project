#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "S1AccountSubsystem.generated.h"

class US1NetworkSubsystem;

namespace Protocol
{
	class S_LOGIN;
	class S_SIGN_UP;
	class S_CREATE_CHARACTER;
	class S_ENTER_GAME;
}

struct FS1CharacterSummary
{
	int64 CharacterId = 0;
	FString CharacterName;
	int32 Level = 1;
	int32 PlayerType = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnLoginCompleted,
	bool, bSuccess,
	const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSignUpCompleted,
	bool, bSuccess,
	int32, ResultCode,
	const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCharacterCreated,
	bool, bSuccess,
	const FString&, Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnEnterGameCompleted,
	bool, bSuccess,
	const FString&, Message);

UCLASS()
class S1_API US1AccountSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(
		FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ResetAuthenticationState();
	uint64 GetAuthenticatedAccountId() const { return AuthenticatedAccountId; }
	void Login(const FString& AccountName, const FString& Password);
	void LoginFromAuthUI(
		const FString& AccountName,
		const FString& Password);
	void SignUp(const FString& AccountName, const FString& Password);
	void CreateCharacter(
		const FString& CharacterName,
		int32 PlayerType);
	void EnterCharacter(int64 CharacterId);
	void EnterFirstCharacter();

	const TArray<FS1CharacterSummary>& GetCharacters() const
	{
		return Characters;
	}

	UPROPERTY(BlueprintAssignable)
	FOnLoginCompleted OnLoginCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnSignUpCompleted OnSignUpCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterCreated OnCharacterCreated;

	UPROPERTY(BlueprintAssignable)
	FOnEnterGameCompleted OnEnterGameCompleted;

private:
	void HandleLoginPacket(const Protocol::S_LOGIN& Packet);
	void HandleSignUpPacket(const Protocol::S_SIGN_UP& Packet);
	void HandleCharacterCreatedPacket(
		const Protocol::S_CREATE_CHARACTER& Packet);
	void HandleEnterGamePacket(const Protocol::S_ENTER_GAME& Packet);
	void SetCharacterList(const Protocol::S_LOGIN& LoginPacket);
	bool ConsumePendingLoginResponse();
	US1NetworkSubsystem* GetNetworkSubsystem() const;

private:
	bool bLoginRequestPending = false;
	uint64 AuthenticatedAccountId = 0;
	bool bLoginUiActionAuthorized = false;
	TArray<FS1CharacterSummary> Characters;
};
